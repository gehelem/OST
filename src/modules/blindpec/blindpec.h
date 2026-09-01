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

        bool   _trace = true;                          ///< verbose state / frame tracing during bring-up

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
        enum Phase { PhInit, PhGainCal, PhCharacterize, PhGuide };
        Phase _phase = PhInit;
        bool  _calibrateOnly = false;                  ///< "calibrate" button: stop after characterization

        void computeGainCal();
        void computeCharacterize();
        void computeGuide();

        // ---- gain calibration ----
        int    _gainStep   = 0;                        ///< current gain-cal pulse index
        double _gainPulseMs = 0;                       ///< pulse length used for gain cal (ms)
        double _gainRefX = 0, _gainRefY = 0;           ///< displacement snapshot before the current gain-cal pulse
        std::vector<double> _gainDeltas;               ///< measured |step| per gain-cal pulse (px)
        double _G          = 0;                        ///< gain, ms of W/E pulse per px of image move
        double _arcsecPerPx = 0;                       ///< display-only scale (needs mount guide rate)
        double _guideRateK  = 0.5;                     ///< mount guide rate, x sidereal (fallback 0.5)

        // ---- characterization / target rate ----
        int    _charFrames  = 0;                       ///< frames collected in the Characterize phase
        std::vector<double> _charT;                    ///< time samples (s since phase start)
        std::vector<double> _charP;                    ///< displacement samples (px, RA axis)
        double _V           = 0;                       ///< target rate, px/s for perfect sidereal tracking
        double _phaseT0     = 0;                       ///< epoch (ms) of the current phase start

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
        void   pushGuiding(double raArcsec, double deArcsec, double rms);
        double nowMs() const { return QDateTime::currentDateTime().toMSecsSinceEpoch(); }
        void   fitTargetRate();                         ///< robust slope of (_charT, _charP) -> _V
};

extern "C" MODULE_INIT BlindPec *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
