#include <QtCore>
#include <basedevice.h>
#include "basemodule.h"

Basemodule::Basemodule(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : _modulename(name),
      _modulelabel(label),
      _availableModuleLibs(availableModuleLibs)
{
    Q_INIT_RESOURCE(basemodule);
    _moduletype = "basemodule";
    loadPropertiesFromFile(":basemodule.json");
    setOstProperty("moduleLabel", label, false);
    setOstProperty("moduleType", "basemodule", false);

}
Basemodule::~Basemodule()
{
    foreach(const QString &key, _ostproperties.keys())
    {
        deleteOstProperty(key);
    }
    Q_CLEANUP_RESOURCE(basemodule);
}
void Basemodule::sendMessage(QString message)
{
    QString mess = QDateTime::currentDateTime().toString("[yyyyMMdd hh:mm:ss.zzz]") + " - " + _modulename + " - " + message;
    qDebug() << mess;
    setOstProperty("message", mess, true);
}

bool Basemodule::createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel,
                                   const int &pPropertyPermission, const  QString &pPropertyDevcat, const QString &pPropertyGroup, QString &err)
{
    //BOOST_LOG_TRIVIAL(debug) << "createOstProperty  - " << _modulename.toStdString() << "-" << pPropertyName.toStdString();
    if (_ostproperties.contains(pPropertyName))
    {
        err = _modulename + " - createOstProperty " + pPropertyName + " already exists";
        BOOST_LOG_TRIVIAL(debug) << err.toStdString();
        return false;
    }
    else
    {
        QVariantMap _prop;
        _prop["propertyLabel"] = pPropertyLabel;
        _prop["permission"] = pPropertyPermission;
        _prop["devcat"] = pPropertyDevcat;
        _prop["group"] = pPropertyGroup;
        _prop["name"] = pPropertyName;
        _ostproperties[pPropertyName] = _prop;
        err = "OK";
        return true;
    }
}
void Basemodule::emitPropertyCreation(const QString &pPropertyName)
{
    QVariantMap _prop = _ostproperties[pPropertyName].toMap();
    emit moduleEvent("addprop", _modulename, pPropertyName, _prop);

}

void Basemodule::deleteOstProperty(QString propertyName)
{
    //BOOST_LOG_TRIVIAL(debug) << "deleteOstProperty  - " << _modulename.toStdString() << "-" << propertyName.toStdString();
    _ostproperties.remove(propertyName);
    emit moduleEvent("delprop", _modulename, propertyName, QVariantMap());

}

