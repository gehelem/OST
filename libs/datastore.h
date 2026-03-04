#ifndef DATASTORE_h_
#define DATASTORE_h_
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <propertytextdumper.h>
#include <propertyjsondumper.h>
#include <propertyupdate.h>
#include <propertyfactory.h>
#include <lovfactory.h>
#include <elementupdate.h>
#include <elementjsondumper.h>
#include <lovjsondumper.h>

/** @class Datastore
 *  @brief Class to provide properties management for OST modules
 *
 *  zzzzzzzzzzzzzzzzz
 *
 *  xxxxxxxxxxxxxxxxxxxxxx
 *  xxxxxxxxxxxxxxxxxxxxxx
 *  xxxxxxxxxxxxxxxxxxxxxx
 *
 *  @attention xxxxxxxxxxx
 *  xxxxxxxxxxxxxxxxxxxxxx
 *
 *  @see <a href="https://github.com/gehelem/OST">Github repository</a> for more details.
 *  @author Gehelem
 */

class Datastore : public QObject
{
        Q_OBJECT

    public:
        Datastore();
        ~Datastore();
        /**
         * @brief Returns the module/component name (to override in derived classes)
         */
        virtual QString getModuleName() const
        {
            return "Unknown";
        }
        virtual void OnModuleEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                   const QVariantMap &pEventData)
        {
            Q_UNUSED(pEventType);
            Q_UNUSED(pEventModule);
            Q_UNUSED(pEventKey);
            Q_UNUSED(pEventData);
        };
        void setMetadata(const QString key, const QVariant value);
        QVariant* getMetadata(const QString key)
        {
            return &mMetadata[key];
        };
        QVariantMap getAllMetadata(void)
        {
            return mMetadata;
        };
        QMap<QString, OST::PropertyMulti*> getStore(void)
        {
            return mStore;
        }
        QMap<QString, OST::LovBase*> getGlobLovs(void)
        {
            return mGlobLov;
        }
    protected:
        OST::PropertyMulti* getProperty(QString pProperty);
        OST::ElementBase* getEltBase(QString pProperty, QString pElement);
        OST::ElementString* getEltString(QString pProperty, QString pElement);
        OST::ElementInt* getEltInt(QString pProperty, QString pElement);
        OST::ElementFloat* getEltFloat(QString pProperty, QString pElement);
        OST::ElementBool* getEltBool(QString pProperty, QString pElement);
        OST::ElementLight* getEltLight(QString pProperty, QString pElement);
        OST::ElementImg* getEltImg(QString pProperty, QString pElement);
        OST::ElementVideo* getEltVideo(QString pProperty, QString pElement);
        OST::ElementPrg* getEltPrg(QString pProperty, QString pElement);
        OST::ElementDate* getEltDate(QString pProperty, QString pElement);
        OST::ElementTime* getEltTime(QString pProperty, QString pElement);

        QString getString(QString pProperty, QString pElement);
        QString getString(QString pProperty, QString pElement, long line);
        long getInt(QString pProperty, QString pElement);
        long getInt(QString pProperty, QString pElement, long line);
        double getFloat(QString pProperty, QString pElement);
        double getFloat(QString pProperty, QString pElement, long line);
        bool getBool(QString pProperty, QString pElement);
        bool getBool(QString pProperty, QString pElement, long line);
        QDate getDate(QString pProperty, QString pElement);
        QDate getDate(QString pProperty, QString pElement, long line);
        QTime getTime(QString pProperty, QString pElement);
        QTime getTime(QString pProperty, QString pElement, long line);
        OST::LovString* getGlovString(QString pLov);
        bool createProperty(OST::PropertyMulti* pProperty)
        {
            if (mStore.contains(pProperty->key()))
            {
                logWarning("createProperty - property %1 already exists.", {pProperty->key()});
                return false;
            }
            mStore[pProperty->key()] = pProperty;
            connect(mStore[pProperty->key()], &OST::PropertyMulti::prpEvent, this, &Datastore::onPropertyEvent);
            connect(mStore[pProperty->key()], &OST::PropertyMulti::logMessage, this, &Datastore::onPropertyLog);
            onPropertyEvent(OST::EvType::aa, QVariant(), nullptr, pProperty); // force property creation event
            return true;
        }
        bool createGlobLov(const QString &pLovName,  OST::LovBase* pLov)
        {
            if (mGlobLov.contains(pLovName))
            {
                logWarning("createGlobLov - lov %1 already exists.", {pLovName});
                return false;
            }
            pLov->setKey(pLovName);
            mGlobLov[pLovName] = pLov;
            connect(mGlobLov[pLovName], &OST::LovBase::lovChanged, this, &Datastore::onLovChanged);
            emit datastoreEvent(OST::EvType::lc, pLov->getKey(), nullptr, nullptr, pLov, this);
            return true;
        }

        void loadOstPropertiesFromFile(const QString &pFileName);
        void saveOstPropertiesToFile(const QString &pFileName);

        void deleteOstProperty(const QString &pPropertyName);

        QVariantMap getProfile(void);


    private slots:
        void onPropertyEvent(OST::EvType event, QVariant data, OST::ElementBase* e, OST::PropertyBase* p);
        void onPropertyLog(OST::LogLevel l, QString m, QVariantList args);
        void onLovChanged(void);


    signals:
        /**
         * @brief Signal emitted for all log messages
         * @param level Log level (Debug, Info, Warning, Error, Critical)
         * @param message Translation key for the message
         * @param args Dynamic arguments for parametric translation
         * @param context Context/source (module name, component, etc.)
         */
        void logSignal(OST::LogLevel level, const QString &message,
                       const QVariantList &args, const QString &context);

        /**
         * @brief Signal emitted when datastore changes
         * @param event Event descriptor
         * @param data Additional payload
         * @param elt Pointer to element if required
         * @param prop Pointer to property if required
         * @param glov Pointer to globallov if required
         * @param mod Pointer to this datastore/module
         */
        void datastoreEvent(OST::EvType, QVariant, OST::ElementBase*, OST::PropertyBase*, OST::LovBase*, Datastore*);


    protected:
        /**
         * @brief Helper methods to facilitate usage
         */
        void logDebug(const QString &message, const QVariantList &args = {});
        void logInfo(const QString &message, const QVariantList &args = {});
        void logWarning(const QString &message, const QVariantList &args = {});
        void logError(const QString &message, const QVariantList &args = {});
        void logCritical(const QString &message, const QVariantList &args = {});

    private:
        QMap<QString, OST::PropertyMulti*> mStore;
        QMap<QString, OST::LovBase*> mGlobLov;
        QVariantMap mMetadata;

}
;
#endif

