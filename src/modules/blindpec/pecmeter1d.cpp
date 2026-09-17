#include "pecmeter1d.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace pecmeter1d
{

namespace
{

/// Signed DFT frequency of bin k for an n-point transform (Nyquist bin = -n/2).
/// Same convention as pecmeter.cpp's dftFreq - duplicated here to keep this
/// prototype self-contained (no dependency on the production estimator).
inline double dftFreq(int k, int n)
{
    return (2 * k < n) ? static_cast<double>(k) : static_cast<double>(k - n);
}

/// Phase kernel for the upsampled inverse DFT, following the reference `dftups`
/// (Guizar-Sicairos MATLAB code). Shape S x n, CV_64FC2. Same convention as
/// pecmeter.cpp's phaseKernel (verified sign there against a Fourier-shift
/// bench); reused verbatim, just duplicated for isolation.
///   K[s, k] = exp( -i.2pi / (n.kappa) * (s - offset) * dftFreq(k, n) )
cv::Mat phaseKernel(int n, int S, double kappa, double offset)
{
    cv::Mat K(S, n, CV_64FC2);
    const double scale = -2.0 * CV_PI / (n * kappa);
    for (int s = 0; s < S; ++s)
    {
        cv::Vec2d *row = K.ptr<cv::Vec2d>(s);
        const double a = (s - offset) * scale;
        for (int k = 0; k < n; ++k)
        {
            const double ph = a * dftFreq(k, n);
            row[k] = cv::Vec2d(std::cos(ph), std::sin(ph));
        }
    }
    return K;
}

} // namespace

cv::Mat project(const cv::Mat &frame, double thetaRad)
{
    cv::Mat f32;
    if (frame.type() != CV_32F)
        frame.convertTo(f32, CV_32F);
    else
        f32 = frame;

    cv::Mat rotated;
    if (std::fabs(thetaRad) < 1e-9)
    {
        rotated = f32;
    }
    else
    {
        // Rotate so the axis of interest lands on the image X axis, then sum
        // columns. getRotationMatrix2D takes degrees, CCW positive.
        const cv::Point2f c(f32.cols / 2.0f, f32.rows / 2.0f);
        const cv::Mat R = cv::getRotationMatrix2D(c, thetaRad * 180.0 / CV_PI, 1.0);
        cv::warpAffine(f32, rotated, R, f32.size(), cv::INTER_LINEAR, cv::BORDER_REPLICATE);
    }

    cv::Mat profile;
    cv::reduce(rotated, profile, 0, cv::REDUCE_SUM, CV_32F);   // 1 x N
    return profile;
}

double upsampledShift(const cv::Mat &ref, const cv::Mat &cur, double kappa)
{
    const int N = ref.cols;

    // --- normalised cross-power spectrum  R = F_cur . conj(F_ref) -----------
    cv::Mat Fr, Fc;
    cv::dft(ref, Fr, cv::DFT_COMPLEX_OUTPUT | cv::DFT_ROWS);
    cv::dft(cur, Fc, cv::DFT_COMPLEX_OUTPUT | cv::DFT_ROWS);

    cv::Mat R;
    cv::mulSpectrums(Fc, Fr, R, 0, /*conjB=*/true);
    {
        std::vector<cv::Mat> ch(2);
        cv::split(R, ch);
        cv::Mat mag;
        cv::magnitude(ch[0], ch[1], mag);
        mag += 1e-12f;
        cv::divide(ch[0], mag, ch[0]);
        cv::divide(ch[1], mag, ch[1]);
        cv::merge(ch, R);
    }

    // --- coarse integer peak --------------------------------------------
    cv::Mat cc;
    cv::idft(R, cc, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE | cv::DFT_ROWS);
    cv::Point peak;
    cv::minMaxLoc(cc, nullptr, nullptr, nullptr, &peak);
    const double dx0 = (peak.x > N / 2) ? peak.x - N : peak.x;

    // --- local upsampled zoom around the coarse peak ------------------
    kappa = std::max(1.0, kappa);
    const int    S    = std::max(3, static_cast<int>(std::ceil(1.5 * kappa)));
    const int    dc   = S / 2;
    const double coff = dc - dx0 * kappa;

    // Same conj(R)-before-the-zoom convention as the 2D estimator (verified
    // there against a Fourier-shift bench - without it the zoom peak leaves
    // the grid).
    cv::Mat R64;
    {
        std::vector<cv::Mat> ch(2);
        cv::split(R, ch);
        ch[1] *= -1.0f;
        cv::Mat Rc;
        cv::merge(ch, Rc);
        Rc.convertTo(R64, CV_64FC2);
    }

    const cv::Mat K = phaseKernel(N, S, kappa, coff);           // S x N
    cv::Mat out;
    cv::gemm(K, R64.reshape(2, N), 1.0, cv::noArray(), 0.0, out); // (S x N)*(N x 1) = S x 1

    cv::Mat mag;
    {
        std::vector<cv::Mat> ch(2);
        cv::split(out, ch);
        cv::magnitude(ch[0], ch[1], mag);   // S x 1
    }
    cv::Point up;
    cv::minMaxLoc(mag, nullptr, nullptr, nullptr, &up);   // up.y in [0, S)

    return dx0 + static_cast<double>(up.y - dc) / kappa;
}

} // namespace pecmeter1d