void Basemodule::setOstProperty(const QString &pPropertyName, QVariant _value, bool emitEvent)
{
    //BOOST_LOG_TRIVIAL(debug) << "setpropvalue  - " << _modulename.toStdString() << "-" << pPropertyName.toStdString();

    QVariantMap _prop = _ostproperties[pPropertyName].toMap();
    _prop["value"] = _value;
    _ostproperties[pPropertyName] = _prop;
    if (emitEvent) emit moduleEvent("setpropvalue", _modulename, pPropertyName, _prop);
}
void Basemodule::createOstElement (QString propertyName, QString elementName, QString elementLabel, bool emitEvent)
{
    if (!_ostproperties.contains(propertyName) )
    {
        sendMessage(" createOstElement - property " + propertyName + " not found");
        return ;
    }
    QVariantMap _prop = _ostproperties[propertyName].toMap();
    QVariantMap _elements = _prop["elements"].toMap();
    if (_elements.contains(elementName) )
    {
        sendMessage(" createOstElement - property " + propertyName + " : element " + elementName + "already exists.");
        return ;
    }
    QVariantMap _element = _elements[elementName].toMap();
    _element["elementLabel"] = elementLabel;
    _elements[elementName] = _element;
    _prop["elements"] = _elements;
    _ostproperties[propertyName] = _prop;
    if (emitEvent) emit moduleEvent("addelt", _modulename, propertyName, _prop);

}
bool Basemodule::setOstElement    (QString propertyName, QString elementName, QVariant elementValue, bool emitEvent)
{
    if (!_ostproperties.contains(propertyName) )
    {
        sendMessage(" setOstElement - property " + propertyName + " not found");
        return false;
    }

    QVariantMap _prop = _ostproperties[propertyName].toMap();

    if (_prop.contains("elements"))
    {
        if (_prop["elements"].toMap().contains(elementName))
        {
            QVariantMap _elements = _prop["elements"].toMap();
            if (!_elements.contains(elementName) )
            {
                sendMessage(" setOstElement - property " + propertyName + " : element " + elementName + " not found.");
                return false;
            }

            QVariantMap element = _elements[elementName].toMap();
            if (element.contains("value"))
            {
                if (strcmp(element["value"].typeName(), "double") == 0)
                {
                    //element["value"]=elementValue.toDouble();
                    element["value"].setValue(elementValue.toDouble());
                }
                if (strcmp(element["value"].typeName(), "float") == 0)
                {
                    //element["value"]=elementValue.toFloat();
                    element["value"].setValue(elementValue.toFloat());
                }
                if (strcmp(element["value"].typeName(), "qlonglong") == 0)
                {
                    //element["value"]=elementValue.toFloat();
                    element["value"].setValue(elementValue.toDouble());
                }
                if (strcmp(element["value"].typeName(), "int") == 0)
                {
                    //element["value"]=elementValue.toInt();
                    element["value"].setValue(elementValue.toInt());
                }
                if (strcmp(element["value"].typeName(), "QString") == 0)
                {
                    element["value"] = elementValue.toString();
                }
                if (strcmp(element["value"].typeName(), "bool") == 0)
                {
                    element["value"] = elementValue.toBool();
                }
            }
            else
            {
                element["value"] = elementValue;
            }
            _elements[elementName] = element;
            _prop["elements"] = _elements;
        }
    }
    _ostproperties[propertyName] = _prop;
    if (emitEvent) emit moduleEvent("setpropvalue", _modulename, propertyName, _prop);


    return true; // should return false when request is invalid, we'll see that later

}
bool Basemodule::pushOstElements        (QString propertyName)
{
    QVariantMap _prop = _ostproperties[propertyName].toMap();
    if (!_prop.contains("grid") )
    {
        qDebug() << this->getName() << "no grid defined for property " << propertyName;
        return false;
    }
    QVariantList _arr = _prop["grid"].toList();
    QVariantList _cols;
    foreach(auto _elt, _prop["elements"].toMap())
    {
        _cols.push_back(_elt.toMap()["value"]);
    }
    _arr.push_back(_cols);
    _prop["grid"] = _arr;
    _ostproperties[propertyName] = _prop;
    QVariantMap _mess;
    _mess["values"] = _cols;
    emit moduleEvent("pushvalues", _modulename, propertyName, _mess);
    return true;
}
bool Basemodule::resetOstElements      (QString propertyName)
{
    QVariantMap _prop = _ostproperties[propertyName].toMap();
    if (!_prop.contains("grid") )
    {
        qDebug() << this->getName() << "no grid defined for property " << propertyName;
        return false;
    }
    _prop["grid"].clear();
    _ostproperties[propertyName] = _prop;
    emit moduleEvent("resetvalues", _modulename, propertyName, QVariantMap());
    return true;
}

void Basemodule::setOstPropertyAttribute   (const QString &pPropertyName, const QString &pAttributeName, QVariant _value,
        bool emitEvent)
{
    //BOOST_LOG_TRIVIAL(debug) << "setOstPropertyAttribute  - " << _modulename.toStdString() << "-" << pPropertyName.toStdString();
    QVariantMap _prop = _ostproperties[pPropertyName].toMap();
    _prop[pAttributeName] = _value;
    _ostproperties[pPropertyName] = _prop;
    if (emitEvent)  emit moduleEvent("setattributes", _modulename, pPropertyName, _prop);

}
bool Basemodule::setOstElementAttribute(QString propertyName, QString elementName, QString attributeName, QVariant _value,
                                        bool emitEvent)
{
    QVariantMap _prop = _ostproperties[propertyName].toMap();
    if (_prop.contains("elements"))
    {
        if (_prop["elements"].toMap().contains(elementName))
        {
            QVariantMap elements = _prop["elements"].toMap();
            QVariantMap element = elements[elementName].toMap();
            element[attributeName] = _value;
            elements[elementName] = element;
            _prop["elements"] = elements;
        }
    }
    _ostproperties[propertyName] = _prop;
    if (emitEvent) emit moduleEvent("setpropvalue", _modulename, propertyName, _prop);
    return true; // should return false when request is invalid, we'll see that later


}

void Basemodule::loadPropertiesFromFile(QString fileName)
{
    QString val;
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject props = d.object();
    foreach(const QString &key, props.keys())
    {
        if (key != "baseVersion" && key != "baseVersion")
        {
            _ostproperties[key] = props[key].toVariant();
        }
    }

    //QByteArray docByteArray = d.toJson(QJsonDocument::Compact);
    //QString strJson = QLatin1String(docByteArray);
    //BOOST_LOG_TRIVIAL(debug) << "loadPropertiesFromFile  - " << _modulename.toStdString() << " - filename=" << fileName.toStdString() << " - " << strJson.toStdString();

}

