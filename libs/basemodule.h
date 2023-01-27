#ifndef BASEMODULE_h_
#define BASEMODULE_h_
#include <QObject>
#include <basedevice.h>
#include <baseclient.h>
#include <boost/log/trivial.hpp>
#include <QVariant>
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
        void setWebroot(QString webroot)
        {
            mWebroot = webroot;
        }
        void requestProfile(QString profileName);
        void setProfile(QVariantMap profiledata);
        void setProfiles(QVariantMap profilesdata);
        void sendDump(void);
        QVariantMap getProfile(void);


        /**
         * @brief gets webroot directory
         * @return webroot directory full path directory full
         */
        QString getWebroot(void)
        {
            return mWebroot;
        }
        QString getName(void)
        {
            return mOstProperties["name"].toString();
        }
        QString getLabel(void)
        {
            return mModulelabel;
        }
        QVariantMap getOstProperties(void)
        {
            return mOstProperties["properties"].toMap();
        }
        QVariantMap getOstProperty(QString name)
        {
            return mOstProperties["properties"].toMap()[name].toMap();
        }
        QVariantMap getModuleInfo(void);
        QVariantMap getAvailableModuleLibs(void)
        {
            return mAvailableModuleLibs;
        }
        QString getModuleLabel()
        {
            return mOstProperties["label"].toString();
        }
        QString getModuleDescription()
        {
            return mOstProperties["description"].toString();
        }
        QString getModuleVersion()
        {
            return mOstProperties["version"].toString();
        }
        QString getModuleType()
        {
            return mOstProperties["type"].toString();
        }

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

        /**
         * @brief Sends a message to controller
         * @param message is the message to send
         */
        void sendMessage(QString message);
        /**
         * @brief Sets module name property
         * @param name is the name
         */
        void setName(QString name)
        {
            mOstProperties["name"] = name;
        }
        /**
         * @brief Sets module label property
         * @param label is the label
         */
        void setModuleLabel(QString label)
        {
            mOstProperties["label"] = label;
        }
        void setModuleDescription(QString _s)
        {
            mOstProperties["description"] = _s;
        }
        void setModuleVersion(QString _s)
        {
            mOstProperties["version"] = _s;
        }
        void setModuleType(QString _s)
        {
            mOstProperties["type"] = _s;
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
            return mOstProperties["properties"].toMap()[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap()["value"]    ;
        }

        void loadPropertiesFromFile(const QString &pFileName);
        void savePropertiesToFile(const QString &pFileName);

    private:

        QVariantMap mOstProperties;
        //QString mModulename;
        QString mModulelabel;
        QVariantMap mAvailableModuleLibs;
        QVariantMap mAvailableProfiles;
        QString mModuleType;
        QString mWebroot;



    signals:
        void moduleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                         const QVariantMap &eventData);
        void loadOtherModule(QString &lib, QString &name, QString &label, QString &profile);
}
;
#endif
