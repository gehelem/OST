#ifndef FOCUS_MODULE_h_
#define FOCUS_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>
#include <imagequality.h>
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
        void onNewDevice      (INDI::BaseDevice dp) override     {} ;
        void onRemoveDevice   (INDI::BaseDevice dp) override     {} ;
        void onNewProperty    (INDI::Property property) override {} ;
        void onRemoveProperty (INDI::Property property) override {} ;
        void onUpdateProperty (INDI::Property property) override {} ;

    signals:
        void cameraAlert();
        void abort();
    public slots:
        void OnSucessSEP();
        // "Loop" action's own StellarSolver completion, separate from
        // OnSucessSEP() (the autofocus state machine's) - see startLoop().
        void OnLoopSucessSEP();
    protected:
        void onExternalEvent(OST::ExtEvent event) override;
    private:
        void updateProperty(INDI::Property p) override;
        void newBLOB(INDI::PropertyBlob pblob);

        // "Loop" action: continuous measure-only monitoring, independent of
        // the SCXML autofocus sweep (no focuser move of its own - the user
        // is expected to move it by hand while watching the chart). Runs
        // until "abortfocus". Plots on the same "values" chart as the
        // autofocus V-curve, but against an incrementing frame index
        // (_loopIndex) rather than a real focuser position.
        void startLoop();
        void requestLoopExposure();
        void measureLoopFrame();
        void onLoopFrameMeasured(double quality);

        void SMRequestFrameReset();
        void SMRequestBacklash();
        void SMRequestGotoStart();
        void SMRequestExposure();
        void SMFindStars();
        Parameters buildStarDetectionParameters();
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
        void setupMachineConnections();
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
        double _khi;
        double _r2;
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

        // Set by SMFindStars() when "parameters.usesharpness" is on, as the
        // star-independent alternative to _solver.HFRavg (see imagequality.h).
        // Inverse of the Laplacian variance, so it keeps the same "lower is
        // better" convention as HFR - every comparison/parabola-minimum fit
        // below (SMCompute, SMRequestGotoBest, ...) stays meaningful without
        // caring which metric actually produced the value.
        double _sharpnessQuality = 99;

        // "Loop" action's own state (see startLoop() et al.) - deliberately
        // separate from the autofocus state machine's own _iteration/
        // _loopIteration counters.
        bool _loopRunning = false;
        int _loopIndex = 0;


};

extern "C" MODULE_INIT Focus *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
