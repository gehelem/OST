#ifndef GUIDER_MODULE_h_
#define GUIDER_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>

#if defined(GUIDER_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

#include <QtCore>
#include <QtConcurrent>
#include <QStateMachine>


/**
 * @struct Trig
 * @brief Represents a triangle formed by 3 stars for invariant-based matching
 *
 * Used in trigonometric matching algorithm to identify the same stars across
 * multiple exposures. Triangles are invariant under translation, rotation, and
 * scaling, making them robust for tracking star field changes.
 *
 * The ratio (surface/perimeter) is a unique fingerprint for each triangle.
 */
struct Trig
{
    double x1, y1;      // Position of first star
    double x2, y2;      // Position of second star
    double x3, y3;      // Position of third star
    double d12;         // Distance between star 1 and 2
    double d13;         // Distance between star 1 and 3
    double d23;         // Distance between star 2 and 3
    double p;           // Perimeter of triangle (d12 + d13 + d23)
    double s;           // Surface area of triangle
    double ratio;       // Fingerprint = surface/perimeter (invariant under scaling)
};

/**
 * @struct MatchedPair
 * @brief Represents a successfully matched star between reference and current frame
 *
 * When a star is found in both reference frame and current frame, we record
 * the drift (dx, dy) which is used to calculate telescope tracking error.
 */
struct MatchedPair
{
    double xr, yr;      // Reference frame star position
    double xc, yc;      // Current frame star position
    double dx;          // Drift in X axis (pixels) = xc - xr
    double dy;          // Drift in Y axis (pixels) = yc - yr
};

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
        void onUpdateProperty (INDI::Property property) override {} ;

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
        void updateProperty(INDI::Property p) override;  ///< Handle INDI property updates
        void newBLOB(INDI::PropertyBlob pblob);          ///< Handle incoming CCD images

        // ==================== Image Processing ====================
        QPointer<fileio> _image;       ///< Current image from CCD
        Solver _solver;                ///< Star detection and centroid solver
        FITSImage::Statistic stats;    ///< Image statistics
        QList<FITSImage::Star> starsFirst; ///< image reference stars

        // ==================== Current Guiding Pulses (ms) ====================
        int _pulseN = 0;  ///< Pulse to send North (positive DEC)
        int _pulseS = 0;  ///< Pulse to send South (negative DEC)
        int _pulseE = 0;  ///< Pulse to send East (positive RA, compensated for DEC)
        int _pulseW = 0;  ///< Pulse to send West (negative RA, compensated for DEC)

        // ==================== Calibration Results (pixels/ms per 1000ms pulse) ====================
        double _calPulseN = 300;   ///< Calibration: ms per pixel pulse North
        double _calPulseS = 300;   ///< Calibration: ms per pixel pulse South
        double _calPulseE = 300;   ///< Calibration: ms per pixel pulse East
        double _calPulseW = 300;   ///< Calibration: ms per pixel pulse West
        double _calPulseRA = 0;    ///< Calibration: result for RA (unused currently)
        double _calPulseDEC = 0;   ///< Calibration: result for DEC (unused currently)

        // ==================== Calibration State Machine Variables ====================
        int _calState = 0;      ///< Calibration phase counter (0-2)
        int _calStep = 0;       ///< Current calibration pulse direction (0-7 for 4 directions × 2 iterations)
        bool _pulseRAfinished = true;   ///< Flag: RA pulse completed on mount
        bool _pulseDECfinished = true;  ///< Flag: DEC pulse completed on mount
        bool _doDither = false;         ///< Flag: dither requested, apply on next ComputeGuide

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
        double _ccdOrientation;             ///< CCD rotation angle (degrees, from polynomial fit)
        double _calCcdOrientation;          ///< CCD orientation at time of calibration
        double _calMountDEC = 0;            ///< Mount DEC at calibration time (for compensation)
        double _ccdSampling = 206 * 5.2 / 800;  ///< arcsec/pixel (telescope-dependent, may need config)
        int _itt = 0;  ///< Iteration counter

        // ==================== State Machines (3 phases: Init → Calibration → Guiding) ====================
        QStateMachine *_machine;        ///< Pointer to active state machine (unused currently)
        QStateMachine _SMInit;          ///< State machine: Connection and star reference detection
        QStateMachine _SMCalibration;   ///< State machine: Calibration (measure pulse offsets)
        QStateMachine _SMGuide;         ///< State machine: Continuous guiding loop

        // ==================== Trigonometric Indices (for star matching) ====================
        QVector<Trig> _trigFirst;       ///< Triangle indices from FIRST image (reference)
        QVector<Trig> _trigPrev;        ///< Triangle indices from PREVIOUS image (calibration)
        QVector<Trig> _trigCurrent;     ///< Triangle indices from CURRENT image
        QVector<MatchedPair> _matchedCurPrev;   ///< Stars matched: current vs previous
        QVector<MatchedPair> _matchedCurFirst;  ///< Stars matched: current vs first (overall drift)

        // ==================== Calibration Data Collection (for polynomial fitting) ====================
        std::vector<double> _dxvector;     ///< X drifts during calibration (for orientation calc)
        std::vector<double> _dyvector;     ///< Y drifts during calibration
        std::vector<double> _coefficients; ///< Polynomial coefficients (for CCD orientation)

        // ==================== RMS Tracking (for statistics) ====================
        std::vector<double> _dRAvector;    ///< RMS history of RA drifts (bounded by rmsOver parameter from guideParams)
        std::vector<double> _dDEvector;    ///< RMS history of DEC drifts (bounded by rmsOver parameter from guideParams)

        // ==================== Private Methods ====================

        /// @brief Build triangle indices from detected stars using solver
        /// @param solver Star detection engine
        /// @param trig Output: vector of triangles (will be cleared and filled)
        void buildIndexes(Solver &solver, QVector<Trig> &trig);

        /// @brief Match reference triangles with current triangles and calculate drift
        /// @param ref Reference frame triangles
        /// @param act Current frame triangles
        /// @param pairs Output: matched star pairs with drift values
        /// @param dx Output: mean X drift (pixels)
        /// @param dy Output: mean Y drift (pixels)
        void matchIndexes(QVector<Trig> ref, QVector<Trig> act, QVector<MatchedPair> &pairs, double &dx, double &dy);

        /// @brief Helper: compute v²
        inline double square(double value)
        {
            return value * value;
        }

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
