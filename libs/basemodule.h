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

    signals:
        void moduleStatusRequest(void);
        void moduleStatusAnswer(const QString module, OST::ModuleStatus);
        //void moduleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
        //                 const QVariantMap &eventData);
        void loadOtherModule(QString &lib, QString &name, QString &label, QString &profile);

        /**
         * @brief Signal emitted when datastore changes
         * @param event Event descriptor
         * @param data Additional payload
         * @param elt Pointer to element if required
         * @param prop Pointer to property if required
         * @param glov Pointer to globallov if required
         * @param mod Pointer to this datastore/module
         */
        void moduleEvent(OST::EvType, QVariant, OST::ElementBase*, OST::PropertyBase*, OST::LovBase*, Basemodule*);


    public slots:
        void onDatastoreEvent(OST::EvType evt, QVariant data, OST::ElementBase* elt, OST::PropertyBase* prp, OST::LovBase* lov,
                              Datastore* mod);

        void onExternalEvent(QVariantMap extEvent);

}
;
#endif
