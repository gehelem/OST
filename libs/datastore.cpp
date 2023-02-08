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
                    element["value"] = pElementValue.toBool();
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
        mProperties[key] = props[key].toVariant();
    }

    //QByteArray docByteArray = d.toJson(QJsonDocument::Compact);
    //QString strJson = QLatin1String(docByteArray);
    //BOOST_LOG_TRIVIAL(debug) << "loadPropertiesFromFile  - " << mModulename.toStdString() << " - filename=" << fileName.toStdString() << " - " << strJson.toStdString();

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
    //BOOST_LOG_TRIVIAL(debug) << "setOstPropertyAttribute  - " << mModulename.toStdString() << "-" << pPropertyName.toStdString();
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
                        elements[keyelt] = element;
                    }
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
            sendMessage("No grid defined for property  " + pPropertyName);
            return false;
        }
        else
        {
            QVariantList _arr = _prop["grid"].toList();
            QVariantList _cols;
            foreach(auto _elt, _prop["elements"].toMap())
            {
                _cols.push_back(_elt.toMap()["value"]);
            }
            _arr.push_back(_cols);
            _prop["grid"] = _arr;
            mProperties[pPropertyName] = _prop;
            QVariantMap _mess;
            _mess["values"] = _cols;
            emit OnModuleEvent("pushvalues", QString(), pPropertyName, _mess);
            return true;

        }
    }
    else
    {
        sendMessage("Can't push inexistant property " + pPropertyName);
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
