#include "propertymulti.h"
#include "elementupdate.h"

namespace  OST
{

PropertyMulti::PropertyMulti(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                             const QString &level2,
                             const QString &order, const bool &hasProfile, const bool &hasGrid)
    : PropertyBase(key, label, permission, level1, level2,
                   order, hasProfile), mHasGrid(hasGrid)
{
    emit propertyCreated();
}
PropertyMulti::~PropertyMulti()
{

}
bool  PropertyMulti::setElt(QString key, QVariant val)
{
    if (!mElts.contains(key))
    {
        sendError("PropertyMulti::setValue - " + key + " - not found");
        return false;
    }
    if (((mElts[key]->getType() == "int") && (val.canConvert<long>())) ||
            ((mElts[key]->getType() == "string") && (val.canConvert<QString>())) ||
            ((mElts[key]->getType() == "float") && (val.canConvert<double>())) )
    {
        ElementUpdate vu;
        QVariantMap m;
        m["value"] = val;
        getElt(key)->accept(&vu, m);
        return true;
    }
    if ((mElts[key]->getType() == "bool") && (val.canConvert<bool>()) )
    {
        ElementUpdate vu;
        QVariantMap m;

        switch (this->rule())
        {
            case Any:
                m["value"] = val;
                getElt(key)->accept(&vu, m);
                return true;
                break;
            case OneOfMany:
                if (val.toBool())
                {
                    foreach(const QString &elt, mElts.keys())
                    {
                        if ((mElts[elt]->getType() == "bool") && (elt != key))
                        {
                            m["value"] = false;
                            mElts[elt]->accept(&vu, m);
                        }
                    }
                    m["value"] = val;
                    mElts[key]->accept(&vu, m);
                    return true;

                }
                sendError("PropertyMulti::setValue - " + key + " - OneOfMany - can't just unset");
                return false;
                break;
            case AtMostOne:
                foreach(const QString &elt, mElts.keys())
                {
                    if ((mElts[elt]->getType() == "bool") && (elt != key))
                    {
                        m["value"] = false;
                        mElts[elt]->accept(&vu, m);
                    }
                }
                m["value"] = val;
                mElts[key]->accept(&vu, m);
                return true;
                break;
            default:
                sendError("PropertyMulti::setValue - " + key + " - can't determine SwitchRule");
                return false;
                break;
        }
    }
    sendError("PropertyMulti::setValue - " + key + " - can't update, unhandled type : "
              + mElts[key]->getType() + "(" + val.toString() + ")");
    return false;


}

void PropertyMulti::push()
{
    if (!this->hasGrid())
    {
        sendError("PropertyMulti::Push - no array/grid defined");
        return;
    }
    QMap<QString, ValueBase*> wGridLine;
    foreach(const QString &elt, mGridHeaders)
    {
        wGridLine[elt] = ValueFactory::createValue(mElts[elt]);
    }
    mGrid.append(wGridLine);
    emit propertyEvent("ap", key(), this);

}

void PropertyMulti::newLine(const QVariantMap &pValues)
{
    if (!this->hasGrid())
    {
        sendError("PropertyMulti::newLine - no array/grid defined");
        return;
    }
    /* Check if data is valid and contains every value */
    foreach(const QString &elt, mElts.keys())
    {
        if ((mElts[elt]->getType() == "int") || (mElts[elt]->getType() == "float") || (mElts[elt]->getType() == "string"))
        {
            if (!pValues.contains(elt))
            {
                sendWarning("PropertyMulti::newLine incomplete values, " + elt + " missing ");
                return;
            }
        }
    }

    foreach(const QString &elt, mElts.keys())
    {

        if ((mElts[elt]->getType() == "int") || (mElts[elt]->getType() == "float") || (mElts[elt]->getType() == "string"))
        {
            ElementUpdate d;
            QString action = "newline";
            QVariantMap m;
            m["val"] = pValues[elt];
            mElts[elt]->accept(&d, action, m);
        }
    }


    foreach(const QString &elt, mElts.keys())
    {
        setElt(elt, pValues[elt]);

    }
    push();

    emit propertyEvent("ap", key(), this);

}
void PropertyMulti::deleteLine(const int i)
{
    if (!this->hasGrid())
    {
        sendError("PropertyMulti::Push - no array/grid defined");
        return;
    }

    foreach(const QString &elt, mElts.keys())
    {
        ElementUpdate d;
        QString action = "deleteline";
        QVariantMap m;
        m["i"] = i;
        mElts[elt]->accept(&d, action, m);
    }

    mGrid.removeAt(i);

    emit propertyEvent("ap", key(), this);

}
void PropertyMulti::updateLine(const int i, const QVariantMap &pValues)
{
    if (!this->hasGrid())
    {
        sendError("PropertyMulti::Push - no array/grid defined");
        return;
    }

    /* Check if data is valid and contains every value */
    foreach(const QString &elt, mElts.keys())
    {
        if (mElts[elt]->getType() == "int" || mElts[elt]->getType() == "float" || mElts[elt]->getType() == "string")
        {
            if (!pValues.contains(elt))
            {
                sendError("PropertyMulti::updateLine incomplete values, " + elt + " missing");
                return;
            }
        }
    }
    foreach(const QString &elt, mElts.keys())
    {
        if (mElts[elt]->getType() == "int" || mElts[elt]->getType() == "float" || mElts[elt]->getType() == "string")
        {
            ElementUpdate d;
            QString action = "updateline";
            QVariantMap m;
            m["val"] = pValues[elt];
            m["i"] = i;
            mElts[elt]->accept(&d, action, m);
        }
    }
    foreach(const QString &elt, mElts.keys())
    {
        setElt(elt, pValues[elt]);
    }

    foreach( ValueBase* v, mGrid[i])
    {
        v->updateValue();
    }

    emit propertyEvent("ap", key(), this);

}
void PropertyMulti::clearGrid()
{
    if (!this->hasGrid())
    {
        sendError("PropertyMulti::Push - no array/grid defined");
        return;
    }

    mGrid.clear();
    emit propertyEvent("ap", key(), this);

}

void PropertyMulti::setGraphDefs(GraphDefs defs)
{

    mHasGraph = false;
    switch (defs.type)
    {
        case none:
            mGraphDefs.params = QVariantMap();
            mGraphDefs.type = none;
            return;
            break;
        case XY:
            if (!defs.params.contains("X"))
            {
                qDebug() << "XY graph definition error : param should contain X binding";
                return;
            }
            if (!defs.params.contains("Y"))
            {
                qDebug() << "XY graph definition error : param should contain Y binding";
                return;
            }
            mHasGraph = true;
            break;
        case DY:
            if (!defs.params.contains("D"))
            {
                qDebug() << "DY graph definition error : param should contain D binding";
                return;
            }
            if (!defs.params.contains("Y"))
            {
                qDebug() << "DY graph definition error : param should contain Y binding";
                return;
            }
            mHasGraph = true;
            break;
        case PHD:
            if (!defs.params.contains("D"))
            {
                qDebug() << "PHD graph definition error : param should contain D binding";
                return;
            }
            if (!defs.params.contains("RA"))
            {
                qDebug() << "PHD graph definition error : param should contain RA binding";
                return;
            }
            if (!defs.params.contains("DE"))
            {
                qDebug() << "PHD graph definition error : param should contain DE binding";
                return;
            }
            if (!defs.params.contains("pRA"))
            {
                qDebug() << "PHD graph definition error : param should contain pRA binding";
                return;
            }
            if (!defs.params.contains("pDE"))
            {
                qDebug() << "PHD graph definition error : param should contain pDE binding";
                return;
            }
            mHasGraph = true;
            break;
        default:
            qDebug() << "setGraphDefs unknown graph def";
            mGraphDefs.params = QVariantMap();
            mGraphDefs.type = none;
            return;
            break;
    }
    mGraphDefs = defs;
}


}