void Basemodule::savePropertiesToFile(QString fileName)
{
    QVariantMap map = property("ostproperties").toMap();
    /*foreach(const QString& key, map.keys()) {
        if (key!="ostproperties")
        {
            QVariantMap mm=map[key].toMap();
            mm["propertyValue"]=property(key.toStdString().c_str());
            map[key]=mm;
        }
    }*/
    QJsonObject obj = QJsonObject::fromVariantMap(map);
    QJsonDocument doc(obj);

    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(doc.toJson());
    jsonFile.close();

}
void Basemodule::requestProfile(QString profileName)
{
    emit moduleEvent("profilerequest", _modulename, profileName, QVariantMap());
}

void Basemodule::setProfile(QVariantMap profiledata)
{
    foreach(const QString &key, profiledata.keys())
    {
        if (_ostproperties.contains(key))
        {
            QVariantMap data = profiledata[key].toMap();
            if (_ostproperties[key].toMap().contains("hasprofile"))
            {
                setOstProperty(key, data["value"], true);
                if (_ostproperties[key].toMap().contains("elements")
                        && data.contains("elements"))
                {
                    foreach(const QString &eltkey, profiledata[key].toMap()["elements"].toMap().keys())
                    {
                        setOstElement(key, eltkey, profiledata[key].toMap()["elements"].toMap()[eltkey].toMap()["value"], true);
                    }

                }
            }
        }

    }

}
void Basemodule::setProfiles(QVariantMap profilesdata)
{
    _availableProfiles = profilesdata;
}

QVariantMap Basemodule::getProfile(void)
{
    QVariantMap _res;

    foreach(const QString &keyprop, _ostproperties.keys())
    {
        if (_ostproperties[keyprop].toMap().contains("hasprofile"))
        {
            QVariantMap property;
            if (_ostproperties[keyprop].toMap().contains("value"))
            {
                property["value"] = _ostproperties[keyprop].toMap()["value"];
            }
            if (_ostproperties[keyprop].toMap().contains("elements"))
            {
                QVariantMap element, elements;
                foreach(const QString &keyelt, _ostproperties[keyprop].toMap()["elements"].toMap().keys())
                {
                    if (_ostproperties[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("value"))
                    {
                        element["value"] = _ostproperties[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
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
void Basemodule::OnExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                                 const QVariantMap &eventData)
{

    if ( (eventType == "readall") && ((eventModule == "*") || (eventModule == _modulename)) )
    {
        sendDump();
        return;
    }
    if ( (eventType == "setpropvalue") && (eventModule == _modulename) )
    {
        foreach(const QString &keyprop, eventData.keys())
        {
            if (!_ostproperties.contains(keyprop) )
            {
                sendMessage(" setpropvalue - property " + keyprop + " not found");
                return;
            }
            foreach(const QString &keyelt, eventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (!_ostproperties[keyprop].toMap()["elements"].toMap().contains(keyprop) )
                {
                    sendMessage(" setpropvalue - property " + keyprop + " - element " + keyelt +  " not found");
                    return;
                }
            }
        }
    }


    if (_modulename == eventModule)
    {
        foreach(const QString &keyprop, eventData.keys())
        {
            foreach(const QString &keyelt, eventData[keyprop].toMap()["elements"].toMap().keys())
            {
                QVariant val = eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                if ((keyprop == "profileactions") && (keyelt == "load"))
                {
                    setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                    if (val.toBool()) emit moduleEvent("modloadprofile", _moduletype, getOstElementValue("profileactions", "name").toString(),
                                                           QVariantMap());
                    return;
                }
                if ((keyprop == "profileactions") && (keyelt == "save"))
                {
                    setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                    if (val.toBool()) emit moduleEvent("modsaveprofile", _moduletype, getOstElementValue("profileactions", "name").toString(),
                                                           getProfile());
                    return;
                }
                if ((keyprop == "profileactions") && (keyelt == "name"))
                {
                    setOstElement("profileactions", "name", val, true);
                    return;
                }
                if ((keyprop == "moduleactions") && (keyelt == "kill"))
                {
                    this->~Basemodule();
                    return;
                }
            }
        }
    }
    /* dispatch any message to children */
    OnMyExternalEvent(eventType, eventModule, eventKey, eventData);
    OnDispatchToIndiExternalEvent(eventType, eventModule, eventKey, eventData);


}
QVariantMap Basemodule::getModuleInfo(void)
{
    QVariantMap temp;
    foreach (QString key, _ostproperties.keys())
    {
        if (_ostproperties[key].toMap()["devcat"].toString() == "Info")
        {
            temp[key] = _ostproperties[key].toMap()["value"];
        }
    }
    return temp;
}

void Basemodule::sendDump(void)
{
    emit moduleEvent("moduledump", _modulename, "*", _ostproperties);
}
