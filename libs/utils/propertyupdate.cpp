#include "propertyupdate.h"
#include <sstream>
namespace  OST
{
void PropertyUpdate::visit(PropertySimple *pProperty, QVariantMap &data)
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pProperty->label();
    }
    pProperty->getValue;
    pProperty->setValue(data["value"].toString());
}
void PropertyUpdate::visit(PropertyMulti *pProperty, QVariantMap &data )
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pProperty->label();
    }
    pProperty->setValue(data["value"].toDouble());
}
}

