#include "propertyupdate.h"
#include <sstream>
#include <basicproperty.h>
#include <numberproperty.h>
#include <textproperty.h>

void PropertyUpdate::visit(BasicProperty *pProperty, QVariantMap &data )
{
    Q_UNUSED(pProperty)
    Q_UNUSED(data)
}
void PropertyUpdate::visit(TextProperty *pProperty, QVariantMap &data)
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pProperty->label();
    }
    pProperty->setValue(data["value"].toString());
}
void PropertyUpdate::visit(NumberProperty *pProperty, QVariantMap &data )
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pProperty->label();
    }
    pProperty->setValue(data["value"].toDouble());
}


