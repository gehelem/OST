#ifndef SEQUENCER_MODULE_h_
#define SEQUENCER_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>

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

    signals:

        void RequestFrameResetDone();
        void FrameResetDone();
        void RequestExposureDone();
        void ExposureDone();
        void FindStarsDone();
        void NextLoop();
        void LoopFinished();
        void ComputeDone();
        void InitDone();

        void RequestExposureBestDone();
        void ExposureBestDone();
        void ComputeResultDone();
        void InitLoopFrameDone();
        void LoopFrameDone();
        void NextFrame();

        void blobloaded();
        void cameraAlert();
        void AbortDone();
        void Abort();
    protected:
        void onExternalEvent(OST::ExtEvent event) override;

    public slots:
        void OnSucessSEP();
        void OnFocusDone(const QString &eventType, const QString &eventModule, const QString &eventKey,
                         const QVariantMap &eventData);
        void OnGuidingSettleTimeout();

    private:
        void newBLOB(INDI::PropertyBlob pblob);
        void newProperty(INDI::Property property) override;
        void updateProperty(INDI::Property property) override;
        void newExp(INDI::PropertyNumber exp);

        void Shoot();
        void SMAlert();
        //void SMLoadblob(IBLOB *bp);
        void SMLoadblob();
        void SMAbort();

        void StartSequence();
        void StartLine();

        void refreshFilterLov();
        void requestFocus();

        bool    _newblob;

        QPointer<fileio> _image;
        Solver _solver;
        FITSImage::Statistic stats;

        int    _iterations = 3;
        int    _steps = 3000;
        int    _loopIterations = 2;
        int    _loopIteration;
        double _loopHFRavg;

        int currentLine = 0;
        int currentCount = 0;
        double currentExposure = 0;
        int currentGain = 0;
        int currentOffset = 0;
        QString currentFilter = "";
        QString previousFilter = "";
        QString currentFrameType = "";
        QString currentStatus = "";
        QString currentFolder = "";

        QVariantMap mActiveSeq;
        bool isSequenceRunning = false;
        bool mWaitingForFocus = false;
        bool mWaitingForGuidingSettle = false;
        QTimer *mGuidingSettleTimer = nullptr;
        QString mObjectName = "default";
        QString mDate;



};

extern "C" MODULE_INIT Sequencer *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
