#include "propertyupdate.h"
#include <sstream>
namespace  OST
{
void PropertyUpdate::visit(PropertyMulti *pProperty, QVariantMap &data )
{
    if (!data.contains("elements"))
    {
        qDebug() << "no elements for " << pProperty->label();
    }
    //pProperty->setValue(data);
}
}

