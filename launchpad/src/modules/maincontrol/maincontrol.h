#ifndef MAINCONTROL_MODULE_h_
#define MAINCONTROL_MODULE_h_
#include <basemodule.h>

#if defined(MAINCONTROL_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT Maincontrol : public Basemodule
{
    Q_OBJECT

    public:
        Maincontrol(QString name,QString label,QString profile,QVariantMap availableModuleLibs);
        ~Maincontrol();

    public slots:
        void OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData) override;
    private:

};

extern "C" MODULE_INIT Maincontrol *initialize(QString name,QString label,QString profile,QVariantMap availableModuleLibs);

#endif
