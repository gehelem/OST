#ifndef INSPECTOR_MODULE_h_
#define INSPECTOR_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>

#if defined(INSPECTOR_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT Inspector : public IndiModule
{
        Q_OBJECT

    public:
        Inspector(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Inspector();
        void onNewDevice      (INDI::BaseDevice dp) override     {} ;
        void onRemoveDevice   (INDI::BaseDevice dp) override     {} ;
        void onNewProperty    (INDI::Property property) override {} ;
        void onRemoveProperty (INDI::Property property) override {} ;
        void onUpdateProperty (INDI::Property property) override {} ;

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
        void newImage();

    public slots:
        void OnSucessSEP();
        void OnNewImage();
    protected:
        void onExternalEvent(OST::ExtEvent event) override;


    private:
        void updateProperty(INDI::Property property) override;
        void newBLOB(INDI::PropertyBlob pblob);

        void initIndi(void);

        void Shoot();
        void SMAlert();
        //void SMLoadblob(IBLOB *bp);
        void SMLoadblob();
        void SMAbort();
        void startCoarse();

        bool    _newblob;

        QPointer<fileio> _image;
        Solver _solver;
        FITSImage::Statistic stats;

        int    _iterations = 3;
        int    _steps = 3000;
        int    _loopIterations = 2;
        int    _loopIteration;
        double _loopHFRavg;

        int    _iteration;
        double _bestpos;
        double _bestposfit;
        double _besthfr;
        QString mState = "idle";
        double upperLeftHFR;
        double lowerLeftHFR;
        double upperRightHFR;
        double lowerRightHFR;

};

extern "C" MODULE_INIT Inspector *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
