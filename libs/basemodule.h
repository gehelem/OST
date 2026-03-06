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

        bool saveProfile(const QString &pProfileName);
        bool loadProfile(const QString &pProfileName);
        void updateProfilesLov();

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
        void onDatastoreEvent(OST::EvType evt, QVariant data, OST::ElementBase* elt, OST::PropertyBase* prp,
                              OST::LovBase* lov,
                              Datastore* mod);

        /**
         * @brief Root event orchestrator (FINAL - cannot be overridden)
         *
         * This method is called by signal/slot connections and orchestrates
         * the handling of external events by calling the virtual hooks in order:
         * Base → Indi → Custom
         *
         * Developers should NEVER call or override this method directly.
         * This is an internal orchestration method.
         */
        virtual void onExternalEventRoot(OST::ExtEvent event) final;

    protected:
        /**
         * @brief Hook for base module event handling
         *
         * Called first in the event chain. Handles common events like:
         * - Profile load/save (PL, PS)
         * - Configuration load/save (CL, CS)
         * - Other base module operations
         *
         * Override this in Basemodule descendants if you need to customize
         * base-level event handling (rare).
         */
        virtual void onExternalEventBase(OST::ExtEvent event);

        /**
         * @brief Hook for INDI module event handling
         *
         * Called second in the event chain. Empty by default.
         * IndiModule overrides this to handle INDI-specific events.
         *
         * Custom modules inheriting from IndiModule do NOT need to override this.
         */
        virtual void onExternalEventIndi(OST::ExtEvent event);

        /**
         * @brief Hook for custom module event handling (PRIMARY OVERRIDE POINT)
         *
         * Called last in the event chain. Empty by default.
         * Custom modules (Dummy, Focus, Guider, etc.) should override this
         * to handle module-specific events like:
         * - Set value (SV, SA)
         * - Grid operations (GC, GU, GD)
         * - Custom module actions
         *
         * This is the PRIMARY method for module developers to implement.
         *
         * @note Module developers: override THIS method (onExternalEvent),
         *       NOT onExternalEventRoot (which is final).
         */
        virtual void onExternalEvent(OST::ExtEvent event);

}
;
#endif
