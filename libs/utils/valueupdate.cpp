#include "valueupdate.h"
#include <sstream>
namespace  OST
{
void ValueUpdate::visit(ValueInt *pValue, QVariantMap &data )
{
    if (data.contains("gridaction"))
    {
        if (data["gridaction"].toString() == "push")
        {
            pValue->grid.add(pValue->value());
            return;
        }
    }
    if (data.contains("autoupdate")) pValue->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
        return;
    }
    pValue->setValue(data["value"].toLongLong());
}
void ValueUpdate::visit(ValueFloat *pValue, QVariantMap &data )
{
    if (data.contains("autoupdate")) pValue->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
        return;
    }
    pValue->setValue(data["value"].toDouble());
}
void ValueUpdate::visit(ValueBool *pValue, QVariantMap &data )
{
    if (data.contains("autoupdate")) pValue->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
        return;
    }
    pValue->setValue(data["value"].toBool());
}
void ValueUpdate::visit(ValueString *pValue, QVariantMap &data )
{
    if (data.contains("autoupdate"))
    {
        qDebug() << "autoupdate" << pValue->label() << "/" << data;
        pValue->setAutoUpdate(data["autoupdate"].toBool());
    }
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
        return;
    }
    pValue->setValue(data["value"].toString(), true);
}
void ValueUpdate::visit(ValueLight *pValue, QVariantMap &data )
{
    if (data.contains("autoupdate")) pValue->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
        return;
    }
    if (data["value"].toInt() == 0 ) pValue->setState(Idle);
    if (data["value"].toInt() == 1 ) pValue->setState(Ok);
    if (data["value"].toInt() == 2 ) pValue->setState(Busy);
    if (data["value"].toInt() == 3 ) pValue->setState(Error);

}
void ValueUpdate::visit(ValueImg *pValue, QVariantMap &data )
{
    Q_UNUSED(pValue);
    Q_UNUSED(data);
}
void ValueUpdate::visit(ValueMessage *pValue, QVariantMap &data )
{
    Q_UNUSED(pValue);
    Q_UNUSED(data);
}

void ValueUpdate::visit(ValueInt* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(data);
    if (action == "push")
    {
        pValue->grid.add(pValue->value());
    }
}
void ValueUpdate::visit(ValueFloat* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(data);
    if (action == "push")
    {
        pValue->grid.add(pValue->value());
    }

}
void ValueUpdate::visit(ValueBool* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(data);
    if (action == "push")
    {
        pValue->grid.add(pValue->value());
    }

}
void ValueUpdate::visit(ValueString* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(data);
    if (action == "push")
    {
        pValue->grid.add(pValue->value());
    }

}
void ValueUpdate::visit(ValueLight* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(data);
    if (action == "push")
    {
        pValue->grid.add(pValue->state());
    }

}
void ValueUpdate::visit(ValueImg* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(pValue);
    Q_UNUSED(action);
    Q_UNUSED(data);
}
void ValueUpdate::visit(ValueMessage* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(pValue);
    Q_UNUSED(action);
    Q_UNUSED(data);

}


}
