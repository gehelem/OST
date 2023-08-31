#ifndef DATASTORE_h_
#define DATASTORE_h_
//#include "jsondumper.h"
#include <baseroot.h>
#include <propertytextdumper.h>
#include <propertyjsondumper.h>
#include <propertyupdate.h>
#include <propertyfactory.h>
#include <valueupdate.h>
#include <valuejsondumper.h>

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
    protected:
        OST::PropertyMulti* getProperty(QString pProperty);
        OST::ValueString* getText(QString pProperty, QString pElement);
        QVariantMap getProperties(void)
        {
            //getQtProperties();
            return mProperties;
        }
        QMap<QString, OST::PropertyMulti*> getStore(void)
        {
            return mStore;
        }
        void getQtProperties(void)
        {
            qDebug() << "****************************************";
            qDebug() << "****************************************";
            QJsonObject properties;
            foreach(const QString &key, mStore.keys())
            {
                OST::PropertyJsonDumper d;
                mStore[key]->accept(&d);
                properties[key] = d.getResult();
            }
            QJsonObject json;
            json["properties"] = properties;
            qDebug() << json;
            qDebug() << "store size" << mStore.count();
            qDebug() << "****************************************";
            qDebug() << "****************************************";
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
        bool createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel, const int &pPropertyPermission,
                               const  QString &pPropertyDevcat, const QString &pPropertyGroup);
        void emitPropertyCreation(const QString &pPropertyName);
        bool createProperty(const QString &pPropertyName,  OST::PropertyMulti* pProperty)
        {
            if (mStore.contains(pPropertyName))
            {
                return false;
            }
            mStore[pPropertyName] = pProperty;
            OST::PropertyJsonDumper d;
            mStore[pPropertyName]->accept(&d);
            OnModuleEvent("cp", QString(), pPropertyName, d.getResult().toVariantMap());
            return true;
        }

        //QVariant getOstPropertyValue(const QString &pPropertyName);

        bool createOstElement(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                              bool mEmitEvent);
        bool createOstElementText(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                  bool mEmitEvent);
        bool createOstElementBool(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                  bool mEmitEvent);
        bool setOstElementValue(const QString &pPropertyName, const QString &pElementName, const QVariant &pElementValue,
                                bool mEmitEvent);
        QVariant getOstElementValue(const QString &pPropertyName, const QString &pElementName);
        bool setOstElementGrid(const QString &pPropertyName, const QString &pElementName, const QVariantList &pElementGrid,
                               bool mEmitEvent);
        QVariantList getOstElementGrid(const QString &pPropertyName, const QString &pElementName);

        void loadOstPropertiesFromFile(const QString &pFileName);
        void saveOstPropertiesToFile(const QString &pFileName);

        void setOstPropertyAttribute   (const QString &pPropertyName, const QString &pAttributeName,
                                        const QVariant &AttributeValue,
                                        bool mEmitEvent);
        bool setOstElementAttribute (const QString &pPropertyName, const QString &pElementName, const  QString &pAttributeName,
                                     const QVariant &AttributeValue,
                                     bool mEmitEvent);

        void deleteOstProperty(const QString &pPropertyName);
        bool pushOstElements        (const QString &pPropertyName);
        bool resetOstElements      (const QString &pPropertyName);

        bool newOstPropertyLine(const QString &pPropertyName, const QVariantMap &pElementsValues);
        bool deleteOstPropertyLine(const QString &pPropertyName, const double &pLine);
        bool updateOstPropertyLine(const QString &pPropertyName, const double &pLine, const QVariantMap &pElementsValues);
        QVariant getOstElementLineValue(const QString &pPropertyName, const QString &pElementName, const double &pLine);
        bool setOstElementLineValue(const QString &pPropertyName, const QString &pElementName, const double &pLine,
                                    const QVariant &pElementValue);

        QVariantMap getProfile(void);

        void clearOstElementLov(const QString &pPropertyName, const QString &pElementName);
        void addOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode,
                              const QString &pLovLabel);
        QVariant getOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode);
        void setOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode,
                              const QString &pLovLabel);
        void deleteOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode);

        void clearOstLov(const QString &pPropertyName);
        void addOstLov(const QString &pPropertyName, const QString &pLovCode,
                       const QString &pLovLabel);
        QVariant getOstLov(const QString &pPropertyName, const QString &pLovCode);
        void setOstLov(const QString &pPropertyName, const QString &pLovCode,
                       const QString &pLovLabel);
        void deleteOstLov(const QString &pPropertyName, const QString &pLovCode);
        QJsonObject getPropertiesDump(void);

    private slots:
        void onValueChanged(void);
    private:
        QVariantMap mProperties;
        QMap<QString, OST::PropertyMulti*> mStore;

}
;
#endif

