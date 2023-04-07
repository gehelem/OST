#include <QtCore>
#include "datastore.h"

Datastore::Datastore()
{
    //sendMessage("DataStore init");
}
Datastore::~Datastore()
{
}
bool Datastore::createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel,
                                  const int &pPropertyPermission,
                                  const  QString &pPropertyDevcat, const QString &pPropertyGroup)
{
    if (mProperties.contains(pPropertyName))
    {
        sendWarning("createOstProperty - property " + pPropertyName + " already exists.");
        return false;
    }
    QVariantMap prop;
    prop["propertyLabel"] = pPropertyLabel;
    prop["permission"] = pPropertyPermission;
    prop["devcat"] = pPropertyDevcat;
    prop["group"] = pPropertyGroup;
    prop["name"] = pPropertyName;
    mProperties[pPropertyName] = prop;
    OnModuleEvent("cp", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    return true;
}
void Datastore::emitPropertyCreation(const QString &pPropertyName)
{
    emit OnModuleEvent("cp", QString(), pPropertyName, mProperties[pPropertyName].toMap());
}
bool Datastore::setOstPropertyValue(const QString &pPropertyName, const QVariant &pPropertyValue, bool mEmitEvent)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("setOstPropertyValue - property " + pPropertyName + " not found.");
        return false;
    }
    QVariantMap prop = mProperties[pPropertyName].toMap();
    prop["value"] = pPropertyValue;
    mProperties[pPropertyName] = prop;

    if (prop.contains("value") && mStore.contains(pPropertyName))
    {
        PropertyUpdate d;
        mStore[pPropertyName]->accept(&d, prop);
    }
    getQtProperties();




    if (mEmitEvent) OnModuleEvent("sp", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    return true;
}
QVariant Datastore::getOstPropertyValue(const QString &pPropertyName)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("getPropertyValue - property " + pPropertyName + " not found.");
        return QVariant();
    }
    if (!mProperties[pPropertyName].toMap().contains("value"))
    {
        sendWarning("getPropertyValue - property " + pPropertyName + " contains no value.");
        return QVariant();
    }
    return mProperties[pPropertyName].toMap()["value"];
}

