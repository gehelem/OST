#ifndef INDIMODULE_h_
#define INDIMODULE_h_
#include <basemodule.h>
#include <baseclient.h>
#include <basedevice.h>


/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
*/
class IndiModule : public Basemodule, public INDI::BaseClient
{
        Q_OBJECT

    public:
        IndiModule(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~IndiModule() = default;
        void requestProfile(QString profileName);
        void setProfile(QVariantMap profiledata);
        bool connectIndi(void);

    public slots:
        void connectIndiTimer(void);

    protected:

        /* Indi helpers */
        bool disconnectIndi(void);
        bool connectAllDevices(void);
        bool disconnectAllDevices(void);
        bool connectDevice(QString deviceName);
        bool disconnectDevice(QString deviceName);
        bool loadDevicesConfs(void);
        bool sendModNewText  (QString deviceName, QString propertyName, QString elementName, QString text);
        bool sendModNewSwitch(QString deviceName, QString propertyName, QString elementName, ISState sw);
        bool sendModNewNumber(const QString &deviceName, const QString &propertyName, const QString &elementName,
                              const double &value);

        bool getModNumber(const QString &deviceName, const QString &propertyName, const QString &elementName, double &value);
        bool getModSwitch(const QString &deviceName, const QString &propertyName, const QString &elementName, bool &value);
        bool getModText(const QString &deviceName, const QString &propertyName, const QString &elementName, QString &value);

        bool frameSet(QString devicename, double x, double y, double width, double height);
        bool frameReset(QString devicename);
        bool createDeviceProperty(const QString &key, const QString &label, const QString &level1,
                                  const QString &level2, const QString &order, INDI::BaseDevice::DRIVER_INTERFACE interface);
        bool refreshDeviceslovs();
    private:
        void OnDispatchToIndiExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                                           const QVariantMap &eventData) override;
        bool refreshDevicelovs(INDI::BaseDevice::DRIVER_INTERFACE interface, QString name);

    signals:
        void askedFrameReset(QString devicename);
}
;
#endif
