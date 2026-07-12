#ifndef SEQUENCER_MODULE_h_
#define SEQUENCER_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>
#include <QScxmlStateMachine>
#include <QElapsedTimer>
#include <QDateTime>

#if defined(SEQUENCER_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT Sequencer: public IndiModule
{
        Q_OBJECT

    public:
        Sequencer(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Sequencer();
        void onNewDevice      (INDI::BaseDevice dp) override     {};
        void onRemoveDevice   (INDI::BaseDevice dp) override     {};
        void onNewProperty    (INDI::Property property) override {};
        void onRemoveProperty (INDI::Property property) override {};
        void onUpdateProperty (INDI::Property property) override {};

    signals:
        void cameraAlert();

    protected:
        void onExternalEvent(OST::ExtEvent event) override;

    public slots:
        void onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line) override;

    private slots:
        // ── Pre-sequence entry points ───────────────────────────────────────
        void SMWaitCooling();     // immediate skip, or waits for CCD_TEMPERATURE IPS_OK

        // ── SequenceCtrl entry points ───────────────────────────────────────
        void SMInitLine();
        void SMFilterStep();
        void SMFocusGate();       // immediate: DoFocus or SkipFocus
        void SMGuideGate();       // immediate: StartGuide or SkipGuide
        void SMDitherGate();      // immediate: DoDither or SkipDither
        void SMWaitSettle();
        void SMExposing();
        void SMFindStars();
        void SMEvalShot();        // immediate: NextShot | LineDone | DoFocus | DoRecal

        // ── FocusCtrl entry points ──────────────────────────────────────────
        void SMFocusing();

        // ── GuideCtrl entry points ──────────────────────────────────────────
        void SMGuideStarting();
        void SMDithering();
        void SMRecalibrating();

        // ── Terminal states ─────────────────────────────────────────────────
        void SMDone();
        void SMAborted();
        void SMError();

        // Timer
        void onGuidingSettleTimeout();
        void onElapsedTimerTick();

    private:
        void newBLOB(INDI::PropertyBlob pblob);
        void OnSucessSEP();
        void newProperty(INDI::Property property) override;
        void updateProperty(INDI::Property property) override;
        void newExp(INDI::PropertyNumber exp);
        void refreshFilterLov();
        void setupOutputFolder();
        void recomputeSequenceTotals();
        static QString formatDuration(double seconds);

        QScxmlStateMachine  *pMachine     = nullptr;
        QTimer              *mSettleTimer = nullptr;
        QTimer              *mElapsedTimer = nullptr;

        // ── Per-sequence state ──────────────────────────────────────────────
        int     mCurrentLine           = -1;
        int     mShotCount             = 0;
        int     mShotsSinceDither      = 0;
        bool    mFilterChanged         = false;
        bool    mGuiderActive          = false; // true when guider module is confirmed guiding
        bool    mGuiderWasSuspended    = false; // true when WE aborted the guider for focus
        QString mCurrentFilter;
        QString mCurrentFilterIndex;   // raw LOV key (sequence.filter), for focusprofiles lookup
        QString mPreviousFilter;
        QString mCurrentFrameType;
        QString mCurrentFolder;
        QString mObjectName           = "default";
        QString mDate;

        // ── Whole-sequence progress (computed at start, updated per shot) ────
        int      mTotalShots           = 0;
        int      mShotsCompleted       = 0;
        double   mTotalEstimatedSeconds = 0.0;
        double   mSecondsCompleted     = 0.0;
        QDateTime mSequenceStartTime;

        // ── Focus memory (module lifetime — NOT reset on sequence start) ─────
        // Lets "focus on filter change" also cover the very first line of a
        // sequence, and skip a redundant focus if the same sequence is
        // re-launched without the filter having changed since the last focus.
        bool    mHasFocusedOnce   = false;
        QString mLastFocusedFilter;

        // ── Focus/guide wait timing (module lifetime — NOT reset on sequence
        //    start, only when the module itself is (re)loaded) ────────────────
        QElapsedTimer mFocusTimer;
        double        mFocusDurationSum = 0.0;
        int           mFocusSamples     = 0;
        QElapsedTimer mGuideStartTimer;
        double        mGuideDurationSum = 0.0;
        int           mGuideSamples     = 0;

        // ── Per-exposure monitoring ─────────────────────────────────────────
        double  mLastHFR              = 0.0;
        double  mMaxRMSDuringExposure = 0.0;

        // ── Image processing ────────────────────────────────────────────────
        QPointer<fileio>     _image;
        Solver               _solver;
        FITSImage::Statistic stats;
};

extern "C" MODULE_INIT Sequencer *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
