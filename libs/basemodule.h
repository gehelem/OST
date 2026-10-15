#ifndef BASEMODULE_h_
#define BASEMODULE_h_

#include <dbmanager.h>
#include <QImage>
#include <QSet>
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
        void setMinFreePercent(int percent);
        int  getMinFreePercent() const;

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
        QString getCurrentProfile();
        bool getCurrentProfileChanged();

        /*!
         * "family" = a capability a module declares it has, orthogonal to
         * its type (the C++/library class, e.g. "Guider"/"BlindPEC") and to
         * its display template. A module can belong to 0, 1 or several
         * families - set once at init (declareFamily is meant to be called
         * from a module's own init code, e.g. via a shared IndiModule
         * defineMeAsX() helper), never changed afterwards. The controller
         * exposes membership as one "family-<name>" lov per family (see
         * Controller::loadModule/onExternalEvent's MK case), the same way
         * it already exposes "loadedModules-<type>" - this is what lets
         * e.g. Sequencer pick any loaded module with family "guiding" as
         * its guiding slave, regardless of whether it's actually a Guider
         * or a BlindPEC instance.
         */
        void declareFamily(const QString &family);
        QStringList getFamilies() const;
        bool hasFamily(const QString &family) const;

        virtual void onAfterInit(void);

    private:

        QVariantMap mAvailableModuleLibs;
        QVariantMap mAvailableProfiles;
        QSet<QString> mFamilies;
        QString mWebroot;
        int     mMinFreePercent = 10;
        QString mModuleName;
        QString mModuleLabel;
        OST::ModuleStatus mStatus;
        QString mCurrentProfile = "default";
        bool mCurrentProfileChanged = false;

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

        void interModuleRequest(OST::ExtEvent event);

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
        virtual void onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line);

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
        virtual bool onExternalEventBase(OST::ExtEvent event);

        /**
         * @brief Hook for INDI module event handling
         *
         * Called second in the event chain. Empty by default.
         * IndiModule overrides this to handle INDI-specific events.
         *
         * Custom modules inheriting from IndiModule do NOT need to override this.
         */
        virtual bool onExternalEventIndi(OST::ExtEvent event);

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

        /**
         * @brief Answer a named query from another module (generic RPC-style hook)
         *
         * Override this in modules that expose derived/computed information other
         * modules may need without loading a profile or duplicating properties
         * (e.g. "what would the theoretical duration of profile X be").
         * Empty by default - custom modules override this to answer the query
         * names they support, returning an empty map for anything else.
         *
         * The answer is sent back to the asking module as a moduleEvent of type
         * OST::EvType::qa, received through its onOtherModuleEvent.
         */
        virtual QVariantMap onModuleQuery(const QString &queryName, const QVariantMap &params);

        void otherModuleSetValue(QString mod, QString prop, QString elt, QVariant value);
        void otherModuleRequestPropertyDump(QString mod, QString prop);
        void otherModuleRequestProfileLoad(QString mod, QString profile);
        void otherModuleCreateLine(QString mod, QString prop, QVariantMap values);
        void otherModuleQuery(QString mod, QString queryName, QVariantMap params);
        bool giveMeAState();
        bool giveMeAParms();


    public:
        // Shared datastore access
        void setGlobalDatastore(Basemodule* gds);

        // Grid read utilities — callable on mGlobalDatastore from any module
        int     findGridRow  (const QString &propName, const QString &keyCol, const QString &keyValue);
        int     getGridSize  (const QString &propName);
        QString getGridString(const QString &propName, const QString &colName, int line);
        double  getGridFloat (const QString &propName, const QString &colName, int line);
        long    getGridInt   (const QString &propName, const QString &colName, int line);
        // Key-based overloads — look up by keyCol=keyValue, no intermediate row needed
        QString getGridString(const QString &propName, const QString &colName, const QString &keyCol, const QString &keyValue);
        double  getGridFloat (const QString &propName, const QString &colName, const QString &keyCol, const QString &keyValue);
        long    getGridInt   (const QString &propName, const QString &colName, const QString &keyCol, const QString &keyValue);

    protected:
        Basemodule* mGlobalDatastore = nullptr;
        void setStateEvent(OST::State state, QString statedescription, QString event, QString eventdescription, double valNum = 0.0, int valInt = 0, QString valStr = "");

        // Writes img to finalPath via a temporary file plus an atomic
        // rename(), so a reader going through nginx's sendfile()
        // (preview/live images under /ostmedia/) never sees a
        // partially-written JPEG. Shared by every module that serves a
        // live preview image (issue #239 - used to be a byte-for-byte
        // copy of the same static function in each module's own .cpp
        // file).
        static void atomicSaveJpeg(const QImage &img, const QString &finalPath, int quality = 100);
}
;
#endif
