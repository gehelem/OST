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
        void Abort();

    protected:
        void onExternalEvent(OST::ExtEvent event) override;

    public slots:
        void onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line) override;

    private slots:
        // State machine entry points
        void SMInitLine();
        void SMChangingFilter();
        void SMFocusing();
        void SMStartingGuider();
        void SMGuidingSettling();
        void SMExposing();
        void SMProcessShot();
        void SMDone();
        void SMAborted();

        // Timer
        void onGuidingSettleTimeout();

    private:
        void newBLOB(INDI::PropertyBlob pblob);
        void newProperty(INDI::Property property) override;
        void updateProperty(INDI::Property property) override;
        void newExp(INDI::PropertyNumber exp);
        void refreshFilterLov();
        void setupOutputFolder();

        QScxmlStateMachine *pMachine = nullptr;
        QTimer              *mSettleTimer = nullptr;

        // Sequence runtime state
        int     mCurrentLine      = -1;
        int     mShotCount        = 0;
        bool    mFilterChanged    = false;
        bool    mSuspendedGuiding = false;
        QString mCurrentFilter;
        QString mPreviousFilter;
        QString mCurrentFrameType;
        QString mCurrentFolder;
        QString mObjectName = "default";
        QString mDate;

        // Image processing
        QPointer<fileio>    _image;
        Solver              _solver;
        FITSImage::Statistic stats;
};

extern "C" MODULE_INIT Sequencer *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
