#ifndef BLINDPEC_MODULE_h_
#define BLINDPEC_MODULE_h_

/**
 * @file blindpec.h
 * @brief BlindPEC - optical-encoder RA guiding from a USB microscope on the RA axis.
 *
 * See src/modules/blindpec-notes.fr.md for the design rationale. In short: a
 * microscope images a textured disc fixed perpendicular to the RA axis; OpenCV
 * phase correlation (see pecmeter.{h,cpp}) turns the surface translation into a
 * sub-pixel axis-rotation measurement. The module then holds the axis at the
 * sidereal rate by RA-only guide pulses, which:
 *   1. reveals the periodic error, and
 *   2. cancels the RA tracking drift.
 *
 * DEC is intentionally not handled (assumes good polar alignment; what a normal
 * guider would see in DEC here is mostly seeing). Atmospheric refraction makes
 * the target rate slightly ALT/AZ dependent - deferred, see the notes.
 *
 * Model:  p(t) = V.t + e(t) + n(t)
 *   V.t  : constant sidereal ramp  (the "pixsec" constant, hard to calibrate)
 *   e(t) : periodic error (worm), zero mean over one worm period
 *   n(t) : correlation noise + turbulence + re-anchor random walk
 *
 * Phases (single state machine):
 *   Init         connect devices, read mount ALT/AZ + guide rate, reset camera
 *   GainCal      send known W pulses, measure the px jump -> G (ms/px), derive V
 *   Characterize track only for N frames, robust-fit the residual slope -> refine V
 *   Guide        capture -> measure -> P+I control on residual -> RA pulse -> loop
 *                V is slowly adapted online so the integrator never carries the DC.
 *
 * Plumbing (capture / BLOB guard / watchdog / pulses / preview) follows the
 * current guider module.
 */

#include <indimodule.h>
#include <fileio.h>
#include <opencv2/opencv.hpp>
#include "pecmeter.h"

#if defined(BLINDPEC_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

#include <QtCore>
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <vector>
#include <cmath>

class MODULE_INIT BlindPec : public IndiModule
{
        Q_OBJECT

