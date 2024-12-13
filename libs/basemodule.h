#ifndef BASEMODULE_h_
#define BASEMODULE_h_

#include <dbmanager.h>
/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
*/
class Basemodule : public DBManager
{
        Q_OBJECT

    public:
        Basemodule(QString name, QString label, QString profile, QVariantMap params);
        ~Basemodule();
        void setWebroot(QString webroot);
        void setProfile(QVariantMap profiledata);
        void setProfile(const QString &pProfileName);
        void setProfiles();
        void sendDump(void);
        void killMe(void);
        /*
         * @brief gets webroot directory
         * @return webroot directory full path directory full
         */
        QString getWebroot(void);
        QVariantMap getModuleInfo(void);
        QVariantMap getAvailableModuleLibs(void);
        QString getModuleName();
        QString getModuleLabel();
        QString getModuleDescription();
        QString getModuleVersion();
        QString getClassName();
        QString getHelpContent(QString language);
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
        QMap<QString, QPointer<Basemodule>> loadedModules;


    protected:

        void setModuleDescription(QString description);
        void setModuleVersion(QString version);

    private:

        QVariantMap mAvailableModuleLibs;
        QVariantMap mAvailableProfiles;
        QString mWebroot;
        QString mModuleName;
        QString mModuleLabel;
        QString mModuleDescription;
        QString mModuleVersion;
        QString mClassName = "";
        OST::ElementString* mModuleDesc;
        OST::ModuleStatus mStatus;


        void OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                           const QVariantMap &eventData) override;

    signals:
        void moduleStatusRequest(void);
        void moduleStatusAnswer(const QString module, OST::ModuleStatus);
        void moduleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                         const QVariantMap &eventData);
        void loadOtherModule(QString &lib, QString &name, QString &label, QString &profile);
    public slots:
        void OnExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                             const QVariantMap &pEventData);
        virtual void OnMyExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                       const QVariantMap &pEventData)
        {
            Q_UNUSED(pEventType) Q_UNUSED(pEventModule) Q_UNUSED(pEventKey) Q_UNUSED(pEventData)
        }
        virtual void OnDispatchToIndiExternalEvent(const QString &pEventType, const QString  &pEventModule,
                const QString  &pEventKey,
                const QVariantMap &pEventData)
        {
            Q_UNUSED(pEventType) Q_UNUSED(pEventModule) Q_UNUSED(pEventKey) Q_UNUSED(pEventData)
        }
        void OnModuleStatusRequest();
        virtual void OnModuleStatusAnswer(const QString module, OST::ModuleStatus status)
        {
            Q_UNUSED(module) Q_UNUSED(status)
        };

}
;
#endif
