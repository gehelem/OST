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
        QString getModuleName() const override;
        QString getModuleLabel();
        QString getClassName();
        QString getHelpContent(QString language);

    private:

        QVariantMap mAvailableModuleLibs;
        QVariantMap mAvailableProfiles;
        QString mWebroot;
        QString mModuleName;
        QString mModuleLabel;
        OST::ModuleStatus mStatus;


        void OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                           const QVariantMap &eventData) override;

    signals:
        void moduleStatusRequest(void);
        void moduleStatusAnswer(const QString module, OST::ModuleStatus);
        //void moduleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
        //                 const QVariantMap &eventData);
        void loadOtherModule(QString &lib, QString &name, QString &label, QString &profile);
        /**
         * @brief Signal emitted for custom property events
         * @param prop Pointer to the module
         * @param event Event descriptor
         *
         * Generic event mechanism for property-level operations like
         * grid line creation/deletion, up/down movements, etc.
         */
        void moduleEvent(Basemodule*, OST::Event);


    public slots:
        void onDatastoreEvent(Datastore* datastore, OST::Event e);
        void OnExternalEvent(OST::Event e);
        virtual void OnMyExternalEvent(OST::Event e)
        {
            Q_UNUSED(e)
        }
        virtual void OnDispatchToIndiExternalEvent(OST::Event e)
        {
            Q_UNUSED(e)
        }
        void OnModuleStatusRequest();
        virtual void OnModuleStatusAnswer(const QString module, OST::ModuleStatus status)
        {
            Q_UNUSED(module) Q_UNUSED(status)
        };

}
;
#endif
