#ifndef DATASTORE_h_
#define DATASTORE_h_
#include <QVariant>
#include <baseroot.h>

class Datastore : public Baseroot
{
    public:
        Datastore();
        ~Datastore();
    protected:

        bool createProperty(const QString &pPropertyName, const QString &pPropertyLabel, bool mEmitEvent);
        bool setPropertyValue(const QString &pPropertyName, const QVariant &pPropertyValue, bool mEmitEvent);
        QVariant getPropertyValue(QString &pPropertyName);

        bool createElement(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                           bool mEmitEvent);
        bool setElementValue(const QString &pPropertyName, const QString &pElementName, const QVariant &pElementValue,
                             bool mEmitEvent);
        QVariant getElementValue(const QString &pPropertyName, const QString &pElementName);

    private:
        QVariantMap mProperties;
}
;
#endif

