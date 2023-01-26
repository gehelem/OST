#ifndef BASEMODULE_h_
#define BASEMODULE_h_
#include <QObject>
#include <basedevice.h>
#include <baseclient.h>
#include <boost/log/trivial.hpp>
#include "fileio.h"
#include "solver.h"

/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
*/
class Basemodule : public QObject
{
        Q_OBJECT

    public:
        Basemodule(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Basemodule();
        void setHostport(QString host, int port);
        void setWebroot(QString webroot)
        {
            _webroot = webroot;
        }
        void requestProfile(QString profileName);
        void setProfile(QVariantMap profiledata);
        void setProfiles(QVariantMap profilesdata);
        void sendDump(void);
        QVariantMap getProfile(void);



        QString getWebroot(void)
        {
            return _webroot;
        }
        QString getName(void)
        {
            return _modulename;
        }
        QString getLabel(void)
        {
            return _modulelabel;
        }
        QVariantMap getOstProperties(void)
        {
            return _ostproperties["properties"].toMap();
        }
        QVariantMap getOstProperty(QString name)
        {
            return _ostproperties["properties"].toMap()[name].toMap();
        }
        QVariantMap getModuleInfo(void);
        QVariantMap getAvailableModuleLibs(void)
        {
            return _availableModuleLibs;
        }

        QString _moduletype;
        QString _webroot;

    public slots:
        void OnExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                             const QVariantMap &pEventData);
        virtual void OnMyExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                       const QVariantMap &pEventData)
        {
            Q_UNUSED(pEventType);
            Q_UNUSED(pEventModule);
            Q_UNUSED(pEventKey);
            Q_UNUSED(pEventData);
        }
        virtual void OnDispatchToIndiExternalEvent(const QString &pEventType, const QString  &pEventModule,
                const QString  &pEventKey,
                const QVariantMap &pEventData)
        {
            Q_UNUSED(pEventType);
            Q_UNUSED(pEventModule);
            Q_UNUSED(pEventKey);
            Q_UNUSED(pEventData);
        }

    protected:

        void sendMessage(QString mMessage);
        void setModuleLabel(QString _s)
        {
            _ostproperties["label"] = _s;
        }
        void setModuleDescription(QString _s)
        {
            _ostproperties["description"] = _s;
        }
        void setModuleVersion(QString _s)
        {
            _ostproperties["version"] = _s;
        }
        void setModuleType(QString _s)
        {
            _ostproperties["type"] = _s;
        }
        QString getModuleLabel()
        {
            return _ostproperties["label"].toString();
        }
        QString getModuleDescription()
        {
            return _ostproperties["description"].toString();
        }
        QString getModuleVersion()
        {
            return _ostproperties["version"].toString();
        }
        QString getModuleType()
        {
            return _ostproperties["type"].toString();
        }

        /* OST helpers */
        bool createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel, const int &pPropertyPermission,
                               const  QString &pPropertyDevcat, const QString &pPropertyGroup, QString &err);
        void emitPropertyCreation(const QString &pPropertyName);
        void deleteOstProperty(const QString &pPropertyName);
        void createOstElement (const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                               bool mEmitEvent);
        void setOstProperty   (const QString &pPropertyName, const QVariant &pValue, bool emitEvent);
        void setOstPropertyAttribute   (const QString &pPropertyName, const QString &pAttributeName, QVariant _value,
                                        bool emitEvent);
        bool setOstElement          (const QString &pPropertyName, const QString &pElementName, const QVariant &pElementValue,
                                     bool mEmitEvent);
        bool pushOstElements        (const QString &pPropertyName);
        bool resetOstElements      (const QString &pPropertyName);
        bool setOstElementAttribute (const QString &pPropertyName, const QString &pElementName, const  QString &pAttributeName,
                                     const QVariant &pValue,
                                     bool mEmitEvent);
        QVariant getOstElementValue (const QString &pPropertyName, const QString &pElementName)
        {
            return _ostproperties["properties"].toMap()[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap()["value"]    ;
        }

        void loadPropertiesFromFile(const QString &pFileName);
        void savePropertiesToFile(const QString &pFileName);

    private:

        QVariantMap _ostproperties;
        QString _modulename;
        QString _modulelabel;
        QVariantMap _availableModuleLibs;
        QVariantMap _availableProfiles;

    signals:
        void moduleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                         const QVariantMap &eventData);
        void loadOtherModule(QString &lib, QString &name, QString &label, QString &profile);
}
;
#endif
