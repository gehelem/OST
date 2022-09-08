#ifndef BASEMODULE_h_
#define BASEMODULE_h_
#include <QObject>
#include <basedevice.h>
#include <baseclient.h>
#include <boost/log/trivial.hpp>
#include "ssolverutils/fileio.h"
#include "ssolverutils/solver.h"

/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
*/
class Basemodule : public QObject
{
    Q_OBJECT

    public:
        Basemodule(QString name, QString label,QString profile,QVariantMap availableModuleLibs);
        ~Basemodule();
        void setHostport(QString host, int port);
        void setWebroot(QString webroot) {_webroot = webroot;}
        void requestProfile(QString profileName);
        void setProfile(QVariantMap profiledata);
        QVariantMap getProfile(void);


        QString getWebroot(void) {return _webroot;}
        QString getName(void) {return _modulename;}
        QString getLabel(void) {return _modulelabel;}
        QVariantMap getOstProperties(void) {return _ostproperties;}
        QVariantMap getOstProperty(QString name) {return _ostproperties[name].toMap();}
        QVariantMap getModuleInfo(void);
        QVariantMap getAvailableModuleLibs(void) {return _availableModuleLibs;}

        QString _moduletype;
        QString _webroot;

    public slots:
        void OnExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData);
        virtual void OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
        {Q_UNUSED(eventType);Q_UNUSED(eventModule);Q_UNUSED(eventKey);Q_UNUSED(eventData);}
        virtual void OnDispatchToIndiExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
        {Q_UNUSED(eventType);Q_UNUSED(eventModule);Q_UNUSED(eventKey);Q_UNUSED(eventData);}

    protected:

        void sendMessage(QString message);

        /* OST helpers */
        bool createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel, const int &pPropertyPermission,const  QString &pPropertyDevcat, const QString &pPropertyGroup, QString &err);
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
        void loadOtherModule(QString &lib, QString &name, QString &label, QString &profile);
}
;
#endif
