#include "propertymulti.h"
#include "valueupdate.h"

namespace  OST
{

PropertyMulti::PropertyMulti(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                             const QString &level2,
                             const QString &order, const bool &hasProfile, const bool &hasArray)
    : PropertyBase(key, label, permission, level1, level2,
                   order, hasProfile, hasArray)
{
    emit propertyCreated();
}
PropertyMulti::~PropertyMulti()
{

}
void PropertyMulti::push()
{
    foreach(const QString &key, mValues.keys())
    {
        OST::ValueUpdate d;
        QString action = "push";
        QVariantMap m;
        mValues[key]->accept(&d, action, m);
    }

}

}
