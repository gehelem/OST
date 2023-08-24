#ifndef DATASTORE_h_
#define DATASTORE_h_
//#include "jsondumper.h"
#include <baseroot.h>
#include <propertytextdumper.h>
#include <propertyupdate.h>
#include <propertyfactory.h>
#include <valueupdate.h>

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

        QVariantMap getProperties(void)
        {
            //getQtProperties();
            return mProperties;
        }
        void getQtProperties(void)
        {
            qDebug() << "****************************************";
            qDebug() << "****************************************";
            foreach(const QString &key, mStore.keys())
            {
                OST::PropertyTextDumper d;

                mStore[key]->accept(&d);
                qDebug() << key << "-" << d.getResult();
                //JSonDumper j;
                //mStore[key]->accept(&j);
                //qDebug() << key << "-" << j.getStringResult();
            }
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
        //QVariant getOstPropertyValue(const QString &pPropertyName);

        bool createOstElement(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
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

    private slots:
        void onValueChanged(void);
    private:
        QVariantMap mProperties;
        QMap<QString, OST::PropertyMulti*> mStore;

}
;
#endif

