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

        //std::unique_ptr<Image> image =nullptr;
        QPointer<Image> image;

        int    _exposure = 2;

        QString _camera  = "Guide Simulator";
        QString _mount  = "Telescope Simulator";
        QStateMachine _machine;

        void startCalibration();


        void SMFindStars();
        void SMRequestFrameReset();
        void SMRequestExposure();
        void SMAbort();


};

extern "C" MODULE_INIT GuiderModule *initialize(QString name,QString label);

#endif
