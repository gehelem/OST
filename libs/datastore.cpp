#include <QtCore>
#include "datastore.h"

Datastore::Datastore()
{
    sendMessage("DataStore init");
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
        sendMessage("createOstProperty - property " + pPropertyName + " already exists.");
        return false;
    }
    else
    {
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
}
void Datastore::emitPropertyCreation(const QString &pPropertyName)
{
    emit OnModuleEvent("cp", QString(), pPropertyName, mProperties[pPropertyName].toMap());
}
bool Datastore::setOstPropertyValue(const QString &pPropertyName, const QVariant &pPropertyValue, bool mEmitEvent)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap prop = mProperties[pPropertyName].toMap();
        prop["value"] = pPropertyValue;
        mProperties[pPropertyName] = prop;
        if (mEmitEvent) OnModuleEvent("sp", QString(), pPropertyName, mProperties[pPropertyName].toMap());
        return true;
    }
    else
    {
        return false;
    }
}
QVariant Datastore::getOstPropertyValue(QString &pPropertyName)
{
    if (mProperties.contains(pPropertyName))
    {
        if (mProperties[pPropertyName].toMap().contains("value"))
        {
            return mProperties[pPropertyName].toMap()["value"];
        }
        else
        {
            sendMessage("getPropertyValue - property " + pPropertyName + " contains no value.");
            return QVariant();
        }
    }
    else
    {
        sendMessage("getPropertyValue - property " + pPropertyName + " not found.");
        return QVariant();
    }
}

