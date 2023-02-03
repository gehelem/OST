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
class Basemodule : public QObject, public Datastore, public Profiles
{
        Q_OBJECT

    public:
        Basemodule(QString name, QString label, QString profile, QVariantMap params);
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
        QString getClassName()
        {
            return mClassName;
        }
        /**
         * @brief setClassName is a method to set inherited modules classname (ideally metaObject()->className())
         * @param pClassName is the classname
         * @warning This is uggly, i don't know to do that differently :
         * It's purpose is to share same profiles types between multiple instances of same module
         * i'd like to avoid to do it within inherited module
         * @return A boolean that reports whether it was successful, true means success.
         * False means ClassName has already been set, and sends a corresponding message
         */

        bool setClassName(const QString &pClassName);

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
        QString mClassName = "";

        void OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                           const QVariantMap &eventData) override;



    signals:
        void moduleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                         const QVariantMap &eventData);
        void loadOtherModule(QString &lib, QString &name, QString &label, QString &profile);
}
;
#endif
