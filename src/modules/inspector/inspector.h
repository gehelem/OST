#ifndef INSPECTOR_MODULE_h_
#define INSPECTOR_MODULE_h_
#include <indimodule.h>

#if defined(INSPECTOR_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT InspectorModule : public IndiModule
{
    Q_OBJECT

    public:
        InspectorModule(QString name,QString label,QString profile,QVariantMap availableModuleLibs);
        ~InspectorModule();

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
    public slots:
        void OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData) override;
        void OnSucessSEP();

    private:
        void newNumber(INumberVectorProperty *nvp) override;
        void newBLOB(IBLOB *bp) override;
        void newSwitch(ISwitchVectorProperty *svp) override;



        void SMBuild();
        void SMBuildLoop();
        void SMInit();
        void SMRequestFrameReset();
        void SMRequestExposure();
        void SMFindStars();
        void SMCompute();

        void SMRequestExposureBest();
        void SMComputeResult();
        void SMInitLoopFrame();
        void SMComputeLoopFrame();


        void SMAlert();
        //void SMLoadblob(IBLOB *bp);
        void SMLoadblob();
        void SMAbort();
        void startCoarse();

        /*TextProperty* _devices;
        NumberProperty* _values;
        NumberProperty* _parameters;
        SwitchProperty* _actions;
        ImageProperty* _img;
        GridProperty* _grid;*/


        QString _camera  = "CCD Simulator";
        bool    _newblob;

        QPointer<fileio> _image;
        Solver _solver;
        FITSImage::Statistic stats;

        int    _iterations = 3;
        int    _steps = 3000;
        int    _loopIterations = 2;
        int    _loopIteration;
        double _loopHFRavg;
        double _exposure = 2;

        int    _iteration;
        double _bestpos;
        double _bestposfit;
        double _besthfr;
        QStateMachine _machine;
        QStateMachine _machineLoop;

};

extern "C" MODULE_INIT InspectorModule *initialize(QString name,QString label,QString profile,QVariantMap availableModuleLibs);

#endif
