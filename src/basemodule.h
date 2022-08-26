#ifndef MODULE_h_
#define MODULE_h_
#include <QtCore>
#include <QtConcurrent>
#include <QCoreApplication>
#include <QObject>
#include <basedevice.h>
#include <baseclient.h>
#include <boost/log/trivial.hpp>
#include "ssolverutils/fileio.h"
#include "solver.h"

/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
*/
class Basemodule : public QObject, public INDI::BaseClient
{
    Q_OBJECT

    public:
        Basemodule(QString name, QString label,QString profile,QVariantMap availableModuleLibs);
        ~Basemodule() = default;
        void setHostport(QString host, int port);
        void setWebroot(QString webroot) {_webroot = webroot;}
        void requestProfile(QString profileName);
        void setProfile(QVariantMap profiledata);


        QString getWebroot(void) {return _webroot;}
        bool connectIndi(void);
        void setBlobMode(void);
        QString getName(void) {return _modulename;}
        QString getLabel(void) {return _modulelabel;}
        QVariantMap getOstProperties(void) {return _ostproperties;}
        QVariantMap getOstProperty(QString name) {return _ostproperties[name].toMap();}
        QVariantMap getModuleInfo(void);
        QVariantMap getAvailableModuleLibs(void) {return _availableModuleLibs;}

        QString _moduletype;
        QString _webroot;

    public slots:
        void connectIndiTimer(void);
        void OnExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData);
        virtual void OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
        {Q_UNUSED(eventType);Q_UNUSED(eventModule);Q_UNUSED(eventKey);Q_UNUSED(eventData);}

    protected:

        /* Indi helpers */
        bool disconnectIndi(void);
        void connectAllDevices(void);
        void disconnectAllDevices(void);
        void connectDevice(QString deviceName);
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
        void sendMessage(QString message);

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

        /* OST helpers */
        void createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel, const int &pPropertyPermission,const  QString &pPropertyDevcat, const QString &pPropertyGroup);
        void emitPropertyCreation(const QString &pPropertyName);
        void deleteOstProperty(QString propertyName);
        void createOstElement (QString propertyName, QString elementName, QString elementLabel, bool emitEvent);
        void setOstProperty   (const QString &pPropertyName, QVariant _value,bool emitEvent);
        void setOstPropertyAttribute   (const QString &pPropertyName, const QString &pAttributeName, QVariant _value,bool emitEvent);
        bool setOstElement          (QString propertyName, QString elementName, QVariant elementValue, bool emitEvent);
        bool setOstElementAttribute (QString propertyName, QString elementName, QString attributeName, QVariant _value, bool emitEvent);
        QVariant getOstElementValue (QString propertyName, QString elementName){
            return _ostproperties[propertyName].toMap()["elements"].toMap()[elementName].toMap()["value"]    ;
        }

        void loadPropertiesFromFile(QString fileName);
        void savePropertiesToFile(QString fileName);

    private:

        QVariantMap _ostproperties;
        QString _modulename;
        QString _modulelabel;
        QVariantMap _availableModuleLibs;

    signals:
        void moduleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData);
        void askedFrameReset(QString devicename);
}
;
#endif
