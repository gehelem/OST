#ifndef GUIDER_MODULE_h_
#define GUIDER_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>
#include "startracker.h"

#if defined(GUIDER_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

#include <QtCore>
#include <QtConcurrent>
#include <QStateMachine>
#include <QFinalState>
#include <QState>


class MODULE_INIT Guider  : public IndiModule
{
        Q_OBJECT

    public:
        Guider (QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Guider();
        void onNewDevice      (INDI::BaseDevice dp) override     {} ;
        void onRemoveDevice   (INDI::BaseDevice dp) override     {} ;
        void onNewProperty    (INDI::Property property) override {} ;
        void onRemoveProperty (INDI::Property property) override {} ;
        void onUpdateProperty (INDI::Property property) override;

    signals:
        void InitDone();
        void InitCalDone();
        void InitGuideDone();
        void AbortDone();
        void Abort();
        void RequestFrameResetDone();
        void FrameResetDone();
        void RequestExposureDone();
        void ExposureDone();
        void FindStarsDone();
        void RequestPulsesDone();
        void PulsesDone();
        void ComputeFirstDone();
        void ComputeCalDone();
        void ComputeGuideDone();
        void CalibrationDone();
        void DitherNow();


    public slots:
        /// @brief Called when solver completes star extraction from image
        void OnSucessSEP();
    protected:
        void onExternalEvent(OST::ExtEvent event) override;
    private:
        // ==================== INDI Integration ====================
        void newBLOB(INDI::PropertyBlob pblob);          ///< Handle incoming CCD images

        // ==================== Image Processing ====================
        QPointer<fileio> _image;       ///< Current image from CCD
        Solver _solver;                ///< Star detection and centroid solver
        FITSImage::Statistic stats;    ///< Image statistics
        QList<FITSImage::Star> starsFirst;  ///< fixed reference star field (also the green overlay)
        QList<FITSImage::Star> _prevStars;  ///< previous calibration frame (for the per-pulse increment)
        int _consecutiveMatchFail = 0;      ///< consecutive lost-correlation frames during guiding

        QTimer _watchdog;                   ///< aborts if an INDI callback never comes back
        bool _expectingFrame = false;       ///< true between an exposure request and its BLOB

        // PI controller integral accumulators (pixels), reset at guide start / dither
        double _intRA = 0;
        double _intDE = 0;
        bool   _intRAsat = false;           ///< RA pulse saturated last frame -> freeze the RA integrator
        bool   _intDEsat = false;

        // ==================== Current Guiding Pulses (ms) ====================
        int _pulseN = 0;  ///< Pulse to send North (positive DEC)
        int _pulseS = 0;  ///< Pulse to send South (negative DEC)
        int _pulseE = 0;  ///< Pulse to send East (positive RA, compensated for DEC)
        int _pulseW = 0;  ///< Pulse to send West (negative RA, compensated for DEC)

        // ==================== DEC Backlash Compensation State ====================
        int    _lastDecDir            = 0;     ///< Direction of the last DEC pulse actually sent: -1 South, +1 North, 0 none yet
        bool   _decBacklashLearning   = false;  ///< True while waiting to measure the effect of a reversal pulse
        int    _decBacklashDir        = 0;      ///< Direction of the pending reversal pulse being learned from
        double _decBacklashCorrection = 0;      ///< Correction-only portion (ms, before backlash was added) of that pulse
        double _decBacklashLastDriftDE = 0;     ///< DEC drift (px) measured just before that pulse was sent

        // ==================== Calibration Results (ms per pixel) ====================
        double _calPulseRA = 0;    ///< RA rate used for guiding (drift-cancelled, West-East combination)
        double _calPulseDEC = 0;   ///< DEC rate used for guiding (North-South combination)
        double _calPulseN = 300;   ///< raw per-direction rate, diagnostic only (still carries background drift)
        double _calPulseS = 300;
        double _calPulseE = 300;
        double _calPulseW = 300;
        double _calPassMean[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}}; ///< mean (ref-cur) increment per pass: 0=W 1=E 2=N 3=S

