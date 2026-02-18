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

Q_DECLARE_METATYPE(OST::Event)

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
        QJsonObject getPropertiesDump(OST::Event evt);
        QJsonObject getGlobalLovsDump(void);
        void setMetadata(const QString key, const QVariant value);
        QVariant* getMetadata(const QString key)
        {
            return &mMetadata[key];
        };
        QVariantMap getAllMetadata(void)
        {
            return mMetadata;
        };

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
        QMap<QString, OST::PropertyMulti*> getStore(void)
        {
            return mStore;
        }
        QMap<QString, OST::LovBase*> getGlobLovs(void)
        {
            return mGlobLov;
        }
        OST::LovString* getGlovString(QString pLov);
        /**
         * @brief createOstProperty is a method that creates a property at runtime
         * @param pPropertyName is the property internal name
         * @param pPropertyLabel is the label of the property
         * @param pPropertyPermission sets if this property is :
         *  <ul>
         *    <li>0 = READONLY</li>
         *    <li>1 = WRITEONLY</li>
         *    <li>2 = READWRITE</li>
         *  </ul>
         * @note this permission is related to frontend, it has no effect inside module
         * @param pPropertyDevcat means "device or category", the first hierarchical level of a property (like indilib device level)
         * @note it's only a label associated to a property, without any hierarchical effect
         * @param pPropertyGroup is the second hierarchical level of a property (like indilib group level)
         * @note it's only a label associated to a property, without any hierarchical effect
         * @return A boolean that reports whether it was successful, true means success.
         */
        Q_DECL_DEPRECATED bool createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel,
                const int &pPropertyPermission,
                const  QString &pPropertyDevcat, const QString &pPropertyGroup);
        void emitPropertyCreation(const QString &pPropertyName);
        bool createProperty(const QString &pPropertyName,  OST::PropertyMulti* pProperty)
        {
            if (mStore.contains(pPropertyName))
            {
                logWarning("createProperty - property %1 already exists.", {pPropertyName});
                return false;
            }
            mStore[pPropertyName] = pProperty;
            OST::Event e;
            e.type = "ap";
            e.property = pPropertyName;
            e.module = getModuleName();
            OST::PropertyJsonDumper d(e);
            mStore[pPropertyName]->accept(&d);
            OnModuleEvent("cp", QString(), pPropertyName, d.getResult().toVariantMap());
            connect(mStore[pPropertyName], &OST::PropertyMulti::valueSet, this, &Datastore::onValueSet);
            connect(mStore[pPropertyName], &OST::PropertyMulti::eltChanged, this, &Datastore::onEltChanged);
            connect(mStore[pPropertyName], &OST::PropertyMulti::propertyEvent, this, &Datastore::onPropertyEvent);
            connect(mStore[pPropertyName], &OST::PropertyMulti::logMessage, this, &Datastore::onPropertyLog);
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
            return true;
        }

        //QVariant getOstPropertyValue(const QString &pPropertyName);

        bool createOstElementText(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                  bool mEmitEvent);
        bool createOstElementBool(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                  bool mEmitEvent);

        void loadOstPropertiesFromFile(const QString &pFileName);
        void saveOstPropertiesToFile(const QString &pFileName);

        void deleteOstProperty(const QString &pPropertyName);

        QVariantMap getProfile(void);


    private slots:
        void onEltChanged(void);
        void onValueSet(void);
        void onPropertyEvent(OST::PropertyBase* prop, OST::Event event);
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
         * @brief Signal emitted for custom property events
         * @param prop Pointer to the module
         * @param event Event descriptor
         *
         * Generic event mechanism for property-level operations like
         * grid line creation/deletion, up/down movements, etc.
         */
        void datastoreEvent(Datastore*, OST::Event);


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

