#ifndef INDIMODULE_h_
#define INDIMODULE_h_
#include <QObject>
#include <basemodule.h>
#include <baseclient.h>
#include <basedevice.h>
#include <boost/log/trivial.hpp>


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


        /*indi messages */
        virtual void serverConnected() override
        {

        }
        virtual void serverDisconnected(int exit_code) override
        {
            Q_UNUSED(exit_code);
        }
        virtual void newDevice(INDI::BaseDevice baseDevice) override
        {
            Q_UNUSED(baseDevice);
        }
        virtual void removeDevice(INDI::BaseDevice baseDevice) override
        {
            Q_UNUSED(baseDevice);
        }
        virtual void newProperty(INDI::Property property) override
        {
            Q_UNUSED(property);
        }
        virtual void removeProperty(INDI::Property property) override
        {
            Q_UNUSED(property);
        }
        /*virtual void newText(ITextVectorProperty *tvp)
        {
            Q_UNUSED(tvp);
        }
        virtual void newSwitch(ISwitchVectorProperty *svp)
        {
            Q_UNUSED(svp);
        }
        virtual void newLight(ILightVectorProperty *lvp)
        {
            Q_UNUSED(lvp);
        }
        virtual void newNumber(INumberVectorProperty *nvp)
        {
            Q_UNUSED(nvp);
        }
        virtual void newBLOB(IBLOB *bp)
        {
            Q_UNUSED(bp);
        }*/
        virtual void newMessage(INDI::BaseDevice baseDevice, int messageID) override
        {
            Q_UNUSED(baseDevice);
            Q_UNUSED(messageID);
        }
        virtual void newUniversalMessage(std::string message) override
        {
            Q_UNUSED(message);
        }

    private:
        void OnDispatchToIndiExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                                           const QVariantMap &eventData) override;

    signals:
        void askedFrameReset(QString devicename);
}
;
#endif
