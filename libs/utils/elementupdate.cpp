#include "elementupdate.h"
namespace  OST
{
void ElementUpdate::visit(ElementInt *pElement, QVariantMap &data, SignalType &signalType)
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toLongLong(), signalType);
}
void ElementUpdate::visit(ElementFloat *pElement, QVariantMap &data, SignalType &signalType)
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toDouble(), signalType);
}
void ElementUpdate::visit(ElementBool *pElement, QVariantMap &data, SignalType &signalType)
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toBool(), signalType);
}
void ElementUpdate::visit(ElementString *pElement, QVariantMap &data, SignalType &signalType)
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
    pElement->setValue(data["value"].toString(), signalType);
}
void ElementUpdate::visit(ElementLight *pElement, QVariantMap &data, SignalType &signalType )
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    if (data["value"].toInt() == 0 ) pElement->setValue(Idle, signalType);
    if (data["value"].toInt() == 1 ) pElement->setValue(Ok, signalType);
    if (data["value"].toInt() == 2 ) pElement->setValue(Busy, signalType);
    if (data["value"].toInt() == 3 ) pElement->setValue(Error, signalType);
}
void ElementUpdate::visit(ElementImg *pElement, QVariantMap &data, SignalType &signalType )
{
    Q_UNUSED(pElement) Q_UNUSED(data)  Q_UNUSED(signalType)
}
void ElementUpdate::visit(ElementVideo *pElement, QVariantMap &data, SignalType &signalType)
{
    Q_UNUSED(pElement) Q_UNUSED(data)  Q_UNUSED(signalType)
}
void ElementUpdate::visit(ElementPrg* pElement, QVariantMap &data, SignalType &signalType)
{
    Q_UNUSED(pElement)  Q_UNUSED(data)  Q_UNUSED(signalType)
}
void ElementUpdate::visit(ElementDate* pElement, QVariantMap &data, SignalType &signalType)
{
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    QVariantMap m = data["value"].toMap();
    QDate d;
    d.setDate(m["year"].toInt(), m["month"].toInt(), m["day"].toInt());
    pElement->setValue(d, signalType);

}
void ElementUpdate::visit(ElementTime* pElement, QVariantMap &data, SignalType &signalType)
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
    pElement->setValue(t, signalType);
}

}
