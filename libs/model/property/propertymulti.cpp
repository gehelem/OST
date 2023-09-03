#include "propertymulti.h"
#include "valueupdate.h"

namespace  OST
{

PropertyMulti::PropertyMulti(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                             const QString &level2,
                             const QString &order, const bool &hasProfile, const bool &hasArray)
    : PropertyBase(key, label, permission, level1, level2,
                   order, hasProfile, hasArray)
{
    emit propertyCreated();
}
PropertyMulti::~PropertyMulti()
{

}
void PropertyMulti::push()
{
    if (!this->hasArray())
    {
        qDebug() << "PropertyMulti::Push - no array/grid defined";
        return;
    }
    foreach(const QString &elt, mValues.keys())
    {
        OST::ValueUpdate d;
        QString action = "push";
        QVariantMap m;
        mValues[elt]->accept(&d, action, m);
    }
    emit propertyEvent("ap", key(), this);

}

void PropertyMulti::newLine(const QVariantMap &pValues)
{
    if (!this->hasArray())
    {
        qDebug() << "PropertyMulti::Push - no array/grid defined";
        return;
    }
    /* Check if data is valid and contains every value */
    foreach(const QString &elt, mValues.keys())
    {
        if (!pValues.contains(elt))
        {
            qDebug() << "PropertyMulti::newLine incomplete values, " << elt << " missing in " << pValues;
            return;
        }
    }

    foreach(const QString &elt, mValues.keys())
    {
        OST::ValueUpdate d;
        QString action = "newline";
        QVariantMap m;
        m["val"] = pValues[elt];
        mValues[elt]->accept(&d, action, m);
    }
    emit propertyEvent("ap", key(), this);

}
void PropertyMulti::deleteLine(const int i)
{
    if (!this->hasArray())
    {
        qDebug() << "PropertyMulti::Push - no array/grid defined";
        return;
    }

    foreach(const QString &elt, mValues.keys())
    {
        OST::ValueUpdate d;
        QString action = "deleteline";
        QVariantMap m;
        m["i"] = i;
        mValues[elt]->accept(&d, action, m);
    }
    emit propertyEvent("ap", key(), this);

}
void PropertyMulti::updateLine(const int i, const QVariantMap &pValues)
{
    if (!this->hasArray())
    {
        qDebug() << "PropertyMulti::Push - no array/grid defined";
        return;
    }

    /* Check if data is valid and contains every value */
    foreach(const QString &elt, mValues.keys())
    {
        if (!pValues.contains(elt))
        {
            qDebug() << "PropertyMulti::updateLine incomplete values, " << elt << " missing in " << pValues;
            return;
        }
    }
    foreach(const QString &elt, mValues.keys())
    {
        OST::ValueUpdate d;
        QString action = "updateline";
        QVariantMap m;
        m["val"] = pValues[elt];
        m["i"] = i;
        mValues[elt]->accept(&d, action, m);
    }
    emit propertyEvent("ap", key(), this);

}
void PropertyMulti::clearGrid()
{
    if (!this->hasArray())
    {
        qDebug() << "PropertyMulti::Push - no array/grid defined";
        return;
    }

    foreach(const QString &elt, mValues.keys())
    {
        OST::ValueUpdate d;
        QString action = "cleargrid";
        QVariantMap m;
        mValues[elt]->accept(&d, action, m);
    }
    emit propertyEvent("ap", key(), this);

}

}
