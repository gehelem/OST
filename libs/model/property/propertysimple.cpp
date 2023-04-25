#include "propertysimple.h"
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

}