#ifndef BASEMODULE_h_
#define BASEMODULE_h_
#include "qdebug.h"
#include <QObject>
#include <basedevice.h>
#include <baseclient.h>
#include <datastore.h>
#include <profiles.h>
#include <boost/log/trivial.hpp>
#include <QVariant>
/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
*/
class Basemodule : public QObject, public Datastore
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
        QVariantMap getModuleInfo(void);
        QVariantMap getAvailableModuleLibs(void)
        {
            return mAvailableModuleLibs;
        }
        QString getModuleName()
        {
            return mModuleName;
        }
        QString getModuleLabel()
        {
            return mModuleLabel;
        }
        QString getModuleDescription()
        {
            return mModuleDescription;
        }
        QString getModuleVersion()
        {
            return mModuleVersion;
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

        void setModuleDescription(QString description)
        {
            mModuleDescription = description;
        }
        void setModuleVersion(QString version)
        {
            mModuleVersion = version;
        }

        /* OST helpers */
        void deleteOstProperty(const QString &pPropertyName);
        bool pushOstElements        (const QString &pPropertyName);
        bool resetOstElements      (const QString &pPropertyName);

    private:

        QVariantMap mOstProperties;
        QVariantMap mAvailableModuleLibs;
        QVariantMap mAvailableProfiles;
        QString mWebroot;
        QString mModuleName;
        QString mModuleLabel;
        QString mModuleDescription;
        QString mModuleVersion;

        void OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                           const QVariantMap &eventData) override;



    signals:
        void moduleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                         const QVariantMap &eventData);
        void loadOtherModule(QString &lib, QString &name, QString &label, QString &profile);
}
;
#endif