    public:
        BlindPec(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~BlindPec();

        void onNewDevice(INDI::BaseDevice dp) override {}
        void onRemoveDevice(INDI::BaseDevice dp) override {}
        void onNewProperty(INDI::Property property) override {}
        void onRemoveProperty(INDI::Property property) override {}
        void onUpdateProperty(INDI::Property property) override;

    signals:
        void InitDone();
        void Abort();
        void AbortDone();
        void RequestFrameResetDone();
        void FrameResetDone();
        void RequestExposureDone();
        void ExposureDone();
        void MeasureDone();
        void ComputeDone();
        void RequestPulsesDone();
        void PulsesDone();

    protected:
        void onExternalEvent(OST::ExtEvent event) override;

    private:
        // ==================== INDI integration ====================
        void newBLOB(INDI::PropertyBlob pblob);       ///< incoming microscope frame

        QPointer<fileio>       _image;
        FITSImage::Statistic   stats;
        bool                   _expectingFrame = false; ///< true between an exposure request and its BLOB

        QTimer _watchdog;                              ///< aborts if an INDI callback never comes back
        void   armWatchdog();
        void   disarmWatchdog();

        bool   _trace = false;                         ///< verbose per-frame state / property tracing (bring-up only)

        // ==================== Measurement core ====================
        pecmeter::Meter        _meter {};
        double                 _measX = 0;            ///< cumulative displacement, image X (px)
        double                 _measY = 0;            ///< cumulative displacement, image Y (cross axis, px)
        double                 _measResp = 0;         ///< last correlation response
        bool                   _measOk = false;
        bool                   _reanchored = false;
        int                    _consecutiveBad = 0;   ///< consecutive untrusted frames

        pecmeter::Params       meterParams();          ///< build the meter tuning from live guideParams

        // ==================== Phases ====================
        // Flow: Init -> Characterize (step 1: free-run drift -> theta + V)
        //            -> GainCal    (step 2: W/E pulses, decontaminated with V -> G)
        //            -> Guide
        // "calibrate" stops after GainCal; "guide" with a stored G skips GainCal.
        enum Phase { PhInit, PhCharacterize, PhGainCal, PhGuide };
        Phase _phase = PhInit;
        bool  _calibrateOnly    = false;               ///< "calibrate" button pressed
        bool  _skipGainCal      = false;               ///< reuse a stored gain (guide with existing calibration)
        bool  _stopAfterGainCal = false;               ///< calibration-only run
        bool  _finishOk         = false;               ///< SMAbort reached from a clean finish, not an abort

        void  setActionRunning(const QString &running);///< clear all action buttons, light `running` (guider pattern)

        void computeCharacterize();
        void computeGainCal();
        void computeGuide();

        // ---- step 1: free-run characterization ----
        int    _charFrames  = 0;                       ///< frames collected in the Characterize phase
        std::vector<double> _charT;                    ///< time samples (s since phase start)
        std::vector<double> _charX;                    ///< cumulative image displacement, X (px)
        std::vector<double> _charY;                    ///< cumulative image displacement, Y (px)
        double _phaseT0     = 0;                       ///< epoch (ms) of the current phase start
        double _theta       = 0;                       ///< RA axis direction in the image (rad), from the free-run drift
        double _V           = 0;                       ///< target rate along +theta, px/s (>= 0)
        void   fitDriftLine();                         ///< (_charT,_charX,_charY) -> _theta, _V

        // ---- step 2: pulse gain calibration ----
        int    _gainStep    = 0;                       ///< pulses sent so far (alternating W,E,W,E,...)
        int    _gainPending  = 0;                      ///< direction of the pulse awaiting measurement: -1 W, +1 E, 0 none
        double _gainSnapP   = 0;                       ///< projected position snapshot before that pulse (px)
        double _gainSnapT   = 0;                       ///< epoch (ms) of that snapshot
        std::vector<double> _gainWeff;                 ///< W pulse-only effects (px per calpulse ms, signed along +theta)
        std::vector<double> _gainEeff;                 ///< E pulse-only effects
        double _G           = 0;                       ///< gain magnitude, ms of pulse per px along the RA axis
        int    _wDir        = -1;                      ///< sign of a W pulse's effect along +theta (+1 or -1)
        double _arcsecPerPx = 0;                       ///< display-only scale (needs mount guide rate)
        double _guideRateK  = 0.5;                     ///< mount guide rate, x sidereal (fallback 0.5)

        // ---- guiding control ----
        double _t0Guide     = 0;                       ///< epoch (ms) of guiding start
        double _tPrev       = 0;                       ///< previous guide-frame time (s since guiding start)
        double _residual    = 0;                       ///< p - V.t on the RA axis (px)
        double _residualPrev = 0;                      ///< previous frame's error (residual - dither offset)
        double _intR        = 0;                       ///< integral accumulator (px.s)
        bool   _intRsat     = false;                   ///< pulse saturated last frame -> freeze the integrator
        int    _blank       = 0;                       ///< frames left to skip after a pulse (settle)
        int    _lastPulseDir = 0;                      ///< -1 = W, +1 = E, 0 = none
        bool   _ditherPending = false;                 ///< dither requested, apply on the next guide frame
        double _ditherOffset  = 0;                     ///< current RA target bias from dithering (px)

        // ==================== Current pulses (ms) ====================
        int _pulseE = 0;                               ///< speed up RA
        int _pulseW = 0;                               ///< slow down RA
        bool _pulseRAfinished = true;

        // ==================== Mount info ====================
        double _mountRA = 0, _mountDEC = 0;
        double _mountALT = 0, _mountAZ = 0;

        // ==================== RMS (display) ====================
        std::vector<double> _rmsBuf;

        inline double square(double v) { return v * v; }

        // ==================== State machine ====================
        QStateMachine _sm;
        void buildStateMachine();

        // handlers
        void SMInitInit();
        void SMRequestFrameReset();
        void SMRequestExposure();
        void SMMeasure();       ///< frame -> pecmeter -> _measX/_measY
        void SMCompute();       ///< phase logic: gain cal / characterize / guide -> sets _pulseE/_pulseW
        void SMRequestPulses();
        void SMAbort();

        // helpers
        void   publishFrame();                          ///< jpeg preview + image property
        void   pushGuiding(double raArcsec, double rms);
        double nowMs() const { return QDateTime::currentDateTime().toMSecsSinceEpoch(); }
        double projRA(double x, double y)    const { return  x * std::cos(_theta) + y * std::sin(_theta); }
        double projCross(double x, double y) const { return -x * std::sin(_theta) + y * std::cos(_theta); }
        void   enterGuide();                            ///< reset the guiding state and switch to PhGuide
};

extern "C" MODULE_INIT BlindPec *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
