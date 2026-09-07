/**
 * @file pecmeter.h
 * @brief Isolated PE-meter: measures the RA-axis rotation as an image translation.
 *
 * BlindPEC watches, through a USB microscope, a textured surface fixed on and
 * perpendicular to the mount's RA axis (a sanded CD, pressed onto the shaft).
 * As the axis rotates the surface texture streams across the field; that
 * translation IS the axis rotation. This class turns a stream of frames into a
 * cumulative sub-pixel displacement using OpenCV phase correlation.
 *
 * Deliberately free of any Qt / INDI / StellarSolver dependency so it can be
 * unit-tested in isolation: feed it `cv::Mat` frames, read back `Sample`.
 *
 * Displacement is measured against an anchor frame (bounded noise), not
 * frame-to-frame (random walk). When the running shift against the anchor gets
 * large the anchor is re-dropped and its own accumulated displacement is carried
 * over, so `x`/`y` stay continuous across re-anchors.
 */
#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

namespace pecmeter
{

struct Params
{
    bool   hann         = true;  ///< apply a Hann window before correlation (kills edge wrap-around)
    double preBlur      = 1.0;   ///< Gaussian sigma (px) applied before correlation; 0 = off. A little blur cuts pixel-locking
    bool   eccRefine    = true;  ///< refine the sub-pixel shift with findTransformECC (interpolation-based, low pixel-locking)
    int    eccIters     = 60;    ///< ECC max iterations
    double eccEps       = 1e-4;  ///< ECC convergence epsilon (on the correlation coefficient)
    int    eccGaussFilt = 5;     ///< ECC internal Gaussian kernel size (images + gradients); larger = smoother, less pixel-locking
    bool   sCurve       = true;  ///< measure the estimator's pixel-locking S-curve (once) and subtract it at runtime
    int    sCurvePoints = 32;    ///< fractional-shift samples per axis for the S-curve fit
    int    sCurveHarm   = 3;     ///< number of harmonics in the S-curve fit (1 hump/px dominates, 2-3 is plenty)
    bool   dftShift       = true; ///< sub-pixel shift by upsampled-DFT cross-correlation (Guizar-Sicairos); false = phaseCorrelate + ECC + S-curve
    int    upsampleFactor = 50;   ///< upsampled-DFT: 1/factor px resolution of the local zoom (kappa). CPU ~ factor^2
    double reanchorFrac  = 0.35; ///< re-anchor once |shift vs anchor| exceeds this * min(w,h)/2
    double maxStepPx     = 40.0; ///< reject a frame whose move since the last accepted one exceeds this (vibration / glitch)
    double minResponse   = 0.10; ///< correlation response below this -> frame not trusted (ECC coeff when eccRefine, else phaseCorrelate response)
};

struct Sample
{
    bool   ok         = false;  ///< measurement trusted (false: caller should skip the frame, not abort)
    double x          = 0;      ///< cumulative displacement along image X since reset(), px
    double y          = 0;      ///< cumulative displacement along image Y since reset(), px
    double stepX      = 0;      ///< move along X since the previous accepted frame, px
    double stepY      = 0;      ///< move along Y since the previous accepted frame, px
    double shiftX     = 0;      ///< raw phaseCorrelate shift vs the current anchor, px (for pixel-locking diagnosis)
    double shiftY     = 0;
    double response   = 0;      ///< phaseCorrelate response (peak sharpness / confidence)
    bool   reanchored = false;  ///< this frame dropped a fresh anchor
};

/**
 * @brief Frame-stream translation tracker (single axis of interest, both image axes reported).
 */
class Meter
{
    public:
        explicit Meter(const Params &p = Params());

        /// Drop the anchor and zero the cumulative displacement.
        void reset();

        /**
         * @brief Feed one frame.
         * @param frame single-channel image, any depth (converted to CV_32F internally).
         *              Every frame must have the same size; a size change forces a reset.
         * @return the measurement for this frame. The first frame after reset() only
         *         seeds the anchor and returns ok == false.
         */
        Sample update(const cv::Mat &frame);

        double cumX() const { return _cumX; }
        double cumY() const { return _cumY; }

        /// S-curve (pixel-locking) calibration state, for the caller to log.
        bool   haveSCurve() const { return _haveCurve; }
        double sCurvePeakToPeakX() const { return _sCurvePPx; } ///< px
        double sCurvePeakToPeakY() const { return _sCurvePPy; } ///< px

    private:
        void dropAnchor(const cv::Mat &f32, double originX, double originY);

        /// Sub-pixel shift of `curRaw` w.r.t. the anchor (phaseCorrelate coarse +
        /// optional ECC). `curWin` is the Hann-windowed version for phaseCorrelate.
        cv::Point2d measureShift(const cv::Mat &curWin, const cv::Mat &curRaw, double &response) const;

        /// Sub-pixel shift of `curWin` w.r.t. the anchor by upsampled-DFT
        /// cross-correlation (Guizar-Sicairos). Both images CV_32F, same size,
        /// already Hann-windowed. Deterministic, no calibration, one parameter.
        cv::Point2d upsampledShift(const cv::Mat &refWin, const cv::Mat &curWin) const;

        /// One-time: measure the estimator's pixel-locking bias vs fractional
        /// shift (Fourier-shifted copies of the anchor) and fit it with harmonics.
        void calibrateSCurve();

        Params  _params;
        cv::Mat _anchor;        ///< CV_32F reference frame (already windowed if hann), for phaseCorrelate
        cv::Mat _anchorRaw;     ///< CV_32F reference frame, un-windowed, for ECC + S-curve generation
        cv::Mat _hannWin;       ///< cached Hann window for the working size
        cv::Size _size {0, 0};  ///< working frame size (0,0 until first frame)

        double _anchorOriginX = 0; ///< cumulative displacement of the current anchor itself
        double _anchorOriginY = 0;
        double _cumX = 0, _cumY = 0; ///< last accepted cumulative displacement
        bool   _haveAnchor = false;

        std::vector<double> _biasCoefX; ///< S-curve harmonic coefficients [a1,c1,a2,c2,...] for the X shift
        std::vector<double> _biasCoefY;
        bool   _haveCurve  = false;
        double _sCurvePPx  = 0, _sCurvePPy = 0; ///< fitted S-curve peak-to-peak (px), diagnostic
};

} // namespace pecmeter