bool Datastore::createOstElement(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                 bool mEmitEvent)
{
    if (!mProperties.contains(pPropertyName) )
    {
        sendWarning(" createElement - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap _prop = mProperties[pPropertyName].toMap();
    QVariantMap _elements = _prop["elements"].toMap();
    if (_elements.contains(pElementName) )
    {
        sendWarning(" createElement - property " + pPropertyName + " : element " + pElementName + "already exists.");
        return false ;
    }
    QVariantMap _element;
    _element["elementLabel"] = pElementLabel;
    _elements[pElementName] = _element;
    _prop["elements"] = _elements;
    mProperties[pPropertyName] = _prop;
    if (mEmitEvent) OnModuleEvent("ce", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    return true;
}
bool Datastore::setOstElementValue(const QString &pPropertyName, const QString &pElementName, const QVariant &pElementValue,
                                   bool mEmitEvent)
{
    if (!mProperties.contains(pPropertyName) )
    {
        sendWarning("setElementValue - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap _prop = mProperties[pPropertyName].toMap();
    if (!_prop.contains("elements"))
    {
        sendWarning("setOstElementValue - property " + pPropertyName + " has no elemnts");
        return false;
    }
    QVariantMap _elements = _prop["elements"].toMap();
    if (!_elements.contains(pElementName) )
    {
        sendWarning("setElementValue - property " + pPropertyName + " : element " + pElementName + " not found.");
        return false;
    }
    QVariantMap element = _elements[pElementName].toMap();
    if (element.contains("value"))
    {
        if (strcmp(element["value"].typeName(), "double") == 0)
        {
            //element["value"]=pElementValue.toDouble();
            element["value"].setValue(pElementValue.toDouble());
        }
        if (strcmp(element["value"].typeName(), "float") == 0)
        {
            //element["value"]=pElementValue.toFloat();
            element["value"].setValue(pElementValue.toFloat());
        }
        if (strcmp(element["value"].typeName(), "qlonglong") == 0)
        {
            //element["value"]=pElementValue.toFloat();
            element["value"].setValue(pElementValue.toDouble());
        }
        if (strcmp(element["value"].typeName(), "int") == 0)
        {
            //element["value"]=pElementValue.toInt();
            element["value"].setValue(pElementValue.toInt());
        }
        if (strcmp(element["value"].typeName(), "QString") == 0)
        {
            element["value"] = pElementValue.toString();
        }
        if (strcmp(element["value"].typeName(), "bool") == 0)
        {
            if (_prop.contains("rule"))
            {
                if (_prop["rule"] == 0) // one of many
                {
                    if (pElementValue.toBool())
                    {
                        for(QVariantMap::const_iterator itelt = _elements.begin(); itelt != _elements.end(); ++itelt)
                        {

                            QVariantMap elt = _elements[itelt.key()].toMap();
                            if (pElementName == itelt.key())
                            {
                                elt["value"] = true;
                                element["value"] = true;
                            }
                            else
                            {
                                if (strcmp(elt["value"].typeName(), "bool") == 0) elt["value"] = false;
                            }

                            _elements[itelt.key()] = elt;
                        }
                    }
                    else
                    {

                    }

                }
                if (_prop["rule"] == 1) // at most one
                {
                    if (pElementValue.toBool())
                    {
                        for(QVariantMap::const_iterator itelt = _elements.begin(); itelt != _elements.end(); ++itelt)
                        {

                            QVariantMap elt = _elements[itelt.key()].toMap();
                            if (pElementName == itelt.key())
                            {
                                elt["value"] = true;
                                element["value"] = true;
                            }
                            else
                            {
                                if (strcmp(elt["value"].typeName(), "bool") == 0) elt["value"] = false;
                            }

                            _elements[itelt.key()] = elt;
                        }
                    }
                    else
                    {
                        element["value"] = false;

                    }

                }
                if (_prop["rule"] == 2) // any
                {
                    element["value"] = pElementValue.toBool();
                }

            }
            else
            {
                element["value"] = pElementValue.toBool(); // we shouldn't do that, should we ?
            }

        }
    }
    else
    {
        element["value"] = pElementValue;
    }
    _elements[pElementName] = element;
    _prop["elements"] = _elements;

    mProperties[pPropertyName] = _prop;
    if (mEmitEvent) OnModuleEvent("se", QString(), pPropertyName, mProperties[pPropertyName].toMap());


    return true; // should return false when request is invalid, we'll see that later
}
bool Datastore::setOstElementGrid(const QString &pPropertyName, const QString &pElementName,
                                  const QVariantList &pElementGrid,
                                  bool mEmitEvent)
{
    if (!mProperties.contains(pPropertyName) )
    {
        sendWarning("setOstElementGrid - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap _prop = mProperties[pPropertyName].toMap();
    if (!_prop.contains("elements"))
    {
        sendWarning("setOstElementGrid - property " + pPropertyName + " has no elemnts");
        return false;
    }
    QVariantMap _elements = _prop["elements"].toMap();
    if (!_elements.contains(pElementName))
    {
        sendWarning("setOstElementGrid - property " + pPropertyName + " : element " + pElementName + " not found.");
        return false;
    }

    QVariantMap element = _elements[pElementName].toMap();
    if (!_prop.contains("grid"))
    {
        sendWarning("setOstElementGrid - property " + pPropertyName + " element " + pElementName + " has no grid.");
        return false;
    }
    element["gridvalues"] = pElementGrid;
    _elements[pElementName] = element;
    _prop["elements"] = _elements;
    mProperties[pPropertyName] = _prop;
    if (mEmitEvent) OnModuleEvent("se", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    return true;
}
QVariantList Datastore::getOstElementGrid(const QString &pPropertyName, const QString &pElementName)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " not found.");
        return QVariantList();
    }
    if (!mProperties[pPropertyName].toMap().contains("elements"))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " contains no elements.");
        return QVariantList();
    }
    if (!mProperties[pPropertyName].toMap().contains("grid"))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " element " + pElementName + " has no grid.");
        return QVariantList();
    }

    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();
    if (!elts.contains(pElementName))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " has no " + pElementName + " element.");
        return QVariantList();
    }
    if (!mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap().contains("gridvalues"))
    {
        return QVariantList();
    }
    return mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap()["gridvalues"].toList();
}

