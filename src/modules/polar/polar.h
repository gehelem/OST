#ifndef POLAR_MODULE_h_
#define POLAR_MODULE_h_
#include <basemodule.h>

#if defined(POLAR_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

#include <QtCore>
#include <QtConcurrent>
#include <QStateMachine>
#include "image.h"
#include "rotations.h"



class MODULE_INIT PolarModule : public Basemodule
{
    Q_OBJECT

    public:
        PolarModule(QString name,QString label);
        ~PolarModule();

    signals:
        void InitDone();
        void AbortDone();
        void Abort();
        void RequestFrameResetDone();
        void FrameResetDone();
        void RequestExposureDone();
        void ExposureDone();
        void FindStarsDone();
        void ComputeDone();
        void PolarDone();
        void RequestMoveDone();
        void MoveDone();


    public slots:
        void OnSetPropertyText(TextProperty* prop) override;
        void OnSetPropertyNumber(NumberProperty* prop) override;
        void OnSetPropertySwitch(SwitchProperty* prop) override;
        void OnSucessSEP();
        void DummySlot(){BOOST_LOG_TRIVIAL(debug) << "************************* DUMMY SLOT";}
    private:
        void newNumber(INumberVectorProperty *nvp) override;
        void newBLOB(IBLOB *bp) override;
        void newSwitch(ISwitchVectorProperty *svp) override;

        SwitchProperty* _actions;
        NumberProperty* _commonParams;
        NumberProperty* _calParams;
        NumberProperty* _guideParams;
        NumberProperty* _values;
        NumberProperty* _errors;
        ImageProperty*  _img;
        LightProperty*  _states;


        //std::unique_ptr<Image> image =nullptr;
        QPointer<Image> image;

        double _exposure = 2;
        double _mountDEC;
        double _mountRA;
        bool   _mountPointingWest=false;
        double _ccdOrientation;
        double _aperture;
        double _focalLength;
        double _ccdX;
        double _ccdY;
        double _ccdSize;
        double _ccdFov;
        double _pixelSize;
        double _ccdSampling=206*5.2/800;
        double _ra0=0;
        double _de0=0;
        double _t0=0;
        double _ra1=0;
        double _de1=0;
        double _t1=0;
        double _ra2=0;
        double _de2=0;
        double _t2=0;
        double _erraz=0;
        double _erralt=0;
        double _errtot=0;
        int _itt=0;


        QString _camera  = "CCD Simulator";
        QString _mount  = "Telescope Simulator";
        QStateMachine _machine;

        double square(double value){ return value*value;}

        void buildStateMachine(void);
        void SMInit();
        void SMRequestExposure();
        void SMFindStars();
        void SMCompute();
        void SMComputeFinal();
        void SMRequestFrameReset();
        void SMRequestMove();
        void SMAbort();
};

extern "C" MODULE_INIT PolarModule *initialize(QString name,QString label);

#endif
