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
    if (limit > 0)
    {
        if (limit < mGridLimit) clearGrid();
        if (limit > 5000 )
        {
            logWarning("gridLimit max size is 5000 " + this->label());
            mGridLimit = 5000;
        }
        else
        {
            mGridLimit = limit;
        }
    }
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
    return setElt(key, val, false);
}
bool  PropertyMulti::setElt(QString key, QVariant val, bool emitEvent)
{
    if (!mElts.contains(key))
    {
        logError("PropertyMulti::setValue - %1 - not found", {key});
        return false;
    }
    QVariantMap m;
    bool s = emitEvent;
    if (((mElts[key]->getType() == "int") && (val.canConvert<long>())) ||
            ((mElts[key]->getType() == "string") && (val.canConvert<QString>())) ||
            ((mElts[key]->getType() == "float") && (val.canConvert<double>())) )
    {
        ElementUpdate vu;
        m["value"] = val;
        getElt(key)->accept(&vu, m, s);
        return true;
    }
    if (mElts[key]->getType() == "time")
    {
        ElementUpdate vu;
        m["value"] = val.toMap();
        getElt(key)->accept(&vu, m, s);
        return true;
    }
    if (mElts[key]->getType() == "date")
    {
        ElementUpdate vu;
        m["value"] = val.toMap();
        getElt(key)->accept(&vu, m, s);
        return true;
    }
    if (mElts[key]->getType() == "light")
    {
        ElementUpdate vu;
        m["value"] = val.toMap();
        getElt(key)->accept(&vu, m, s);
        return true;
    }

    if ((mElts[key]->getType() == "bool") && (val.canConvert<bool>()) )
    {
        ElementUpdate vu;

        switch (this->rule())
        {
            case Any:
                m["value"] = val;
                getElt(key)->accept(&vu, m, s);
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
                            mElts[elt]->accept(&vu, m, s);
                        }
                    }
                    m["value"] = val;
                    mElts[key]->accept(&vu, m, s);
                    return true;

                }
                //logWarning("PropertyMulti::setValue - " + key + " - OneOfMany - can't just unset");
                return false;
                break;
            case AtMostOne:
                foreach(const QString &elt, mElts.keys())
                {
                    if ((mElts[elt]->getType() == "bool") && (elt != key))
                    {
                        m["value"] = false;
                        mElts[elt]->accept(&vu, m, s);
                    }
                }
                m["value"] = val;
                mElts[key]->accept(&vu, m, s);
                return true;
                break;
            default:
                logError("PropertyMulti::setValue - " + key + " - can't determine SwitchRule");
                return false;
                break;
        }
    }
    if ((mElts[key]->getType() == "prg") || (mElts[key]->getType() == "img") || (mElts[key]->getType() == "video") )
    {
        // prg/img/video can't be updated via frontend
        return true;
    }
    logError("PropertyMulti::setValue - " + key + " - can't update, unhandled type : "
             + mElts[key]->getType() + "(" + val.toString() + ")");
    return false;


}
void  PropertyMulti::addElt(ElementBase* pElt)
{
    QString key = pElt->key();
    if (mElts.contains(key))
    {
        qDebug() << label() << " - addElt - element " << key << " already exists";
        return;
    }
    mElts[key] = pElt;
    mGrid.clear();
    connect(mElts[key], &ElementBase::eltEvent, this, &PropertyMulti::OnEltEvent);
    connect(mElts[key], &ElementBase::lovChanged, this, &PropertyMulti::OnLovChanged);
    connect(mElts[key], &ElementBase::logMessage, this, &PropertyMulti::OnMessage);
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
    emit prpEvent(OST::EvType::de, key, nullptr,  this);

}

void PropertyMulti::push()
{
    if (!this->hasGrid())
    {
        logError("PropertyMulti::Push - no array/grid defined");
        return;
    }
    QMap<QString, ValueBase*> wGridLine;
    foreach(const QString &elt, mElts.keys())
    {
        wGridLine[elt] = ValueFactory::createValue(mElts[elt]);
    }
    mGrid.append(wGridLine);
    if (mGridLimit > 0 && mGrid.size() > this->mGridLimit)
    {
        mGrid.removeFirst();
        emit PropertyBase::prpEvent(OST::EvType::gd, 0, nullptr, this);
    }

    emit PropertyBase::prpEvent(OST::EvType::gc, mGrid.size() - 1, nullptr, this);
}

