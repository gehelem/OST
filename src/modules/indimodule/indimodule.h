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
        IndiModule(QString name, QString label,QString profile,QVariantMap availableModuleLibs);
        ~IndiModule() = default;
        void setHostport(QString host, int port);
        void setWebroot(QString webroot) {_webroot = webroot;}
        void requestProfile(QString profileName);
        void setProfile(QVariantMap profiledata);


        QString getWebroot(void) {return _webroot;}
        bool connectIndi(void);
        void setBlobMode(void);

    public slots:
        void connectIndiTimer(void);

    protected:

        /* Indi helpers */
        bool disconnectIndi(void);
        void connectAllDevices(void);
        void disconnectAllDevices(void);
        bool connectDevice(QString deviceName);
        void disconnectDevice(QString deviceName);
        void loadDevicesConfs(void);
        bool sendModNewText  (QString deviceName, QString propertyName,QString elementName, QString text);
        bool sendModNewSwitch(QString deviceName, QString propertyName,QString elementName, ISState sw);
        bool sendModNewNumber(const QString& deviceName, const QString& propertyName, const QString& elementName, const double& value);

        bool getModNumber(const QString& deviceName, const QString& propertyName, const QString& elementName, double& value);
        bool getModSwitch(const QString& deviceName, const QString& propertyName, const QString& elementName, bool& value);
        bool getModText(const QString& deviceName, const QString& propertyName, const QString& elementName, QString& value);

        bool frameSet(QString devicename,double x,double y,double width,double height);
        bool frameReset(QString devicename);


        /*indi messages */
        virtual void serverConnected() {}
        virtual void serverDisconnected(int exit_code)          {Q_UNUSED(exit_code);}
        virtual void newDevice(INDI::BaseDevice *dp)            {Q_UNUSED(dp);}
        virtual void removeDevice(INDI::BaseDevice *dp)         {Q_UNUSED(dp);}
        virtual void newProperty(INDI::Property *property)      {Q_UNUSED(property);}
        virtual void removeProperty(INDI::Property *property)   {Q_UNUSED(property);}
        virtual void newText(ITextVectorProperty *tvp)          {Q_UNUSED(tvp);}
        virtual void newSwitch(ISwitchVectorProperty *svp)      {Q_UNUSED(svp);}
        virtual void newLight(ILightVectorProperty *lvp)        {Q_UNUSED(lvp);}
        virtual void newNumber(INumberVectorProperty *nvp)      {Q_UNUSED(nvp);}
        virtual void newBLOB(IBLOB *bp)                         {Q_UNUSED(bp);}
        virtual void newMessage(INDI::BaseDevice *dp, int messageID) {Q_UNUSED(dp);Q_UNUSED(messageID);}
        virtual void newUniversalMessage(std::string message)   {Q_UNUSED(message);}

    private:
        void OnDispatchToIndiExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData) override;

    signals:
        void askedFrameReset(QString devicename);
}
;
#endif
