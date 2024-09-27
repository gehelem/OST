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
        Maincontrol(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Maincontrol();
        void sendMainMessage(const QString &pMessage);
        void sendMainError(const QString &pMessage);
        void sendMainWarning(const QString &pMessage);
        void sendMainConsole(const QString &pMessage);
        void setAvailableModuleLibs(const QVariantMap libs);
        void addModuleData(const QString  &pName, const QString  &pLabel, const QString  &pType, const QString  &pProfile);
        void setModuleData(const QString  &pName, const QString  &pLabel, const QString  &pType, const QString  &pProfile);
        void deldModuleData(const QString  &pName);
        void searchDriver(void);
        void setIndiDriverList(const QStringList pDrivers);

    public slots:
        void OnMyExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                               const QVariantMap &pEventData) override;
    private:
        void setConfigurations(void);
        QStringList mIndiDriverList;
    signals:
        void mainCtlEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                          const QVariantMap &pEventData);

};

extern "C" MODULE_INIT Maincontrol *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
