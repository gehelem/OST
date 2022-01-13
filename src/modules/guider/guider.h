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
        void RequestFrameResetDone();
        void FrameResetDone();
        void RequestExposureDone();
        void ExposureDone();
        void FindStarsDone();
        void RequestPulsesDone();
        void PulsesDone();
        void ComputeRefDone();
        void ComputeCalDone();
        void ComputeGuideDone();
        void StartGuiding();
        void abort();


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
        NumberProperty* _parameters;
        NumberProperty* _values;
        ImageProperty*  _img;
        GridProperty*   _grid;


        //std::unique_ptr<Image> image =nullptr;
        QPointer<Image> image;

        int    _exposure = 2;
        int    _pulse  = 1000;
        int    _pulseN = 0;
        int    _pulseS = 0;
        int    _pulseE = 0;
        int    _pulseW = 0;
        int    _pulseNTot = 0;
        int    _pulseSTot = 0;
        int    _pulseETot = 0;
        int    _pulseWTot = 0;
        int    _calState =0;
        int    _calStep=0;
        int    _calSteps=3;
        bool   _pulseRAfinished= true;
        bool   _pulseDECfinished = true;
        double _avdx=0;
        double _avdy=0;

        QString _camera  = "Guide Simulator";
        QString _mount  = "Telescope Simulator";
        QStateMachine _machine;
        QVector<Trig> _trigRef;
        QVector<Trig> _trigCurrent;
        QVector<MatchedPair> _matchedPairs;

        std::vector<double> _dxvector;
        std::vector<double> _dyvector;
        std::vector<double> _coefficients;
        std::vector<double> _dRAvector;
        std::vector<double> _dDEvector;


        void startCalibration();
        void matchTrig(QVector<Trig> ref,QVector<Trig> act);
        double square(double value){ return value*value;}

        void SMRequestExposure();
        void SMComputeRef();
        void SMRequestPulses();
        void SMFindStars();
        void SMComputeCal();
        void SMComputeGuide();
        void SMRequestFrameReset();
        void SMAbort();
};

extern "C" MODULE_INIT GuiderModule *initialize(QString name,QString label);

#endif
