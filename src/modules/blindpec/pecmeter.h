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

namespace pecmeter
{

struct Params
{
    bool   hann         = true;  ///< apply a Hann window before correlation (kills edge wrap-around)
    double reanchorFrac  = 0.35; ///< re-anchor once |shift vs anchor| exceeds this * min(w,h)/2
    double maxStepPx     = 40.0; ///< reject a frame whose move since the last accepted one exceeds this (vibration / glitch)
    double minResponse   = 0.10; ///< phaseCorrelate response below this -> frame not trusted
};

struct Sample
{
    bool   ok         = false;  ///< measurement trusted (false: caller should skip the frame, not abort)
    double x          = 0;      ///< cumulative displacement along image X since reset(), px
    double y          = 0;      ///< cumulative displacement along image Y since reset(), px
    double stepX      = 0;      ///< move along X since the previous accepted frame, px
    double stepY      = 0;      ///< move along Y since the previous accepted frame, px
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

    private:
        void dropAnchor(const cv::Mat &f32, double originX, double originY);

        Params  _params;
        cv::Mat _anchor;        ///< CV_32F reference frame (already windowed if hann)
        cv::Mat _hannWin;       ///< cached Hann window for the working size
        cv::Size _size {0, 0};  ///< working frame size (0,0 until first frame)

        double _anchorOriginX = 0; ///< cumulative displacement of the current anchor itself
        double _anchorOriginY = 0;
        double _cumX = 0, _cumY = 0; ///< last accepted cumulative displacement
        bool   _haveAnchor = false;
};

} // namespace pecmeter