bool Datastore::createOstElement(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                 bool mEmitEvent)
{
    if (!mProperties.contains(pPropertyName) )
    {
        sendMessage(" createElement - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap _prop = mProperties[pPropertyName].toMap();
    QVariantMap _elements = _prop["elements"].toMap();
    if (_elements.contains(pElementName) )
    {
        sendMessage(" createElement - property " + pPropertyName + " : element " + pElementName + "already exists.");
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
        sendMessage("setElementValue - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap _prop = mProperties[pPropertyName].toMap();
    if (_prop.contains("elements"))
    {
        if (_prop["elements"].toMap().contains(pElementName))
        {
            QVariantMap _elements = _prop["elements"].toMap();
            if (!_elements.contains(pElementName) )
            {
                sendMessage("setElementValue - property " + pPropertyName + " : element " + pElementName + " not found.");
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
        }
    }
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
    if (_prop.contains("elements"))
    {
        if (_prop["elements"].toMap().contains(pElementName))
        {
            QVariantMap _elements = _prop["elements"].toMap();
            if (!_elements.contains(pElementName) )
            {
                sendWarning("setOstElementGrid - property " + pPropertyName + " : element " + pElementName + " not found.");
                return false;
            }

            QVariantMap element = _elements[pElementName].toMap();
            if (_prop.contains("grid"))
            {
                element["gridvalues"] = pElementGrid;
            }
            else
            {
                sendWarning("setOstElementGrid - property " + pPropertyName + " element " + pElementName + " has no grid.");
            }
            _elements[pElementName] = element;
            _prop["elements"] = _elements;
        }
    }
    mProperties[pPropertyName] = _prop;
    if (mEmitEvent) OnModuleEvent("se", QString(), pPropertyName, mProperties[pPropertyName].toMap());


    return true; // should return false when request is invalid, we'll see that later
}
QVariantList Datastore::getOstElementGrid(const QString &pPropertyName, const QString &pElementName)
{
    if (mProperties.contains(pPropertyName))
    {
        if (mProperties[pPropertyName].toMap().contains("elements"))
        {
            QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();
            if (elts.contains(pElementName))
            {
                if (mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap().contains("gridvalues"))
                {
                    return mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap()["gridvalues"].toList();
                }
                else
                {
                    sendWarning("setOstElementGrid - property " + pPropertyName + " element " + pElementName + " has no grid.");
                    return QVariantList();
                }
            }
            else
            {
                sendWarning("setOstElementGrid - property " + pPropertyName + " has no " + pElementName + " element.");
                return QVariantList();
            }
        }
        else
        {
            sendWarning("setOstElementGrid - property " + pPropertyName + " contains no elements.");
            return QVariantList();
        }
    }
    else
    {
        sendWarning("setOstElementGrid - property " + pPropertyName + " not found.");
        return QVariantList();
    }

}

QVariant Datastore::getOstElementValue(const QString &pPropertyName, const QString &pElementName)
{
    if (mProperties.contains(pPropertyName))
    {
        if (mProperties[pPropertyName].toMap().contains("elements"))
        {
            QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();
            if (elts.contains(pElementName))
            {
                return mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap()["value"];
            }
            else
            {
                sendMessage("getElementValue - property " + pPropertyName + " has no " + pElementName + " element.");
                return QVariant();
            }
        }
        else
        {
            sendMessage("getElementValue - property " + pPropertyName + " contains no elements.");
            return QVariant();
        }
    }
    else
    {
        sendMessage("getElementValue - property " + pPropertyName + " not found.");
        return QVariant();
    }


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
        QVariant tt = props[key].toVariant();
        mProperties[key] = tt.toMap();
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
    QVariantMap _prop = mProperties[pPropertyName].toMap();
    if (_prop.contains("elements"))
    {
        if (_prop["elements"].toMap().contains(pElementName))
        {
            QVariantMap elements = _prop["elements"].toMap();
            QVariantMap element = elements[pElementName].toMap();
            element[pAttributeName] = AttributeValue;
            elements[pElementName] = element;
            _prop["elements"] = elements;
        }
    }
    mProperties[pPropertyName] = _prop;
    if (mEmitEvent) OnModuleEvent("ae", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    return true; // should return false when request is invalid, we'll see that later


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
    if (mProperties.contains(pPropertyName))
    {
        mProperties.remove(pPropertyName);
        emit OnModuleEvent("delprop", QString(), pPropertyName, QVariantMap());
    }
    else
    {
        sendMessage("Can't delete inexistant property " + pPropertyName);
    }

}

bool Datastore::pushOstElements         (const QString &pPropertyName)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap _prop = mProperties[pPropertyName].toMap();
        if (!_prop.contains("grid") )
        {
            sendMessage("pushOstElements : No grid defined for property  " + pPropertyName);
            return false;
        }
        else
        {
            QVariantMap _elements = _prop["elements"].toMap();
            for(QVariantMap::const_iterator _elt = _elements.begin(); _elt != _elements.end(); ++_elt)
            {
                QVariant _myVal = _elements[_elt.key()].toMap()["value"];
                _elements[_elt.key()] = _myVal;
            }
            _prop["elements"] = _elements;
            QVariantMap _props;
            _props[pPropertyName] = _prop;
            newOstPropertyLine(pPropertyName, _props);
            return true;

        }
    }
    else
    {
        sendMessage("pushOstElements : Can't add line to inexistant property " + pPropertyName);
        return false;
    }

}
bool Datastore::resetOstElements      (const QString &pPropertyName)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap _prop = mProperties[pPropertyName].toMap();
        if (!_prop.contains("grid") )
        {
            sendMessage("No grid defined for property  " + pPropertyName);
            return false;
        }
        else
        {
            _prop["grid"].clear();
            QVariantMap _elements = _prop["elements"].toMap();
            for(QVariantMap::const_iterator _elt = _elements.begin(); _elt != _elements.end(); ++_elt)
            {
                QVariantMap _myelt = _elements[_elt.key()].toMap();
                _myelt["gridvalues"] = QVariantList();
                _elements[_elt.key()] = _myelt;
            }
            _prop["elements"] = _elements;

            mProperties[pPropertyName] = _prop;
            emit OnModuleEvent("resetvalues", QString(), pPropertyName, QVariantMap());
            return true;

        }
    }
    else
    {
        sendMessage("Can't reset inexistant property " + pPropertyName);
        return false;
    }

}
bool Datastore::newOstPropertyLine(const QString &pPropertyName, const QVariantMap &pElementsValues)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap _prop = mProperties[pPropertyName].toMap();
        if (!_prop.contains("grid") )
        {
            sendMessage("newOstPropertyLine : No grid defined for property  " + pPropertyName);
            return false;
        }
        else
        {
            //qDebug() << "newOstPropertyLine :" << pPropertyName << ">> " << pElementsValues;
            QVariantList _arr = _prop["grid"].toList();
            QVariantList _cols;
            QVariantMap _elements = _prop["elements"].toMap();
            for(QVariantMap::const_iterator _elt = _elements.begin(); _elt != _elements.end(); ++_elt)
            {
                _cols.push_back(pElementsValues[pPropertyName].toMap()["elements"].toMap()[_elt.key()]);
            }
            _arr.push_back(_cols);
            _prop["grid"] = _arr;
            QVariantMap _elementsEmpty;
            long int gridsize = 0;
            for(QVariantMap::const_iterator _elt = _elements.begin(); _elt != _elements.end(); ++_elt)
            {
                //qDebug() << _elt.key() << _elt.value();
                QVariantMap _myelt = _elements[_elt.key()].toMap();
                QVariantMap _myEmptyElt = _elements[_elt.key()].toMap();
                QVariantList _mylist = _myelt["gridvalues"].toList();
                gridsize = _mylist.count();
                QVariantList _myEmptyList;
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

                _mylist.push_back(_myVal);
                _myEmptyList.push_back(_myVal);
                _myelt["gridvalues"] = _mylist;
                _myEmptyElt["gridvalues"] = _myEmptyList;
                _elements[_elt.key()] = _myelt;
                _elementsEmpty[_elt.key()] = _myEmptyElt;
            }
            _prop["elements"] = _elements;
            mProperties[pPropertyName] = _prop;
            emit OnModuleEvent("pushvalues", QString(), pPropertyName, _elementsEmpty);

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
    }
    else
    {
        sendMessage("newOstPropertyLine : Can't add line to inexistant property " + pPropertyName);
        return false;
    }

}
bool Datastore::deleteOstPropertyLine(const QString &pPropertyName, const double &pLine)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap _prop = mProperties[pPropertyName].toMap();
        if (!_prop.contains("grid") )
        {
            sendMessage("deleteOstPropertyLine : No grid defined for property  " + pPropertyName);
            return false;
        }
        else
        {
            qDebug() << "deleteOstPropertyLine :" << pPropertyName << ">> " << pPropertyName;
            QVariantMap _elements = _prop["elements"].toMap();
            QVariantList _arr = _prop["grid"].toList();
            _arr.removeAt(pLine);
            _prop["grid"] = _arr;
            for(QVariantMap::const_iterator _elt = _elements.begin(); _elt != _elements.end(); ++_elt)
            {
                //qDebug() << _elt.key() << _elt.value();
                QVariantMap _myelt = _elements[_elt.key()].toMap();
                QVariantList _mylist = _myelt["gridvalues"].toList();
                _mylist.removeAt(pLine);
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
        sendMessage("deleteOstPropertyLine : Can't add line to inexistant property " + pPropertyName);
        return false;
    }

}
bool Datastore::updateOstPropertyLine(const QString &pPropertyName, const double &pLine, const QVariantMap &pElementsValues)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap _prop = mProperties[pPropertyName].toMap();
        if (!_prop.contains("grid") )
        {
            sendMessage("updateOstPropertyLine : No grid defined for property  " + pPropertyName);
            return false;
        }
        else
        {
            qDebug() << "updateOstPropertyLine :" << pPropertyName << ">> " << pPropertyName;
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
        sendMessage("updateOstPropertyLine : Can't add line to inexistant property " + pPropertyName);
        return false;
    }

}
