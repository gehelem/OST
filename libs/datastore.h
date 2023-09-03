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
        OST::ValueString* getValueString(QString pProperty, QString pElement);
        OST::ValueInt* getValueInt(QString pProperty, QString pElement);
        OST::ValueFloat* getValueFloat(QString pProperty, QString pElement);
        OST::ValueBool* getValueBool(QString pProperty, QString pElement);
        QString getString(QString pProperty, QString pElement);
        QString getString(QString pProperty, QString pElement, long line);
        long getInt(QString pProperty, QString pElement);
        long getInt(QString pProperty, QString pElement, long line);
        double getFloat(QString pProperty, QString pElement);
        double getFloat(QString pProperty, QString pElement, long line);
        bool getBool(QString pProperty, QString pElement);
        bool getBool(QString pProperty, QString pElement, long line);
        QVariantMap getProperties(void)
        {
            //getQtProperties();
            return mProperties;
        }
        QMap<QString, OST::PropertyMulti*> getStore(void)
        {
            return mStore;
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
                sendWarning("createProperty - property " + pPropertyName + " already exists.");
                return false;
            }
            mStore[pPropertyName] = pProperty;
            OST::PropertyJsonDumper d;
            mStore[pPropertyName]->accept(&d);
            OnModuleEvent("cp", QString(), pPropertyName, d.getResult().toVariantMap());
            connect(mStore[pPropertyName], &OST::PropertyMulti::valueChanged, this, &Datastore::onValueChanged);
            return true;
        }

        //QVariant getOstPropertyValue(const QString &pPropertyName);

        bool createOstElementText(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                  bool mEmitEvent);
        bool createOstElementBool(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                  bool mEmitEvent);
        bool setOstElementValue(const QString &pPropertyName, const QString &pElementName, const QVariant &pElementValue,
                                bool mEmitEvent);
        QVariantList getOstElementGrid(const QString &pPropertyName, const QString &pElementName);

        void loadOstPropertiesFromFile(const QString &pFileName);
        void saveOstPropertiesToFile(const QString &pFileName);

        bool setOstElementAttribute (const QString &pPropertyName, const QString &pElementName, const  QString &pAttributeName,
                                     const QVariant &AttributeValue,
                                     bool mEmitEvent);

        void deleteOstProperty(const QString &pPropertyName);

        QVariant getOstElementLineValue(const QString &pPropertyName, const QString &pElementName, const double &pLine);
        bool setOstElementLineValue(const QString &pPropertyName, const QString &pElementName, const double &pLine,
                                    const QVariant &pElementValue);

        QVariantMap getProfile(void);

        QJsonObject getPropertiesDump(void);

    private slots:
        void onValueChanged(void);
        void onPropertyEvent(QString event, QString key, OST::PropertyBase* prop);
    private:
        QVariantMap mProperties;
        QMap<QString, OST::PropertyMulti*> mStore;

}
;
#endif

