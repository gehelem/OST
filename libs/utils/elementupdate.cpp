#include "elementupdate.h"
#include <sstream>
namespace  OST
{
void ElementUpdate::visit(ElementInt *pElement, QVariantMap &data )
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toLongLong(), true);
}
void ElementUpdate::visit(ElementFloat *pElement, QVariantMap &data )
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toDouble(), true);
}
void ElementUpdate::visit(ElementBool *pElement, QVariantMap &data )
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toBool(), true);
}
void ElementUpdate::visit(ElementString *pElement, QVariantMap &data )
{
    if (data.contains("autoupdate"))
    {
        qDebug() << "autoupdate" << pElement->label() << "/" << data;
        pElement->setAutoUpdate(data["autoupdate"].toBool());
    }
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toString(), true);
}
void ElementUpdate::visit(ElementLight *pElement, QVariantMap &data )
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    if (data["value"].toInt() == 0 ) pElement->setValue(Idle, true);
    if (data["value"].toInt() == 1 ) pElement->setValue(Ok, true);
    if (data["value"].toInt() == 2 ) pElement->setValue(Busy, true);
    if (data["value"].toInt() == 3 ) pElement->setValue(Error, true);
}
void ElementUpdate::visit(ElementImg *pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}
void ElementUpdate::visit(ElementVideo *pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}
void ElementUpdate::visit(ElementMessage *pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}
void ElementUpdate::visit(ElementPrg* pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}

void ElementUpdate::visit(ElementInt* pElement, QString &action, QVariantMap &data)
{
}
void ElementUpdate::visit(ElementFloat* pElement, QString &action, QVariantMap &data)
{
}
void ElementUpdate::visit(ElementBool* pElement, QString &action, QVariantMap &data)
{
}
void ElementUpdate::visit(ElementString* pElement, QString &action, QVariantMap &data)
{
}
void ElementUpdate::visit(ElementLight* pElement, QString &action, QVariantMap &data)
{
}
void ElementUpdate::visit(ElementImg* pElement, QString &action, QVariantMap &data)
{
    Q_UNUSED(pElement);
    Q_UNUSED(action);
    Q_UNUSED(data);

}
void ElementUpdate::visit(ElementVideo* pElement, QString &action, QVariantMap &data)
{
    Q_UNUSED(pElement);
    Q_UNUSED(action);
    Q_UNUSED(data);
}
void ElementUpdate::visit(ElementMessage* pElement, QString &action, QVariantMap &data)
{
    Q_UNUSED(pElement);
    Q_UNUSED(action);
    Q_UNUSED(data);

}
void ElementUpdate::visit(ElementPrg* pElement, QString &action, QVariantMap &data)
{
}

}
