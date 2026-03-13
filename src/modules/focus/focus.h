#ifndef FOCUS_MODULE_h_
#define FOCUS_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>
#include <QScxmlStateMachine>

#if defined(FOCUS_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

#include <QStateMachine>


class MODULE_INIT Focus : public IndiModule
{
        Q_OBJECT

    public:
        Focus(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Focus();

    signals:
        void cameraAlert();
        void abort();
    public slots:
        void OnSucessSEP();
    protected:
        void onExternalEvent(OST::ExtEvent event) override;
    private:
        void updateProperty(INDI::Property p) override;
        void newBLOB(INDI::PropertyBlob pblob);

        void SMRequestFrameReset();
        void SMRequestBacklash();
        void SMRequestGotoStart();
        void SMRequestExposure();
        void SMFindStars();
        void SMCompute();
        void SMRequestGotoNext();

        void SMRequestBacklashBest();
        void SMRequestGotoBest();
        void SMRequestExposureBest();
        void SMComputeResult();
        void SMInitLoopFrame();
        void SMComputeLoopFrame();
        void SMFocusDone();


        void SMAlert();
        //void SMLoadblob(IBLOB *bp);
        void SMLoadblob();
        void SMAbort();
        void startCoarse();


        bool    _newblob;

        int    _startpos = 30000;
        int    _backlash = 100;
        int    _iterations = 3;
        int    _steps = 3000;
        int    _loopIterations = 2;
        int    _loopIteration;
        QList<int> _zoneLoopIteration;
        double _loopHFRavg;
        int mZoning = 2;
        QList<double> _zoneloopHFRavg;

        int    _iteration;
        double _bestpos;
        double _bestposfit;
        QList<double> _zoneBestposfit;
        double _besthfr;
        double  _focuserPosition;
        QScxmlStateMachine *pMachine;

        std::vector<double> _posvector;
        std::vector<double> _hfdvector;
        std::vector<double> _coefficients;

        QList<std::vector<double>> _zonePosvector;
        QList<std::vector<double>> _zoneHfdvector;
        QList<std::vector<double>> _zoneCoefficients;

        QPointer<fileio> _image;
        Solver _solver;
        FITSImage::Statistic stats;


};

extern "C" MODULE_INIT Focus *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