        // ==================== Calibration State Machine Variables ====================
        int _calState = 0;      ///< Calibration phase counter (0-2)
        int _calStep = 0;       ///< Current calibration pulse direction (0-7 for 4 directions × 2 iterations)
        int _calRetry = 0;      ///< consecutive calibration attempts rejected for poor quality
        bool _pulseRAfinished = true;   ///< Flag: RA pulse completed on mount
        bool _pulseDECfinished = true;  ///< Flag: DEC pulse completed on mount
        bool _doDither = false;         ///< Flag: dither requested, apply on next ComputeGuide
        bool _firstGuideFrame = false;  ///< first frame of a guiding session: measure, don't correct
        bool _calAwaitingKick = false;  ///< Flag: last pulse was a DEC backlash kick, discard its measurement

        // ==================== Drift Measurements (pixels) ====================
        double _dxFirst = 0;    ///< Drift X from first reference frame
        double _dyFirst = 0;    ///< Drift Y from first reference frame
        double _dxPrev = 0;     ///< Drift X from previous frame (used for current pulse calculation)
        double _dyPrev = 0;     ///< Drift Y from previous frame

        // ==================== Telescope/Mount Information ====================
        double _mountDEC;                   ///< Current mount DEC (degrees, for cos() compensation)
        double _mountRA;                    ///< Current mount RA (degrees)
        bool _mountPointingWest = false;    ///< True if pier side = West (affects CCD orientation)
        bool _calMountPointingWest = false; ///< Pier side at time of calibration
        double _calCcdOrientation = 0;      ///< CCD orientation (radians) from calibration
        double _calMountDEC = 0;            ///< Mount DEC at calibration time (for compensation)
        int _itt = 0;  ///< Iteration counter

        // ==================== State Machines (3 phases: Init → Calibration → Guiding) ====================
        QStateMachine _SMInit;          ///< State machine: Connection and star reference detection
        QStateMachine _SMCalibration;   ///< State machine: Calibration (measure pulse offsets)
        QStateMachine _SMGuide;         ///< State machine: Continuous guiding loop

        // ==================== Calibration Data Collection (for polynomial fitting) ====================
        std::vector<double> _dxvector;     ///< X drifts during calibration (for rate + orientation calc)
        std::vector<double> _dyvector;     ///< Y drifts during calibration

        // ==================== RMS Tracking (for statistics) ====================
        std::vector<double> _dRAvector;    ///< RMS history of RA drifts (bounded by rmsOver parameter from guideParams)
        std::vector<double> _dDEvector;    ///< RMS history of DEC drifts (bounded by rmsOver parameter from guideParams)

        // ==================== Private Methods ====================

        /// @brief Helper: compute v²
        inline double square(double value)
        {
            return value * value;
        }

        /// @brief Build the star-tracker tuning from the live guideParams.
        startracker::Params trackParams();

        void armWatchdog();     ///< start the hardware-response timeout (no-op if guideParams/watchdog == 0)
        void disarmWatchdog();  ///< the awaited callback arrived

        // ==================== State Machine Builders ====================
        void buildInitStateMachines(void);          ///< Build _SMInit state machine
        void buildCalStateMachines(void);           ///< Build _SMCalibration state machine
        void buildGuideStateMachines(void);         ///< Build _SMGuide state machine

        // ==================== State Machine Handlers ====================
        // INITIALIZATION PHASE
        void SMInitInit(void);          ///< Connect INDI devices, reset CCD, get mount position
        void SMRequestFrameReset(void); ///< Request CCD frame reset
        void SMRequestExposure(void);   ///< Request exposure from camera
        void SMFindStars(void);         ///< Detect stars in image (via Solver)
        void SMComputeFirst(void);      ///< Build reference triangle indices from first image

        // CALIBRATION PHASE
        void SMInitCal(void);           ///< Initialize calibration (clear counters)
        void SMRequestPulses(void);     ///< Send N/S/E/W test pulses to mount
        void SMComputeCal(void);        ///< Measure pixel offset from test pulse

        // GUIDING PHASE
        void SMInitGuide(void);         ///< Load calibration, setup DEC compensation
        void SMComputeGuide(void);      ///< Calculate required pulses from measured drift

        // ABORT
        void SMAbort(void);             ///< Emergency stop all operations
};

extern "C" MODULE_INIT Guider *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