QVariant Datastore::getOstElementValue(const QString &pPropertyName, const QString &pElementName)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("getElementValue - property " + pPropertyName + " not found.");
        return QVariant();
    }
    if (!mProperties[pPropertyName].toMap().contains("elements"))
    {
        sendWarning("getElementValue - property " + pPropertyName + " contains no elements.");
        return QVariant();
    }
    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();
    if (!elts.contains(pElementName))
    {
        sendWarning("getElementValue - property " + pPropertyName + " has no " + pElementName + " element.");
        return QVariant();
    }
    return mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap()["value"];
}
void Datastore::loadOstPropertiesFromFile(const QString &pFileName)
{
    QString content;
    QFile file;
    file.setFileName(pFileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    content = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject props = d.object();
    foreach(const QString &key, props.keys())
    {
        QVariantMap tt = props[key].toVariant().toMap();
        mProperties[key] = tt;
        RootProperty *rp = PropertyFactory::createProperty(tt);
        if (rp != nullptr)
        {
            mStore[key] = PropertyFactory::createProperty(tt);
            mStore[key]->setState(RootValue::State::Ok);
        }
        /*if (tt.contains("type"))
        {
            if (strcmp(tt["value"].typeName(), "QString") == 0)
            {
                TextProperty *bp = new TextProperty(tt["propertyLabel"].toString(),
                                                    tt["devcat"].toString(),
                                                    tt["group"].toString(),
                                                    TextProperty::ReadWrite);
                bp->setValue(tt["value"].toString());
                mStore[key] = bp;

            }
            else
            {
                if (strcmp(tt["value"].typeName(), "qlonglong") == 0)
                {
                    NumberProperty *bp = new NumberProperty(tt["propertyLabel"].toString(),
                                                            tt["devcat"].toString(),
                                                            tt["group"].toString(),
                                                            NumberProperty::ReadWrite);
                    bp->setValue(tt["value"].toDouble());
                    mStore[key] = bp;

                }
                else
                {
                    BasicProperty *bp = new BasicProperty(tt["propertyLabel"].toString(),
                                                          tt["devcat"].toString(),
                                                          tt["group"].toString(),
                                                          BasicProperty::ReadWrite);
                    mStore[key] = bp;

                }

            }
        }*/

        //qDebug() << bp->property("label").toString();


    }

    //QByteArray docByteArray = d.toJson(QJsonDocument::Compact);
    //QString strJson = QLatin1String(docByteArray);
    //qDebug() << "loadPropertiesFromFile  - " << strJson;

}

void Datastore::saveOstPropertiesToFile(const QString &pFileName)
{
    QJsonObject obj = QJsonObject::fromVariantMap(mProperties);
    QJsonDocument doc(obj);

    QFile jsonFile(pFileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(doc.toJson());
    jsonFile.close();
}
void Datastore::setOstPropertyAttribute   (const QString &pPropertyName, const QString &pAttributeName,
        const QVariant &AttributeValue,
        bool mEmitEvent)
{
    //sendMessage("setOstPropertyAttribute  - " + pPropertyName);
    QVariantMap _prop = mProperties[pPropertyName].toMap();
    _prop[pAttributeName] = AttributeValue;
    mProperties[pPropertyName] = _prop;
    if (mEmitEvent) OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

}
bool Datastore::setOstElementAttribute(const QString &pPropertyName, const QString &pElementName,
                                       const  QString &pAttributeName,
                                       const QVariant &AttributeValue,
                                       bool mEmitEvent)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("setOstElementAttribute - property " + pPropertyName + " not found.");
        return false;
    }
    QVariantMap prop = mProperties[pPropertyName].toMap();
    if (!prop.contains("elements"))
    {
        sendWarning("setOstElementAttribute - property " + pPropertyName + " contains no elements.");
        return false;
    }
    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();
    if (!elts.contains(pElementName))
    {
        sendWarning("setOstElementAttribute - property " + pPropertyName + " has no " + pElementName + " element.");
        return false;
    }

    QVariantMap elt = elts[pElementName].toMap();
    elts[pAttributeName] = AttributeValue;
    elts[pElementName] = elt;
    prop["elements"] = elts;
    mProperties[pPropertyName] = prop;
    if (mEmitEvent) OnModuleEvent("ae", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    return true;


}
QVariantMap Datastore::getProfile(void)
{
    QVariantMap _res;

    foreach(const QString &keyprop, mProperties.keys())
    {
        if (mProperties[keyprop].toMap().contains("hasprofile"))
        {
            QVariantMap property;
            if (mProperties[keyprop].toMap().contains("value"))
            {
                property["value"] = mProperties[keyprop].toMap()["value"];
            }
            if (mProperties[keyprop].toMap().contains("elements"))
            {
                QVariantMap element, elements;
                foreach(const QString &keyelt, mProperties[keyprop].toMap()["elements"].toMap().keys())
                {
                    if (mProperties[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("value"))
                    {
                        element["value"] = mProperties[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                    }
                    if (mProperties[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("gridvalues"))
                    {
                        element["gridvalues"] = mProperties[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["gridvalues"];
                    }
                    elements[keyelt] = element;
                }
                property["elements"] = elements;
            }
            _res[keyprop] = property;
        }
    }

    return _res;
}

void Datastore::deleteOstProperty(const QString &pPropertyName)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("Can't delete inexistant property " + pPropertyName);
        return;
    }
    mProperties.remove(pPropertyName);
    emit OnModuleEvent("delprop", QString(), pPropertyName, QVariantMap());

}

bool Datastore::pushOstElements         (const QString &pPropertyName)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("pushOstElements - property " + pPropertyName + " not found.");
        return false;
    }
    QVariantMap prop = mProperties[pPropertyName].toMap();
    if (!prop.contains("elements"))
    {
        sendWarning("pushOstElements - property " + pPropertyName + " contains no elements.");
        return false;
    }
    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();

    if (!prop.contains("grid") )
    {
        sendWarning("pushOstElements : No grid defined for property  " + pPropertyName);
        return false;
    }
    for(QVariantMap::const_iterator elt = elts.begin(); elt != elts.end(); ++elt)
    {
        QVariant _myVal = elts[elt.key()].toMap()["value"];
        elts[elt.key()] = _myVal;
    }
    prop["elements"] = elts;
    QVariantMap props;
    props[pPropertyName] = prop;
    newOstPropertyLine(pPropertyName, props);
    return true;


}
bool Datastore::resetOstElements      (const QString &pPropertyName)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("resetOstElements - property " + pPropertyName + " not found.");
        return false;
    }
    QVariantMap prop = mProperties[pPropertyName].toMap();
    if (!prop.contains("elements"))
    {
        sendWarning("resetOstElements - property " + pPropertyName + " contains no elements.");
        return false;
    }
    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();

    if (!prop.contains("grid") )
    {
        sendWarning("resetOstElements : No grid defined for property  " + pPropertyName);
        return false;
    }

    prop["grid"].clear();
    for(QVariantMap::const_iterator elt = elts.begin(); elt != elts.end(); ++elt)
    {
        QVariantMap myelt = elts[elt.key()].toMap();
        myelt["gridvalues"] = QVariantList();
        elts[elt.key()] = myelt;
    }
    prop["elements"] = elts;

    mProperties[pPropertyName] = prop;
    emit OnModuleEvent("resetvalues", QString(), pPropertyName, QVariantMap());
    return true;

}
bool Datastore::newOstPropertyLine(const QString &pPropertyName, const QVariantMap &pElementsValues)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("newOstPropertyLine - property " + pPropertyName + " not found.");
        return false;
    }
    QVariantMap prop = mProperties[pPropertyName].toMap();
    if (!prop.contains("elements"))
    {
        sendWarning("newOstPropertyLine - property " + pPropertyName + " contains no elements.");
        return false;
    }
    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();

    if (!prop.contains("grid") )
    {
        sendWarning("newOstPropertyLine : No grid defined for property  " + pPropertyName);
        return false;
    }
    QVariantList arr = prop["grid"].toList();
    QVariantList cols;
    for(QVariantMap::const_iterator elt = elts.begin(); elt != elts.end(); ++elt)
    {
        cols.push_back(pElementsValues[pPropertyName].toMap()["elements"].toMap()[elt.key()]);
    }
    arr.push_back(cols);
    prop["grid"] = arr;
    QVariantMap elementsEmpty;
    long int gridsize = 0;
    for(QVariantMap::const_iterator elt = elts.begin(); elt != elts.end(); ++elt)
    {
        //qDebug() << _elt.key() << _elt.value();
        QVariantMap myelt = elts[elt.key()].toMap();
        QVariantMap myEmptyElt = elts[elt.key()].toMap();
        QVariantList mylist = myelt["gridvalues"].toList();
        gridsize = mylist.count();
        QVariantList myEmptyList;
        QVariant _myVal;
        if (strcmp(myelt["value"].typeName(), "double") == 0)
        {
            _myVal = pElementsValues[pPropertyName].toMap()["elements"].toMap()[elt.key()].toDouble();
        }
        if (strcmp(myelt["value"].typeName(), "float") == 0)
        {
            _myVal = pElementsValues[pPropertyName].toMap()["elements"].toMap()[elt.key()].toFloat();
        }
        if (strcmp(myelt["value"].typeName(), "qlonglong") == 0)
        {
            _myVal = pElementsValues[pPropertyName].toMap()["elements"].toMap()[elt.key()].toLongLong();
        }
        if (strcmp(myelt["value"].typeName(), "int") == 0)
        {
            _myVal = pElementsValues[pPropertyName].toMap()["elements"].toMap()[elt.key()].toInt();
        }
        if (strcmp(myelt["value"].typeName(), "QString") == 0)
        {
            _myVal = pElementsValues[pPropertyName].toMap()["elements"].toMap()[elt.key()].toString();
        }

        mylist.push_back(_myVal);
        myEmptyList.push_back(_myVal);
        myelt["gridvalues"] = mylist;
        myEmptyElt["gridvalues"] = myEmptyList;
        elts[elt.key()] = myelt;
        elementsEmpty[elt.key()] = myEmptyElt;
    }
    prop["elements"] = elts;
    mProperties[pPropertyName] = prop;
    emit OnModuleEvent("pushvalues", QString(), pPropertyName, elementsEmpty);

    long int gridlimit = 200;

    if (mProperties[pPropertyName].toMap().contains("gridlimit"))
    {
        gridlimit = mProperties[pPropertyName].toMap()["gridlimit"].toLongLong();
    }
    if (gridsize >= gridlimit )
    {
        deleteOstPropertyLine(pPropertyName, 0);
    }
    return true;
}
bool Datastore::deleteOstPropertyLine(const QString &pPropertyName, const double &pLine)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("deleteOstPropertyLine - property " + pPropertyName + " not found.");
        return false;
    }
    QVariantMap prop = mProperties[pPropertyName].toMap();
    if (!prop.contains("elements"))
    {
        sendWarning("deleteOstPropertyLine - property " + pPropertyName + " contains no elements.");
        return false;
    }
    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();

    if (!prop.contains("grid") )
    {
        sendWarning("deleteOstPropertyLine : No grid defined for property  " + pPropertyName);
        return false;
    }

    QVariantList arr = prop["grid"].toList();
    arr.removeAt(pLine);
    prop["grid"] = arr;
    for(QVariantMap::const_iterator elt = elts.begin(); elt != elts.end(); ++elt)
    {
        QVariantMap myelt = elts[elt.key()].toMap();
        QVariantList mylist = myelt["gridvalues"].toList();
        mylist.removeAt(pLine);
        myelt["gridvalues"] = mylist;
        elts[elt.key()] = myelt;
    }
    prop["elements"] = elts;
    mProperties[pPropertyName] = prop;
    OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    return true;
}
bool Datastore::updateOstPropertyLine(const QString &pPropertyName, const double &pLine,
                                      const QVariantMap &pElementsValues)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap _prop = mProperties[pPropertyName].toMap();
        if (!_prop.contains("grid") )
        {
            sendWarning("updateOstPropertyLine : No grid defined for property  " + pPropertyName);
            return false;
        }
        else
        {
            //qDebug() << "updateOstPropertyLine :" << pPropertyName << ">> " << pPropertyName;
            QVariantMap _elements = _prop["elements"].toMap();
            QVariantList _arr = _prop["grid"].toList();
            _arr.removeAt(pLine);
            _prop["grid"] = _arr;
            for(QVariantMap::const_iterator _elt = _elements.begin(); _elt != _elements.end(); ++_elt)
            {
                //qDebug() << _elt.key() << _elt.value();
                QVariantMap _myelt = _elements[_elt.key()].toMap();
                QVariantList _mylist = _myelt["gridvalues"].toList();
                QVariant _myVal;
                if (strcmp(_myelt["value"].typeName(), "double") == 0)
                {
                    _myVal = pElementsValues[pPropertyName].toMap()["elements"].toMap()[_elt.key()].toDouble();
                }
                if (strcmp(_myelt["value"].typeName(), "float") == 0)
                {
                    _myVal = pElementsValues[pPropertyName].toMap()["elements"].toMap()[_elt.key()].toFloat();
                }
                if (strcmp(_myelt["value"].typeName(), "qlonglong") == 0)
                {
                    _myVal = pElementsValues[pPropertyName].toMap()["elements"].toMap()[_elt.key()].toLongLong();
                }
                if (strcmp(_myelt["value"].typeName(), "int") == 0)
                {
                    _myVal = pElementsValues[pPropertyName].toMap()["elements"].toMap()[_elt.key()].toInt();
                }
                if (strcmp(_myelt["value"].typeName(), "QString") == 0)
                {
                    _myVal = pElementsValues[pPropertyName].toMap()["elements"].toMap()[_elt.key()].toString();
                }


                _mylist[pLine] = _myVal;
                _myelt["gridvalues"] = _mylist;
                _elements[_elt.key()] = _myelt;
            }
            _prop["elements"] = _elements;
            mProperties[pPropertyName] = _prop;
            OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
            return true;
        }
    }
    else
    {
        sendWarning("updateOstPropertyLine : Can't add line to inexistant property " + pPropertyName);
        return false;
    }

}
QVariant Datastore::getOstElementLineValue(const QString &pPropertyName, const QString &pElementName,
        const double &pLine)
{
    if (getOstElementGrid(pPropertyName, pElementName).isEmpty())
    {
        sendWarning("Try to read an empty grid (" + pPropertyName + "/" + pElementName + ")");
        return QVariant();

    }
    else
    {
        if (pLine >= getOstElementGrid(pPropertyName, pElementName).count())
        {
            sendWarning("Try to access inexistant line grid (" + pPropertyName + "/" + pElementName + ") size=" + getOstElementGrid(
                            pPropertyName, pElementName).count() + " vs requested=" + pLine);
            return QVariant();
        }
        else
        {
            return getOstElementGrid(pPropertyName, pElementName)[pLine];
        }
    }
}
bool Datastore::setOstElementLineValue (const QString &pPropertyName, const QString &pElementName, const double &pLine,
                                        const QVariant &pElementValue)
{
    QVariantList elementList  = getOstElementGrid(pPropertyName, pElementName);
    if (pLine >= elementList.count())
    {
        sendWarning("invalid line (" + pPropertyName + "/" + pElementName + ") size=" + QString::number(
                        elementList.count()) + " vs requested(-1)=" +
                    QString::number(pLine));
        return false;
    }
    else
    {
        elementList[pLine] = pElementValue;
        setOstElementGrid(pPropertyName, pElementName, elementList, true);
        OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
        return true;
    }


}
void Datastore::clearOstElementLov(const QString &pPropertyName, const QString &pElementName)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("elements"))
        {
            QVariantMap mElts = mProp["elements"].toMap();
            if (mElts.contains(pElementName))
            {
                QVariantMap mElt =  mElts[pElementName].toMap();
                if (mElt.contains("listOfValues"))
                {
                    mElt["listOfValues"] = QVariantMap();
                    mElts[pElementName] = mElt;
                    mProp["elements"] = mElts;
                    mProperties[pPropertyName] = mProp;
                    OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
                }
                else
                {
                    sendWarning("clearOstElementLov - property " + pPropertyName + " element " + pElementName + " has no list of values.");
                }
            }
            else
            {
                sendWarning("clearOstElementLov - property " + pPropertyName + " has no " + pElementName + " element.");
                return;
            }
        }
        else
        {
            sendWarning("clearOstElementLov - property " + pPropertyName + " contains no elements.");
            return;
        }
    }
    else
    {
        sendWarning("clearOstElementLov - property " + pPropertyName + " not found.");
        return;
    }

}
void Datastore::addOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode,
                                 const QString &pLovLabel)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("elements"))
        {
            QVariantMap mElts = mProperties[pPropertyName].toMap()["elements"].toMap();
            if (mElts.contains(pElementName))
            {
                QVariantMap mElt =  mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap();
                if (mElt.contains("listOfValues"))
                {
                    QVariantMap mLov = mElt["listOfValues"].toMap();
                    if (mLov.contains(pLovCode))
                    {
                        sendWarning("addOstElementLov - property " + pPropertyName + " element " + pElementName + " list of values member " +
                                    pLovCode +
                                    " already exists");
                    }
                    else
                    {
                        mLov[pLovCode] = pLovLabel;
                        mElt["listOfValues"] = mLov;
                        mElts[pElementName] = mElt;
                        mProp["elements"] = mElts;
                        mProperties[pPropertyName] = mProp;
                        OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

                    }
                }
                else
                {
                    sendWarning("addOstElementLov - property " + pPropertyName + " element " + pElementName + " has no list of values.");
                }
            }
            else
            {
                sendWarning("addOstElementLov - property " + pPropertyName + " has no " + pElementName + " element.");
            }
        }
        else
        {
            sendWarning("addOstElementLov - property " + pPropertyName + " contains no elements.");
        }
    }
    else
    {
        sendWarning("addOstElementLov - property " + pPropertyName + " not found.");
    }

}
QVariant Datastore::getOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("getOstElementLov - property " + pPropertyName + " not found.");
        return QVariant();
    }
    if (!mProperties[pPropertyName].toMap().contains("elements"))
    {
        sendWarning("getOstElementLov - property " + pPropertyName + " contains no elements.");
        return QVariant();
    }
    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();
    if (!elts.contains(pElementName))
    {
        sendWarning("getOstElementLov - property " + pPropertyName + " has no " + pElementName + " element.");
        return QVariant();
    }
    QVariantMap elt =  mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap();
    if (!elt.contains("listOfValues"))
    {
        sendWarning("getOstElementLov - property " + pPropertyName + " element " + pElementName + " has no list of values.");
        return QVariant();
    }
    if (!elt["listOfValues"].toMap().contains(pLovCode))
    {
        sendWarning("getOstElementLov - property " + pPropertyName + " element " + pElementName + " has no " + pLovCode +
                    " member in list of values.");
        return QVariant();
    }

    return elt["listOfValues"].toMap()[pLovCode];

}
void Datastore::setOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode,
                                 const QString &pLovLabel)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("elements"))
        {
            QVariantMap mElts = mProperties[pPropertyName].toMap()["elements"].toMap();
            if (mElts.contains(pElementName))
            {
                QVariantMap mElt =  mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap();
                if (mElt.contains("listOfValues"))
                {
                    QVariantMap mLov = mElt["listOfValues"].toMap();
                    if (mLov.contains(pLovCode))
                    {
                        mLov[pLovCode] = pLovLabel;
                        mElt["listOfValues"] = mLov;
                        mElts[pElementName] = mElt;
                        mProp["elements"] = mElts;
                        mProperties[pPropertyName] = mProp;
                        OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
                    }
                    else
                    {
                        sendWarning("setOstElementLov - property " + pPropertyName + " element " + pElementName + " has no " + pLovCode +
                                    " member in list of values.");
                    }
                }
                else
                {
                    sendWarning("setOstElementLov - property " + pPropertyName + " element " + pElementName + " has no list of values.");
                }
            }
            else
            {
                sendWarning("setOstElementLov - property " + pPropertyName + " has no " + pElementName + " element.");
            }
        }
        else
        {
            sendWarning("setOstElementLov - property " + pPropertyName + " contains no elements.");
        }
    }
    else
    {
        sendWarning("setOstElementLov - property " + pPropertyName + " not found.");
    }
}
void Datastore::deleteOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("elements"))
        {
            QVariantMap mElts = mProperties[pPropertyName].toMap()["elements"].toMap();
            if (mElts.contains(pElementName))
            {
                QVariantMap mElt =  mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap();
                if (mElt.contains("listOfValues"))
                {
                    QVariantMap mLov = mElt["listOfValues"].toMap();
                    if (mLov.contains(pLovCode))
                    {
                        mLov[pLovCode].clear();
                        mElt["listOfValues"] = mLov;
                        mElts[pElementName] = mElt;
                        mProp["elements"] = mElts;
                        mProperties[pPropertyName] = mProp;
                        OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

                    }
                    else
                    {
                        sendWarning("deleteOstElementLov - property " + pPropertyName + " element " + pElementName + " has no " + pLovCode +
                                    " member in list of values.");
                    }
                }
                else
                {
                    sendWarning("deleteOstElementLov - property " + pPropertyName + " element " + pElementName + " has no list of values.");
                }
            }
            else
            {
                sendWarning("deleteOstElementLov - property " + pPropertyName + " has no " + pElementName + " element.");
            }
        }
        else
        {
            sendWarning("deleteOstElementLov - property " + pPropertyName + " contains no elements.");
        }
    }
    else
    {
        sendWarning("deleteOstElementLov - property " + pPropertyName + " not found.");
    }

}
void Datastore::clearOstLov(const QString &pPropertyName)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("listOfValues"))
        {
            QVariantMap mLov = mProp["listOfValues"].toMap();
            mProp["listOfValues"] = QVariantMap();
            mProperties[pPropertyName] = mProp;
            OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
        }
        else
        {
            sendWarning("clearOstLov - property " + pPropertyName + " has no list of values.");
            return;
        }
    }
    else
    {
        sendWarning("clearOstLov - property " + pPropertyName + " not found.");
        return;
    }
}
void Datastore::addOstLov(const QString &pPropertyName, const QString &pLovCode,
                          const QString &pLovLabel)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("listOfValues"))
        {
            QVariantMap mLov = mProp["listOfValues"].toMap();
            if (mLov.contains(pLovCode))
            {
                sendWarning("addOstLov - property " + pPropertyName + " list of values member " + pLovCode +
                            " already exists");
                return;

            }
            else
            {
                mLov[pLovCode] = pLovLabel;
                mProp["listOfValues"] = mLov;
                mProperties[pPropertyName] = mProp;
                OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

            }
        }
        else
        {
            sendWarning("addOstLov - property " + pPropertyName + " has no list of values.");
            return;
        }
    }
    else
    {
        sendWarning("addOstLov - property " + pPropertyName + " not found.");
        return;
    }
    return;
}
QVariant Datastore::getOstLov(const QString &pPropertyName, const QString &pLovCode)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("listOfValues"))
        {
            QVariantMap mLov = mProp["listOfValues"].toMap();
            if (!mLov.contains(pLovCode))
            {
                sendWarning("getOstLov - property " + pPropertyName + " has no " + pLovCode +
                            " member in list of values.");

                return QVariant();
            }
            else
            {
                return mLov[pLovCode];
            }
        }
        else
        {
            sendWarning("getOstLov - property " + pPropertyName + " has no list of values.");
            return QVariant();
        }
    }
    else
    {
        sendWarning("getOstLov - property " + pPropertyName + " not found.");
        return QVariant();
    }
    return QVariant();
}
void Datastore::setOstLov(const QString &pPropertyName, const QString &pLovCode,
                          const QString &pLovLabel)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("listOfValues"))
        {
            QVariantMap mLov = mProp["listOfValues"].toMap();
            if (!mLov.contains(pLovCode))
            {
                sendWarning("setOstLov - property " + pPropertyName + " list of values member " + pLovCode +
                            " does not exists");
                return;

            }
            else
            {
                mLov[pLovCode] = pLovLabel;
                mProp["listOfValues"] = mLov;
                mProperties[pPropertyName] = mProp;
                OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

            }
        }
        else
        {
            sendWarning("setOstLov - property " + pPropertyName + " has no list of values.");
            return;
        }
    }
    else
    {
        sendWarning("setOstLov - property " + pPropertyName + " not found.");
        return;
    }
    return;

}
void Datastore::deleteOstLov(const QString &pPropertyName, const QString &pLovCode)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("listOfValues"))
        {
            QVariantMap mLov = mProp["listOfValues"].toMap();
            if (!mLov.contains(pLovCode))
            {
                sendWarning("deleteOstLov - property " + pPropertyName + " list of values member " + pLovCode +
                            " does not exists");
                return;

            }
            else
            {
                mLov[pLovCode].clear();
                mProp["listOfValues"] = mLov;
                mProperties[pPropertyName] = mProp;
                OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

            }
        }
        else
        {
            sendWarning("deleteOstLov - property " + pPropertyName + " has no list of values.");
            return;
        }
    }
    else
    {
        sendWarning("deleteOstLov - property " + pPropertyName + " not found.");
        return;
    }
    return;

}
