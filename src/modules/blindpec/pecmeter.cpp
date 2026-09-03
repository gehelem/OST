#include "pecmeter.h"

#include <algorithm>
#include <cmath>

namespace pecmeter
{

namespace
{

/// Fourier shift: return `img` translated so its content moves by (+dx, +dy) px.
/// Exact for band-limited images (multiplies the spectrum by a linear phase ramp),
/// so it introduces no interpolation bias of its own - ideal for generating known
/// sub-pixel shifts to probe an estimator's pixel-locking S-curve.
cv::Mat fourierShift(const cv::Mat &img, double dx, double dy)
{
    const int H = img.rows, W = img.cols;

    cv::Mat F;
    cv::dft(img, F, cv::DFT_COMPLEX_OUTPUT);   // W x H, CV_32FC2

    // Separable phase ramps: exp(-i 2 pi f dx / W) along X, likewise Y.
    std::vector<cv::Vec2f> rx(W), ry(H);
    for (int k = 0; k < W; ++k)
    {
        const double f  = (k <= W / 2) ? k : k - W;
        const double ph = -2.0 * CV_PI * f * dx / W;
        rx[k] = cv::Vec2f(static_cast<float>(std::cos(ph)), static_cast<float>(std::sin(ph)));
    }
    for (int k = 0; k < H; ++k)
    {
        const double f  = (k <= H / 2) ? k : k - H;
        const double ph = -2.0 * CV_PI * f * dy / H;
        ry[k] = cv::Vec2f(static_cast<float>(std::cos(ph)), static_cast<float>(std::sin(ph)));
    }
    for (int y = 0; y < H; ++y)
    {
        cv::Vec2f *row = F.ptr<cv::Vec2f>(y);
        const cv::Vec2f cy = ry[y];
        for (int x = 0; x < W; ++x)
        {
            const cv::Vec2f rxv = rx[x];
            const float pr = rxv[0] * cy[0] - rxv[1] * cy[1];   // ramp = rx * cy  (complex)
            const float pi = rxv[0] * cy[1] + rxv[1] * cy[0];
            const cv::Vec2f v = row[x];
            row[x] = cv::Vec2f(v[0] * pr - v[1] * pi, v[0] * pi + v[1] * pr);
        }
    }

    cv::Mat out;
    cv::idft(F, out, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
    return out;
}

/// Least-squares fit of  b(f) ~= sum_{k=1..K} a_k sin(2 pi k f) + c_k cos(2 pi k f).
/// Returns [a1,c1,a2,c2,...] (2K), or empty if there are too few samples.
std::vector<double> fitHarmonics(const std::vector<double> &f, const std::vector<double> &b, int K)
{
    const int n = static_cast<int>(f.size());
    if (n < 2 * K + 2)
        return {};

    cv::Mat A(n, 2 * K, CV_64F), rhs(n, 1, CV_64F);
    for (int i = 0; i < n; ++i)
    {
        for (int k = 1; k <= K; ++k)
        {
            A.at<double>(i, 2 * (k - 1))     = std::sin(2.0 * CV_PI * k * f[i]);
            A.at<double>(i, 2 * (k - 1) + 1) = std::cos(2.0 * CV_PI * k * f[i]);
        }
        rhs.at<double>(i, 0) = b[i];
    }

    cv::Mat c;
    if (!cv::solve(A, rhs, c, cv::DECOMP_SVD))
        return {};

    std::vector<double> out(2 * K);
    for (int k = 0; k < 2 * K; ++k)
        out[k] = c.at<double>(k, 0);
    return out;
}

/// Normalised cross-correlation (Pearson, in [-1,1]) between `a` and `b` over the
/// region they share once `b` is shifted back by the found translation. An
/// estimator-independent "how good is this match" score for the trust gate.
double nccAtShift(const cv::Mat &a, const cv::Mat &b, cv::Point2d shift)
{
    const int dx = static_cast<int>(std::lround(shift.x));
    const int dy = static_cast<int>(std::lround(shift.y));
    const int w = a.cols - std::abs(dx);
    const int h = a.rows - std::abs(dy);
    if (w < 32 || h < 32)
        return 0.0;

    const cv::Rect ra(std::max(0, -dx), std::max(0, -dy), w, h);
    const cv::Rect rb(std::max(0,  dx), std::max(0,  dy), w, h);
    const cv::Mat A = a(ra), B = b(rb);

    cv::Scalar ma, sa, mb, sb;
    cv::meanStdDev(A, ma, sa);
    cv::meanStdDev(B, mb, sb);
    if (sa[0] < 1e-6 || sb[0] < 1e-6)
        return 0.0;

    const double cov = cv::mean((A - ma[0]).mul(B - mb[0]))[0];
    return cov / (sa[0] * sb[0]);
}

/// Evaluate the fitted harmonic series at fractional shift `frac` in [0,1).
double biasEval(const std::vector<double> &c, double frac)
{
    double s = 0.0;
    const int K = static_cast<int>(c.size()) / 2;
    for (int k = 1; k <= K; ++k)
        s += c[2 * (k - 1)]     * std::sin(2.0 * CV_PI * k * frac)
             + c[2 * (k - 1) + 1] * std::cos(2.0 * CV_PI * k * frac);
    return s;
}

/// Peak-to-peak of the fitted curve over [0,1).
double curvePeakToPeak(const std::vector<double> &c)
{
    if (c.empty())
        return 0.0;
    double lo = 1e30, hi = -1e30;
    for (int i = 0; i < 200; ++i)
    {
        const double v = biasEval(c, i / 200.0);
        lo = std::min(lo, v);
        hi = std::max(hi, v);
    }
    return hi - lo;
}

} // namespace

Meter::Meter(const Params &p)
    : _params(p)
{
}

void Meter::reset()
{
    _anchor.release();
    _anchorRaw.release();
    _haveAnchor = false;
    _anchorOriginX = _anchorOriginY = 0;
    _cumX = _cumY = 0;
    _size = cv::Size(0, 0);
    _biasCoefX.clear();
    _biasCoefY.clear();
    _haveCurve = false;
    _sCurvePPx = _sCurvePPy = 0;
}

void Meter::dropAnchor(const cv::Mat &f32, double originX, double originY)
{
    if (_params.hann)
        cv::multiply(f32, _hannWin, _anchor);
    else
        _anchor = f32.clone();
    _anchorRaw = f32.clone();   // un-windowed, for ECC (not FFT-based, no wraparound)
    _anchorOriginX = originX;
    _anchorOriginY = originY;
    _haveAnchor = true;

    // The pixel-locking S-curve is a property of the estimator + the surface
    // texture, which does not change - so calibrate it once and reuse it for
    // every later anchor.
    if (_params.sCurve && !_haveCurve && !_anchorRaw.empty())
        calibrateSCurve();
}

cv::Point2d Meter::measureShift(const cv::Mat &curWin, const cv::Mat &curRaw, double &response) const
{
    double pcResp = 0.0;
    cv::Point2d shift = cv::phaseCorrelate(_anchor, curWin, cv::noArray(), &pcResp);

    if (_params.eccRefine)
    {
        cv::Mat warp = (cv::Mat_<float>(2, 3) << 1.f, 0.f, static_cast<float>(shift.x),
                        0.f, 1.f, static_cast<float>(shift.y));
        try
        {
            cv::findTransformECC(
                _anchorRaw, curRaw, warp, cv::MOTION_TRANSLATION,
                cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS,
                                 _params.eccIters, _params.eccEps),
                cv::noArray(), std::max(1, _params.eccGaussFilt | 1));
            shift.x = warp.at<float>(0, 2);
            shift.y = warp.at<float>(1, 2);
        }
        catch (const cv::Exception &)
        {
            // ECC failed to converge - keep the phase-correlation result.
        }
    }

