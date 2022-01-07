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

    public slots:
        void OnSetPropertyText(TextProperty* prop) override;
        void OnSetPropertyNumber(NumberProperty* prop) override;
        void OnSetPropertySwitch(SwitchProperty* prop) override;

    private:
        void newNumber(INumberVectorProperty *nvp) override;
        void newBLOB(IBLOB *bp) override;
        void newSwitch(ISwitchVectorProperty *svp) override;



        //std::unique_ptr<Image> image =nullptr;
        QPointer<Image> image;

        QString _camera  = "CCD Simulator";
        QString _mount  = "Telescope Simulator";
        QStateMachine _machine;


};

extern "C" MODULE_INIT GuiderModule *initialize(QString name,QString label);

#endif
