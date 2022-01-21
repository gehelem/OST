#ifndef GUIDER_MODULE_h_
#define GUIDER_MODULE_h_
#include <basemodule.h>

#if defined(GUIDER_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

#include <QtCore>
#include <QtConcurrent>
#include <QStateMachine>
#include "image.h"

struct Trig
{
  double x1; // first star position
  double y1;
  double x2; // second star position
  double y2;
  double x3; // third star position
  double y3;
  double d12; // 1-2 distance
  double d13; // 1-3 distance
  double d23; // 2-3 distance
  double p; // perimeter
  double s; // surface
  double ratio; // surface/perimeter ratio
};

struct MatchedPair
{
  double xr; // ref star position
  double yr;
  double xc; // current star position
  double yc;
  double dx; // x drift
  double dy; // y drift
};

class MODULE_INIT GuiderModule : public Basemodule
{
    Q_OBJECT

    public:
        GuiderModule(QString name,QString label);
        ~GuiderModule();

    signals:
        void InitDone();
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


    public slots:
        void OnSetPropertyText(TextProperty* prop) override;
        void OnSetPropertyNumber(NumberProperty* prop) override;
        void OnSetPropertySwitch(SwitchProperty* prop) override;
        void OnSucessSEP();
    private:
        void newNumber(INumberVectorProperty *nvp) override;
        void newBLOB(IBLOB *bp) override;
        void newSwitch(ISwitchVectorProperty *svp) override;

        SwitchProperty* _actions;
        NumberProperty* _commonParams;
        NumberProperty* _calParams;
        NumberProperty* _guideParams;
        NumberProperty* _values;
        ImageProperty*  _img;
        GridProperty*   _grid;
        LightProperty*  _states;
        GridProperty*   _gridguide;


        //std::unique_ptr<Image> image =nullptr;
        QPointer<Image> image;

        double _exposure = 0.5;
        int    _pulse  = 1000;
        int    _pulseMax  = 2000;
        int    _pulseMin  = 100;
        double _raAgr = 0.7;
        double _deAgr = 0.7;
        int    _pulseN = 0;
        int    _pulseS = 0;
        int    _pulseE = 0;
        int    _pulseW = 0;
        int    _calPulseN = 0;
        int    _calPulseS = 0;
        int    _calPulseE = 0;
        int    _calPulseW = 0;
        int    _calPulseRA = 0;
        int    _calPulseDEC = 0;
        int    _calState =0;
        int    _calStep=0;
        int    _calSteps=3;
        bool   _pulseRAfinished= true;
        bool   _pulseDECfinished = true;
        double _dxFirst=0;
        double _dyFirst=0;
        double _dxRef=0;
        double _dyRef=0;
        double _dxPrev=0;
        double _dyPrev=0;
        double _mountDEC;
        double _mountRA;
        bool   _mountPointingWest=false;
        bool   _calMountPointingWest=false;
        double _ccdOrientation;
        double _calCcdOrientation;
        double _ccdSampling=206*5.2/800;
        int _itt=0;


        QString _camera  = "Guide Simulator";
        QString _mount  = "Telescope Simulator";
        QStateMachine *_machine;
        QStateMachine _SMInit;
        QStateMachine _SMCalibration;
        QStateMachine _SMGuide;
        QVector<Trig> _trigFirst;
        QVector<Trig> _trigRef;
        QVector<Trig> _trigPrev;
        QVector<Trig> _trigCurrent;
        QVector<MatchedPair> _matchedCurRef;
        QVector<MatchedPair> _matchedCurPrev;
        QVector<MatchedPair> _matchedCurFirst;


        std::vector<double> _dxvector;
        std::vector<double> _dyvector;
        std::vector<double> _coefficients;
        std::vector<double> _dRAvector;
        std::vector<double> _dDEvector;


        void startCalGuide(bool cal);
        void buildIndexes(Solver& solver, QVector<Trig>& trig);
        void matchIndexes(QVector<Trig> ref,QVector<Trig> act, QVector<MatchedPair>& pairs, double& dx,double& dy);
        double square(double value){ return value*value;}

        void buildStateMachines(void);
        void SMInit();
        void SMRequestExposure();
        void SMRequestPulses();
        void SMFindStars();
        void SMComputeFirst();
        void SMComputeCal();
        void SMComputeGuide();
        void SMRequestFrameReset();
        void SMAbort();
};

extern "C" MODULE_INIT GuiderModule *initialize(QString name,QString label);

#endif
