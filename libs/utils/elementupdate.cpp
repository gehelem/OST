#include "elementupdate.h"
namespace  OST
{
void ElementUpdate::visit(ElementInt *pElement, QVariantMap &data, bool &emitEvent)
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toLongLong(), emitEvent);
}
void ElementUpdate::visit(ElementFloat *pElement, QVariantMap &data, bool &emitEvent)
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toDouble(), emitEvent);
}
void ElementUpdate::visit(ElementBool *pElement, QVariantMap &data, bool &emitEvent)
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toBool(), emitEvent);
}
void ElementUpdate::visit(ElementString *pElement, QVariantMap &data, bool &emitEvent)
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
    pElement->setValue(data["value"].toString(), emitEvent);
}
void ElementUpdate::visit(ElementLight *pElement, QVariantMap &data, bool &emitEvent )
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    if (data["value"].toInt() == 0 ) pElement->setValue(Idle, emitEvent);
    if (data["value"].toInt() == 1 ) pElement->setValue(Ok, emitEvent);
    if (data["value"].toInt() == 2 ) pElement->setValue(Busy, emitEvent);
    if (data["value"].toInt() == 3 ) pElement->setValue(Error, emitEvent);
}
void ElementUpdate::visit(ElementImg *pElement, QVariantMap &data, bool &emitEvent )
{
    Q_UNUSED(pElement) Q_UNUSED(data)  Q_UNUSED(emitEvent)
}
void ElementUpdate::visit(ElementVideo *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(pElement) Q_UNUSED(data)  Q_UNUSED(emitEvent)
}
void ElementUpdate::visit(ElementPrg* pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(pElement)  Q_UNUSED(data)  Q_UNUSED(emitEvent)
}
void ElementUpdate::visit(ElementDate *pElement, QVariantMap &data, bool &emitEvent)
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    QVariantMap m = data["value"].toMap();
    QDate d;
    d.setDate(m["year"].toInt(), m["month"].toInt(), m["day"].toInt());
    pElement->setValue(d, emitEvent);

}
void ElementUpdate::visit(ElementTime* pElement, QVariantMap &data, bool &emitEvent)
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    QVariantMap m = data["value"].toMap();
    QTime t;
    if (pElement->getUseMs())
    {
        t.setHMS(m["hh"].toInt(), m["mm"].toInt(), m["ss"].toInt(), m["ms"].toInt());
    }
    else
    {
        t.setHMS(m["hh"].toInt(), m["mm"].toInt(), m["ss"].toInt(), 0);
    }
    pElement->setValue(t, emitEvent);
}
void ElementUpdate::visit(ElementDateTime* pElement, QVariantMap &data, bool &emitEvent)
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    QVariantMap m = data["value"].toMap();
    QDate d;
    d.setDate(m["year"].toInt(), m["month"].toInt(), m["day"].toInt());
    QTime t;
    t.setHMS(m["hh"].toInt(), m["mm"].toInt(), m["ss"].toInt(), m["ms"].toInt());
    QDateTime dt;
    dt.setDate(d);
    dt.setTime(t);
    pElement->setValue(dt, emitEvent);


}

}