void PropertyMulti::setAll(const QVariantMap &pValues)
{
    /* Check if data is valid and contains every value */
    foreach(const QString &elt, mElts.keys())
    {
        if ((mElts[elt]->getType() == "int") || (mElts[elt]->getType() == "float") || (mElts[elt]->getType() == "string")
                || (mElts[elt]->getType() == "bool") || (mElts[elt]->getType() == "date") || (mElts[elt]->getType() == "time")
                || (mElts[elt]->getType() == "datetime"))
        {
            if (!pValues.contains(elt))
            {
                logWarning("PropertyMulti::setAll incomplete values, %1 missing ", {elt});
                return;
            }
        }
    }

    foreach(const QString &elt, mElts.keys())
    {
        setElt(elt, pValues[elt], false);

    }
    emit PropertyBase::prpEvent(OST::EvType::ea, QVariant(), nullptr, this);

}
void PropertyMulti::newLine(const QVariantMap &pValues)
{
    if (!this->hasGrid())
    {
        logError("PropertyMulti::newLine - no array/grid defined");
        return;
    }
    /* Check if data is valid and contains every value */
    foreach(const QString &elt, mElts.keys())
    {
        if ((mElts[elt]->getType() == "int") || (mElts[elt]->getType() == "float") || (mElts[elt]->getType() == "string")
                || (mElts[elt]->getType() == "bool") || (mElts[elt]->getType() == "date") || (mElts[elt]->getType() == "time")
                || (mElts[elt]->getType() == "datetime"))

        {
            if (!pValues.contains(elt))
            {
                logWarning("PropertyMulti::newLine incomplete values, %1 missing ", {elt});
                return;
            }
        }
    }

    foreach(const QString &elt, mElts.keys())
    {
        setElt(elt, pValues[elt]);

    }
    push();
}
bool PropertyMulti::updateLine(const int i, const QVariantMap &pValues)
{
    if (!this->hasGrid())
    {
        logError("PropertyMulti::updateLine - no array/grid defined");
        return false;
    }

    if (i >= mGrid.size())
    {
        logWarning("Can't update line" + QString::number(i)  + " >= " + QString::number(mGrid.size()));
        return false;
    }

    /* Check if data is valid and contains every value */
    foreach(const QString &elt, mElts.keys())
    {
        if ((mElts[elt]->getType() == "int") || (mElts[elt]->getType() == "float") || (mElts[elt]->getType() == "string")
                || (mElts[elt]->getType() == "bool") || (mElts[elt]->getType() == "date") || (mElts[elt]->getType() == "time"))
        {
            if (!pValues.contains(elt))
            {
                logError("PropertyMulti::updateLine incomplete values, %1 missing ", {elt});
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

    emit PropertyBase::prpEvent(OST::EvType::gu, i, nullptr, this);

    return true;

}
bool PropertyMulti::deleteLine(const int i)
{
    if (!this->hasGrid())
    {
        logError("PropertyMulti::Push - no array/grid defined");
        return false;
    }
    if (i >= mGrid.size())
    {
        logWarning("Can't delete line" + QString::number(i)  + " >= " + QString::number(mGrid.size()));
        return false;
    }

    mGrid.removeAt(i);
    emit PropertyBase::prpEvent(OST::EvType::gd, i, nullptr, this);
    return true;
}

void PropertyMulti::clearGrid()
{
    if (!this->hasGrid())
    {
        logError("PropertyMulti::Push - no array/grid defined");
        return;
    }

    mGrid.clear();
    emit PropertyBase::prpEvent(OST::EvType::gr, QVariant(), nullptr, this);

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
        case SXY:
            if (!defs.params.contains("S"))
            {
                qDebug() << "SXY graph definition error : param should contain S binding";
                return;
            }
            if (!defs.params.contains("X"))
            {
                qDebug() << "SXY graph definition error : param should contain X binding";
                return;
            }
            if (!defs.params.contains("Y"))
            {
                qDebug() << "SXY graph definition error : param should contain Y binding";
                return;
            }
            mHasGraph = true;
            break;
        case SDY:
            if (!defs.params.contains("S"))
            {
                qDebug() << "SDY graph definition error : param should contain S binding";
                return;
            }
            if (!defs.params.contains("D"))
            {
                qDebug() << "SDY graph definition error : param should contain D binding";
                return;
            }
            if (!defs.params.contains("Y"))
            {
                qDebug() << "SDY graph definition error : param should contain Y binding";
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
        logWarning("Can't swap lines" + QString::number(l1) + "/" + QString::number(l2) + " > " + QString::number(mGrid.size()));
        return false;
    }
    if ((l1 < 0) || (l2 < 0))
    {
        logWarning("Can't swap lines" + QString::number(l1) + "/" + QString::number(l2));
        return false;
    }
    mGrid.swapItemsAt(l2, l1);
    emit PropertyBase::prpEvent(OST::EvType::ap, QVariant(), nullptr, this);
    return true;
}

bool PropertyMulti::fetchLine(int l)
{
    if ((l >= mGrid.size()))
    {
        logWarning("Can't fetch line" + QString::number(l)  + " >= " + QString::number(mGrid.size()));
        return false;
    }
    foreach(const QString &e, mGrid.at(l).keys())
    {
        mGrid.at(l)[e]->updateElement(true);
    }
    //emit PropertyBase::prpEvent(OST::EvType::ap, QVariant(), nullptr, this); //we don't need any dedicated event : setValue is already doing it
    return true;
}
bool PropertyMulti::updateLine(const int i)
{
    if ((i >= mGrid.size()))
    {
        logWarning("Can't update line" + QString::number(i)  + " >= " + QString::number(mGrid.size()));
        return false;
    }
    foreach(const QString &e, mGrid.at(i).keys())
    {
        mGrid.at(i)[e]->updateValue();
    }
    emit PropertyBase::prpEvent(OST::EvType::ap, QVariant(), nullptr, this);
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
void PropertyMulti::OnEltEvent(OST::EvType e, QVariant data, OST::ElementBase* elt)
{
    if (e == OST::EvType::ee) emit PropertyBase::prpEvent(OST::EvType::ea, data, elt, this); /* force all switches send */
    else emit PropertyBase::prpEvent(e, data, elt, this);
}
void PropertyMulti::OnLovChanged(ElementBase*)
{
    //emit propertyEvent(this, {"ap", "", this->key(), "", 0, QVariantMap()});
}
void PropertyMulti::OnMessage(LogLevel l, QString m, QVariantList args)
{
    // Just pass through to PropertyBase methods which will add key() prefix
    QVariantList newArgs = args;
    newArgs.prepend(key());
    QString newMessage = "%1 - " + incrementPlaceholders(m);
    emit logMessage(l, newMessage, newArgs);
}


}
