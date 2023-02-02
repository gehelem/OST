#ifndef DATASTORE_h_
#define DATASTORE_h_
#include <QVariant>
#include <baseroot.h>

/** @class Datastore
class Datastore : virtual public Baseroot
{
    public:
        Datastore();
        ~Datastore();
    protected:

        QVariantMap getProperties(void)
        {
            return mProperties;
        }

        bool createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel, const int &pPropertyPermission,
                               const  QString &pPropertyDevcat, const QString &pPropertyGroup);
        bool setOstPropertyValue(const QString &pPropertyName, const QVariant &pPropertyValue, bool mEmitEvent);
        QVariant getOstPropertyValue(QString &pPropertyName);

        bool createOstElement(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                              bool mEmitEvent);
        bool setOstElementValue(const QString &pPropertyName, const QString &pElementName, const QVariant &pElementValue,
                                bool mEmitEvent);
        QVariant getOstElementValue(const QString &pPropertyName, const QString &pElementName);

        void loadOstPropertiesFromFile(const QString &pFileName);
        void saveOstPropertiesToFile(const QString &pFileName);

        void setOstPropertyAttribute   (const QString &pPropertyName, const QString &pAttributeName, const QVariant &AttributeValue,
                                        bool mEmitEvent);
        bool setOstElementAttribute (const QString &pPropertyName, const QString &pElementName, const  QString &pAttributeName,
                                     const QVariant &AttributeValue,
                                     bool mEmitEvent);


    private:
        QVariantMap mProperties;
}
;
#endif

