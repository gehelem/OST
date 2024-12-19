#ifndef DATASTORE_h_
#define DATASTORE_h_
//#include "jsondumper.h"
#include <baseroot.h>
#include <propertytextdumper.h>
#include <propertyjsondumper.h>
#include <propertyupdate.h>
#include <propertyfactory.h>
#include <lovfactory.h>
#include <elementset.h>
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

class Datastore : public Baseroot
{
    public:
        Datastore();
        ~Datastore();
        OST::LovString* getGlovString(QString pLov);
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
        OST::ElementMessage* getEltMsg(QString pProperty, QString pElement);
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
                sendWarning("createProperty - property " + pPropertyName + " already exists.");
                return false;
            }
            mStore[pPropertyName] = pProperty;
            OST::PropertyJsonDumper d;
            mStore[pPropertyName]->accept(&d);
            OnModuleEvent("cp", QString(), pPropertyName, d.getResult().toVariantMap());
            connect(mStore[pPropertyName], &OST::PropertyMulti::valueSet, this, &Datastore::onValueSet);
            connect(mStore[pPropertyName], &OST::PropertyMulti::eltChanged, this, &Datastore::onEltChanged);
            connect(mStore[pPropertyName], &OST::PropertyMulti::propertyEvent, this, &Datastore::onPropertyEvent);
            connect(mStore[pPropertyName], &OST::PropertyMulti::sendMessage, this, &Datastore::onPropertyMessage);
            return true;
        }
        bool createGlobLov(const QString &pLovName,  OST::LovBase* pLov)
        {
            if (mGlobLov.contains(pLovName))
            {
                sendWarning("createGlobLov - lov " + pLovName + " already exists.");
                return false;
            }
            pLov->setKey(pLovName);
            mGlobLov[pLovName] = pLov;
            OST::PropertyJsonDumper d;
            connect(mGlobLov[pLovName], &OST::LovBase::lovChanged, this, &Datastore::onLovChanged);
            return true;
        }

        //QVariant getOstPropertyValue(const QString &pPropertyName);

        bool createOstElementText(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                  bool mEmitEvent);
        bool createOstElementBool(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                  bool mEmitEvent);
        Q_DECL_DEPRECATED bool setOstElementValue(const QString &pPropertyName, const QString &pElementName,
                const QVariant &pElementValue,
                bool mEmitEvent);
        Q_DECL_DEPRECATED QVariantList getOstElementGrid(const QString &pPropertyName, const QString &pElementName);

        void loadOstPropertiesFromFile(const QString &pFileName);
        void saveOstPropertiesToFile(const QString &pFileName);

        void deleteOstProperty(const QString &pPropertyName);

        QVariantMap getProfile(void);

        QJsonObject getPropertiesDump(void);
        QJsonObject getGlobalLovsDump(void);

    private slots:
        void onEltChanged(void);
        void onValueSet(void);
        void onPropertyEvent(QString event, QString key, OST::PropertyBase* prop);
        void onPropertyMessage(OST::MsgLevel l, QString m);
        void onLovChanged(void);

    private:
        QMap<QString, OST::PropertyMulti*> mStore;
        QMap<QString, OST::LovBase*> mGlobLov;

}
;
#endif

