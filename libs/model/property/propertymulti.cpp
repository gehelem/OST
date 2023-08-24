#include "propertymulti.h"
namespace  OST
{

PropertyMulti::PropertyMulti(const QString &label, const Permission &permission, const QString &level1,
                             const QString &level2,
                             const QString &order, const bool &hasProfile, const bool &hasArray)
    : PropertyBase(label, permission, level1, level2,
                   order, hasProfile, hasArray)
{
    emit propertyCreated();
}
PropertyMulti::~PropertyMulti()
{

}

}
