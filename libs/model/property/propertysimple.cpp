#include "propertysimple.h"
#include <valueupdate.h>
#include <valuefactory.h>
namespace  OST
{

PropertySimple::PropertySimple(const QString &label, const Permission &permission, const QString &level1,
                               const QString &level2,
                               const QString &order, const bool &hasProfile, const bool &hasArray)
    : PropertyBase(label, permission, level1, level2,
                   order, hasProfile, hasArray)
{

    emit propertyCreated();
}
PropertySimple::~PropertySimple()
{

}
void PropertySimple::setValue(QVariantMap &data)
{
    qDebug() << "setValue " << label();
    QVariantMap dta;
    dta["value"] = data["value"];
    if (mValue == nullptr)
    {
        qDebug() << "setValue - create simple value" << label();
        mValue = ValueFactory::createValue(dta);
    }
    ValueUpdate v;
    mValue->accept(&v, dta);
}


}
