#include "propertymulti.h"
#include "elementupdate.h"

namespace  OST
{

void PropertyMulti::accept(PropertyVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void PropertyMulti::accept(PropertyVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}

PropertyMulti::PropertyMulti(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                             const QString &level2,
                             const QString &order, const bool &hasProfile, const bool &hasGrid)
    : PropertyBase(key, label, permission, level1, level2,
                   order, hasProfile), mHasGrid(hasGrid)
{
}
PropertyMulti::~PropertyMulti()
{

}
[[nodiscard]] SwitchsRule PropertyMulti::rule() const
{
    return mRule;
}
void PropertyMulti::setRule(SwitchsRule rule)
{
    mRule = rule;
}

QMap<QString, ElementBase*>* PropertyMulti::getElts()
{
    return &mElts;
}
ElementBase* PropertyMulti::getElt(QString pElement)
{
    if (!mElts.contains(pElement))
    {
        qDebug() << label() << " - getElt - element " << pElement << " does not exists.";
        return nullptr;
    }
    return mElts[pElement];
}


QList<QString> PropertyMulti::getGridHeaders()
{
    return mElts.keys();
}
QList<QMap<QString, ValueBase*>> PropertyMulti::getGrid()
{
    return mGrid;
}
bool PropertyMulti::getShowElts()
{
    return mShowElts;
}
void PropertyMulti::setShowElts(bool b)
{
    mShowElts = b;
}
bool PropertyMulti::hasGrid()
{
    return mHasGrid;
}
void PropertyMulti::setHasGrid(bool b)
{
    mHasGrid = b;
}
bool PropertyMulti::getShowGrid()
{
    return mShowGrid;
}
void PropertyMulti::setShowGrid(bool b)
{
    mShowGrid = b;
}
int PropertyMulti::getGridLimit()
{
    return mGridLimit;
}
void PropertyMulti::setGridLimit(int limit)
{
    if (limit > 0) mGridLimit = limit;
}
bool PropertyMulti::hasGraph()
{
    return mHasGraph;
}
void PropertyMulti::setHasGraph(bool b)
{
    mHasGraph = b;
}
GraphDefs PropertyMulti::getGraphDefs(void)
{
    return mGraphDefs;
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
void  PropertyMulti::addElt(QString key, ElementBase* pElt)
{
    if (mElts.contains(key))
    {
        qDebug() << label() << " - addElt - element " << key << " already exists";
        return;
    }
    mElts[key] = pElt;
    mGrid.clear();
    connect(mElts[key], &ElementBase::eltChanged, this, &PropertyMulti::OnEltChanged);
    connect(mElts[key], &ElementBase::valueSet, this, &PropertyMulti::OnValueSet);
    connect(mElts[key], &ElementBase::listChanged, this, &PropertyMulti::OnListChanged);
    connect(mElts[key], &ElementBase::lovChanged, this, &PropertyMulti::OnLovChanged);
    connect(mElts[key], &ElementBase::sendMessage, this, &PropertyMulti::OnMessage);
}
void  PropertyMulti::deleteElt(QString key)
{
    if (!mElts.contains(key))
    {
        qDebug() << label() << " - deleteElt - element " << key << " doesn't exist";
        return;
    }
    mElts.remove(key);
    mGrid.clear();
    emit propertyEvent("ap", key, this);

}

void PropertyMulti::push()
{
    if (!this->hasGrid())
    {
        sendError("PropertyMulti::Push - no array/grid defined");
        return;
    }
    QMap<QString, ValueBase*> wGridLine;
    foreach(const QString &elt, mElts.keys())
    {
        wGridLine[elt] = ValueFactory::createValue(mElts[elt]);
    }
    mGrid.append(wGridLine);
    if (mGridLimit > 0 && mGrid.size() > this->mGridLimit) mGrid.removeFirst();

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
bool PropertyMulti::updateLine(const int i, const QVariantMap &pValues)
{
    if (!this->hasGrid())
    {
        sendError("PropertyMulti::updateLine - no array/grid defined");
        return false;
    }
    /* Check if data is valid and contains every value */
    foreach(const QString &elt, mElts.keys())
    {
        if (mElts[elt]->getType() == "int" || mElts[elt]->getType() == "float" || mElts[elt]->getType() == "string")
        {
            if (!pValues.contains(elt))
            {
                sendError("PropertyMulti::updateLine incomplete values, " + elt + " missing");
                return false;
            }
        }
    }
    //foreach(const QString &elt, mElts.keys())
    //{
    //    if (mElts[elt]->getType() == "int" || mElts[elt]->getType() == "float" || mElts[elt]->getType() == "string")
    //    {
    //        ElementUpdate d;
    //        QString action = "updateline";
    //        QVariantMap m;
    //        m["val"] = pValues[elt];
    //        m["i"] = i;
    //        mElts[elt]->accept(&d, action, m);
    //    }
    //}
    foreach(const QString &elt, mElts.keys())
    {
        setElt(elt, pValues[elt]);
    }

    foreach( ValueBase* v, mGrid[i])
    {
        v->updateValue();
    }

    emit propertyEvent("ap", key(), this);
    return true;

}
bool PropertyMulti::deleteLine(const int i)
{
    if (!this->hasGrid())
    {
        sendError("PropertyMulti::Push - no array/grid defined");
        return false;
    }
    mGrid.removeAt(i);
    emit propertyEvent("ap", key(), this);
    return true;
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
bool PropertyMulti::swapLines(int l1, int l2)
{
    if ((l1 >= mGrid.size()) || (l2 >= mGrid.size()))
    {
        sendWarning("Can't swap lines" + QString::number(l1) + "/" + QString::number(l2) + " > " + QString::number(mGrid.size()));
        return false;
    }
    if ((l1 < 0) || (l2 < 0))
    {
        sendWarning("Can't swap lines" + QString::number(l1) + "/" + QString::number(l2));
        return false;
    }
    mGrid.swapItemsAt(l2, l1);
    emit propertyEvent("ap", key(), this);
    return true;
}

bool PropertyMulti::fetchLine(int l)
{
    if ((l >= mGrid.size()))
    {
        sendWarning("Can't fetch line" + QString::number(l)  + " >= " + QString::number(mGrid.size()));
        return false;
    }
    foreach(const QString &e, mGrid.at(l).keys())
    {
        mGrid.at(l)[e]->updateElement(true);
    }
    emit propertyEvent("ap", key(), this);
    return true;
}
bool PropertyMulti::updateLine(const int i)
{
    if ((i >= mGrid.size()))
    {
        sendWarning("Can't update line" + QString::number(i)  + " >= " + QString::number(mGrid.size()));
        return false;
    }
    foreach(const QString &e, mGrid.at(i).keys())
    {
        mGrid.at(i)[e]->updateValue();
    }
    emit propertyEvent("ap", key(), this);
    return true;
}
bool PropertyMulti::autoUpDown(void)
{
    return mAutoUpDown;
}
void PropertyMulti::setAutoUpDown(bool b)
{
    mAutoUpDown = b;
}
bool PropertyMulti::autoSelect(void)
{
    return mAutoSelect;
}
void PropertyMulti::setAutoSelect(bool b)
{
    mAutoSelect = b;
}



/* Slots */
void PropertyMulti::OnValueSet(ElementBase*)
{
    emit valueSet(this);
}
void PropertyMulti::OnEltChanged(ElementBase*)
{
    emit eltChanged(this);
}
void PropertyMulti::OnListChanged(ElementBase*)
{
    emit propertyEvent("ap", key(), this);
}
void PropertyMulti::OnLovChanged(ElementBase*)
{
    emit propertyEvent("ap", key(), this);
}
void PropertyMulti::OnMessage(MsgLevel l, QString m)
{
    switch (l)
    {
        case Info:
            sendInfo(this->key() + "-" + m);
            break;
        case Warn:
            sendWarning(this->key() + "-" + m);
            break;
        case Err:
            sendError(this->key() + "-" + m);
            break;
        default:
            sendError(this->key() + "-" + m);
            break;
    }
}


}
