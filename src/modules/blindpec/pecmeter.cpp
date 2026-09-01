#include "pecmeter.h"

#include <cmath>

namespace pecmeter
{

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

    // A little Gaussian blur widens the correlation peak and strongly reduces
    // the pixel-locking (peak-locking) bias of any centroid-based sub-pixel step.
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
        return s;   // first frame: anchor only
    }

    if (!_haveAnchor)
    {
        dropAnchor(f32, 0.0, 0.0);
        return s;
    }

    // --- register against the anchor ------------------------------------
    cv::Mat cur;
    if (_params.hann)
        cv::multiply(f32, _hannWin, cur);
    else
        cur = f32;

    double response = 0.0;
    // Coarse shift from phase correlation (fast, robust for the integer part).
    cv::Point2d shift = cv::phaseCorrelate(_anchor, cur, cv::noArray(), &response);

    // Sub-pixel refinement with ECC: it maximises a continuous, illumination-
    // normalised correlation criterion via bilinear interpolation, so its S-curve
    // (pixel-locking) bias is far smaller than a phase-correlation peak centroid.
    // Seed it with the coarse shift; the residual it solves is well under 1 px.
    if (_params.eccRefine)
    {
        cv::Mat warp = (cv::Mat_<float>(2, 3) << 1.f, 0.f, static_cast<float>(shift.x),
                        0.f, 1.f, static_cast<float>(shift.y));
        try
        {
            const double cc = cv::findTransformECC(
                                  _anchorRaw, f32, warp, cv::MOTION_TRANSLATION,
                                  cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS,
                                          _params.eccIters, _params.eccEps),
                                  cv::noArray(), std::max(1, _params.eccGaussFilt | 1));
            shift.x  = warp.at<float>(0, 2);
            shift.y  = warp.at<float>(1, 2);
            response = cc;   // ECC coefficient in [0,1]
        }
        catch (const cv::Exception &)
        {
            // ECC failed to converge - keep the phase-correlation result.
        }
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

    // --- trust gates -----------------------------------------------------
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

    // --- re-anchor when the running shift gets large ---------------------
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
