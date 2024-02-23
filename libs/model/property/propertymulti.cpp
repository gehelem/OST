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
bool  PropertyMulti::setValue(QString key, QVariant pValue)
{
    if (!mValues.contains(key))
    {
        sendError("PropertyMulti::setValue - " + key + " - not found");
        return false;
    }
    if (((mValues[key]->getType() == "int") && (pValue.canConvert<long>())) ||
            ((mValues[key]->getType() == "string") && (pValue.canConvert<QString>())) ||
            ((mValues[key]->getType() == "float") && (pValue.canConvert<double>())) )
    {
        OST::ValueUpdate vu;
        QVariantMap m;
        m["value"] = pValue;
        getValue(key)->accept(&vu, m);
        return true;
    }
    if ((mValues[key]->getType() == "bool") && (pValue.canConvert<bool>()) )
    {
        OST::ValueUpdate vu;
        QVariantMap m;

        switch (this->rule())
        {
            case Any:
                m["value"] = pValue;
                getValue(key)->accept(&vu, m);
                return true;
                break;
            case OneOfMany:
                if (pValue.toBool())
                {
                    foreach(const QString &elt, mValues.keys())
                    {
                        if ((mValues[elt]->getType() == "bool") && (elt != key))
                        {
                            m["value"] = false;
                            mValues[elt]->accept(&vu, m);
                        }
                    }
                    m["value"] = pValue;
                    mValues[key]->accept(&vu, m);
                    return true;

                }
                sendError("PropertyMulti::setValue - " + key + " - OneOfMany - can't just unset");
                return false;
                break;
            case AtMostOne:
                foreach(const QString &elt, mValues.keys())
                {
                    if ((mValues[elt]->getType() == "bool") && (elt != key))
                    {
                        m["value"] = false;
                        mValues[elt]->accept(&vu, m);
                    }
                }
                m["value"] = pValue;
                mValues[key]->accept(&vu, m);
                return true;
                break;
            default:
                sendError("PropertyMulti::setValue - " + key + " - can't determine SwitchRule");
                return false;
                break;
        }
    }
    sendError("PropertyMulti::setValue - " + key + " - can't update, unhandled type : "
              + mValues[key]->getType() + "(" + pValue.toString() + ")");
    return false;


}

void PropertyMulti::push()
{
    if (!this->hasArray())
    {
        sendError("PropertyMulti::Push - no array/grid defined");
        return;
    }
    foreach(const QString &elt, mValues.keys())
    {
        OST::ValueUpdate d;
        QString action = "push";
        QVariantMap m;
        m["arrayLimit"] = this->getArrayLimit();
        mValues[elt]->accept(&d, action, m);
    }
    emit propertyEvent("ap", key(), this);

}

void PropertyMulti::newLine(const QVariantMap &pValues)
{
    if (!this->hasArray())
    {
        sendError("PropertyMulti::newLine - no array/grid defined");
        return;
    }
    /* Check if data is valid and contains every value */
    foreach(const QString &elt, mValues.keys())
    {
        if ((mValues[elt]->getType() == "int") || (mValues[elt]->getType() == "float") || (mValues[elt]->getType() == "string")
                || (mValues[elt]->getType() == "bool") || (mValues[elt]->getType() == "img") || (mValues[elt]->getType() == "prg")
                || (mValues[elt]->getType() == "video"))
        {
            if (!pValues.contains(elt))
            {
                sendWarning("PropertyMulti::newLine incomplete values, " + elt + " missing ");
                return;
            }
        }
    }

    foreach(const QString &elt, mValues.keys())
    {
        OST::ValueUpdate d;
        QString action = "newline";
        QVariantMap m;
        m["val"] = pValues[elt];
        m["arrayLimit"] = this->getArrayLimit();
        mValues[elt]->accept(&d, action, m);
    }
    emit propertyEvent("ap", key(), this);

}
void PropertyMulti::deleteLine(const int i)
{
    if (!this->hasArray())
    {
        sendError("PropertyMulti::Push - no array/grid defined");
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
        sendError("PropertyMulti::Push - no array/grid defined");
        return;
    }

    /* Check if data is valid and contains every value */
    foreach(const QString &elt, mValues.keys())
    {
        if (!pValues.contains(elt))
        {
            sendError("PropertyMulti::updateLine incomplete values, " + elt + " missing");
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
        sendError("PropertyMulti::Push - no array/grid defined");
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
