#include "valueupdate.h"
#include <sstream>
namespace  OST
{
void ValueUpdate::visit(ValueInt *pValue, QVariantMap &data )
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
    }
    pValue->setValue(data["value"].toLongLong());
}
void ValueUpdate::visit(ValueFloat *pValue, QVariantMap &data )
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
    }
    pValue->setValue(data["value"].toDouble());
}
void ValueUpdate::visit(ValueBool *pValue, QVariantMap &data )
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
    }
    pValue->setValue(data["value"].toBool());
}
void ValueUpdate::visit(ValueString *pValue, QVariantMap &data )
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
    }
    pValue->setValue(data["value"].toString());
}
void ValueUpdate::visit(ValueLight *pValue, QVariantMap &data )
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
    }
    if (data["value"].toInt() == 0 ) pValue->setState(Idle);
    if (data["value"].toInt() == 1 ) pValue->setState(Ok);
    if (data["value"].toInt() == 2 ) pValue->setState(Busy);
    if (data["value"].toInt() == 3 ) pValue->setState(Error);

}
void ValueUpdate::visit(ValueImg *pValue, QVariantMap &data )
{
}
void ValueUpdate::visit(ValueMessage *pValue, QVariantMap &data )
{
}

}
