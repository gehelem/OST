#ifndef SEQUENCER_MODULE_h_
#define SEQUENCER_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>
#include <QScxmlStateMachine>

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

    private:
        void newBLOB(INDI::PropertyBlob pblob);
        void OnSucessSEP();
        void newProperty(INDI::Property property) override;
        void updateProperty(INDI::Property property) override;
        void newExp(INDI::PropertyNumber exp);
        void refreshFilterLov();
        void setupOutputFolder();

        QScxmlStateMachine  *pMachine     = nullptr;
        QTimer              *mSettleTimer = nullptr;

        // ── Per-sequence state ──────────────────────────────────────────────
        int     mCurrentLine           = -1;
        int     mShotCount             = 0;
        int     mShotsSinceDither      = 0;
        bool    mFilterChanged         = false;
        bool    mGuiderActive          = false; // true when guider module is confirmed guiding
        bool    mGuiderWasSuspended    = false; // true when WE aborted the guider for focus
        QString mCurrentFilter;
        QString mPreviousFilter;
        QString mCurrentFrameType;
        QString mCurrentFolder;
        QString mObjectName           = "default";
        QString mDate;

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