    // Estimator-independent trust score: normalised cross-correlation of the
    // anchor and the current frame at the found shift. Same meaning whatever the
    // sub-pixel method.
    response = nccAtShift(_anchorRaw, curRaw, shift);
    return shift;
}

void Meter::calibrateSCurve()
{
    const int N = std::max(8, _params.sCurvePoints);
    const int K = std::max(1, _params.sCurveHarm);

    std::vector<double> fX, bX, fY, bY;

    for (int i = 0; i < N; ++i)
    {
        const double d = (i + 0.5) / N;   // fractional shift in (0,1)
        double resp = 0.0;

        // X axis: anchor shifted by (d, 0)
        {
            cv::Mat sh = fourierShift(_anchorRaw, d, 0.0);
            cv::Mat win;
            if (_params.hann) cv::multiply(sh, _hannWin, win);
            else              win = sh;
            const cv::Point2d m = measureShift(win, sh, resp);
            if (resp > 0.2 && std::fabs(m.x - d) < 0.5 && std::fabs(m.y) < 0.5)
            {
                fX.push_back(d);
                bX.push_back(m.x - d);
            }
        }
        // Y axis: anchor shifted by (0, d)
        {
            cv::Mat sh = fourierShift(_anchorRaw, 0.0, d);
            cv::Mat win;
            if (_params.hann) cv::multiply(sh, _hannWin, win);
            else              win = sh;
            const cv::Point2d m = measureShift(win, sh, resp);
            if (resp > 0.2 && std::fabs(m.y - d) < 0.5 && std::fabs(m.x) < 0.5)
            {
                fY.push_back(d);
                bY.push_back(m.y - d);
            }
        }
    }

    _biasCoefX = fitHarmonics(fX, bX, K);
    _biasCoefY = fitHarmonics(fY, bY, K);
    _sCurvePPx = curvePeakToPeak(_biasCoefX);
    _sCurvePPy = curvePeakToPeak(_biasCoefY);

    // If the fit is degenerate or implausibly large, don't apply it.
    const bool ok = !_biasCoefX.empty() && !_biasCoefY.empty()
                    && _sCurvePPx < 0.8 && _sCurvePPy < 0.8;
    _haveCurve = ok;
    if (!ok)
    {
        _biasCoefX.clear();
        _biasCoefY.clear();
    }
}

Sample Meter::update(const cv::Mat &frame)
{
    Sample s;

    if (frame.empty())
        return s;

    // --- normalise to single-channel CV_32F ---------------------------------
    cv::Mat mono;
    if (frame.channels() == 1)
        mono = frame;
    else
        cv::cvtColor(frame, mono, cv::COLOR_BGR2GRAY);

    cv::Mat f32;
    mono.convertTo(f32, CV_32F);

    // A little Gaussian blur widens the correlation peak and reduces the
    // pixel-locking (peak-locking) bias of any sub-pixel step.
    if (_params.preBlur > 0.0)
        cv::GaussianBlur(f32, f32, cv::Size(0, 0), _params.preBlur);

    // Size change (or first frame) -> (re)build the working state.
    if (f32.size() != _size)
    {
        _size = f32.size();
        if (_params.hann)
            cv::createHanningWindow(_hannWin, _size, CV_32F);
        _cumX = _cumY = 0;
        dropAnchor(f32, 0.0, 0.0);
        return s;   // first frame: anchor only (also runs the S-curve calibration)
    }

    if (!_haveAnchor)
    {
        dropAnchor(f32, 0.0, 0.0);
        return s;
    }

    // --- register against the anchor -----------------------------------------
    cv::Mat cur;
    if (_params.hann)
        cv::multiply(f32, _hannWin, cur);
    else
        cur = f32;

    double response = 0.0;
    cv::Point2d shift = measureShift(cur, f32, response);

    // --- subtract the calibrated pixel-locking S-curve ----------------------
    if (_haveCurve)
    {
        shift.x -= biasEval(_biasCoefX, shift.x - std::floor(shift.x));
        shift.y -= biasEval(_biasCoefY, shift.y - std::floor(shift.y));
    }

    // Displacement of the scene from the anchor, then absolute since reset().
    const double absX = _anchorOriginX + shift.x;
    const double absY = _anchorOriginY + shift.y;

    s.stepX    = absX - _cumX;
    s.stepY    = absY - _cumY;
    s.shiftX   = shift.x;
    s.shiftY   = shift.y;
    s.response = response;

    const double stepMag = std::hypot(s.stepX, s.stepY);

    // --- trust gates -------------------------------------------------------
    if (response < _params.minResponse || stepMag > _params.maxStepPx)
    {
        // Not trusted: report but do NOT advance the cumulative state and do
        // NOT re-anchor on a frame we don't believe.
        s.ok = false;
        s.x  = _cumX;
        s.y  = _cumY;
        return s;
    }

    _cumX = absX;
    _cumY = absY;
    s.ok  = true;
    s.x   = _cumX;
    s.y   = _cumY;

    // --- re-anchor when the running shift gets large ----------------------
    const double lim = _params.reanchorFrac * 0.5 * std::min(_size.width, _size.height);
    if (std::hypot(shift.x, shift.y) > lim)
    {
        dropAnchor(f32, _cumX, _cumY);
        s.reanchored = true;
        // TODO: cross-check the re-anchor (correlate old vs new anchor over the
        // overlap, compare to the accumulated delta) and reject on disagreement.
    }

    return s;
}

} // namespace pecmeter
