#include <QtCore>
#include <basedevice.h>
#include "basemodule.h"
#include "version.cc"

Basemodule::Basemodule(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : mAvailableModuleLibs(availableModuleLibs)
{
    Q_INIT_RESOURCE(basemodule);
    Q_UNUSED(profile)
    setName(name);
    setModuleLabel(label);
    loadPropertiesFromFile(":basemodule.json");
    setOstProperty("moduleLabel", label, false);
    setOstProperty("baseGitHash", QString::fromStdString(Version::GIT_SHA1), false);
    setOstProperty("baseGitDate", QString::fromStdString(Version::GIT_DATE), false);
    setOstProperty("baseGitMessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT), false);

    setModuleLabel(label);
    setModuleDescription("base module description - developer should change this message");
    setModuleVersion("0.1");



}
Basemodule::~Basemodule()
{
    foreach(const QString &key, mOstProperties.keys())
    {
        deleteOstProperty(key);
    }
    Q_CLEANUP_RESOURCE(basemodule);
}
void Basemodule::sendMessage(QString message)
{
    QString mess = QDateTime::currentDateTime().toString("[yyyyMMdd hh:mm:ss.zzz]") + " - " + getName() + " - " + message;
    qDebug() << mess;
    setOstProperty("message", mess, true);
}

bool Basemodule::createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel,
                                   const int &pPropertyPermission, const  QString &pPropertyDevcat, const QString &pPropertyGroup, QString &err)
{
    //BOOST_LOG_TRIVIAL(debug) << "createOstProperty  - " << mModulename.toStdString() << "-" << pPropertyName.toStdString();
    if (mOstProperties["properties"].toMap().contains(pPropertyName))
    {
        err = getName() + " - createOstProperty " + pPropertyName + " already exists";
        BOOST_LOG_TRIVIAL(debug) << err.toStdString();
        return false;
    }
    else
    {
        QVariantMap _prop, _props;
        _prop["propertyLabel"] = pPropertyLabel;
        _prop["permission"] = pPropertyPermission;
        _prop["devcat"] = pPropertyDevcat;
        _prop["group"] = pPropertyGroup;
        _prop["name"] = pPropertyName;
        _props = mOstProperties["properties"].toMap();
        _props[pPropertyName] = _prop;
        mOstProperties["properties"] = _props;
        err = "OK";
        return true;
    }
}
void Basemodule::emitPropertyCreation(const QString &pPropertyName)
{
    QVariantMap _prop = mOstProperties["properties"].toMap()[pPropertyName].toMap();
    emit moduleEvent("addprop", getName(), pPropertyName, _prop);
}

void Basemodule::deleteOstProperty(const QString &pPropertyName)
{
    //BOOST_LOG_TRIVIAL(debug) << "deleteOstProperty  - " << mModulename.toStdString() << "-" << propertyName.toStdString();
    QVariantMap _props = mOstProperties["properties"].toMap();
    _props.remove(pPropertyName);
    mOstProperties["properties"] = _props;
    emit moduleEvent("delprop", getName(), pPropertyName, QVariantMap());

}

void Basemodule::setOstProperty(const QString &pPropertyName, const QVariant &pValue, bool emitEvent)
{
    //BOOST_LOG_TRIVIAL(debug) << "setpropvalue  - " << mModulename.toStdString() << "-" << pPropertyName.toStdString();
    QVariantMap _props = mOstProperties["properties"].toMap();
    QVariantMap _prop = _props[pPropertyName].toMap();
    _prop["value"] = pValue;
    _props[pPropertyName] = _prop;
    mOstProperties["properties"] = _props;
    if (emitEvent) emit moduleEvent("setpropvalue", getName(), pPropertyName, _prop);
}
void Basemodule::createOstElement (const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                   bool mEmitEvent)
{
    QVariantMap _props = mOstProperties["properties"].toMap();
    if (!_props.contains(pPropertyName) )
    {
        QString toto = " createOstElement - property " + pPropertyName + " not found";
        sendMessage(toto);
        return ;
    }

    QVariantMap _prop = _props[pPropertyName].toMap();
    QVariantMap _elements = _prop["elements"].toMap();
    if (_elements.contains(pPropertyName) )
    {
        sendMessage(" createOstElement - property " + pPropertyName + " : element " + pElementName + "already exists.");
        return ;
    }
    QVariantMap _element = _elements[pElementName].toMap();
    _element["elementLabel"] = pElementLabel;
    _elements[pElementName] = _element;
    _prop["elements"] = _elements;
    _props[pPropertyName] = _prop;
    mOstProperties["properties"] = _props;
    if (mEmitEvent) emit moduleEvent("addelt", getName(), pPropertyName, _prop);

}
bool Basemodule::setOstElement    (const QString &pPropertyName, const QString &pElementName,
                                   const  QVariant &pElementValue, bool mEmitEvent)
{
    QVariantMap _props = mOstProperties["properties"].toMap();
    if (!_props.contains(pPropertyName) )
    {
        sendMessage(" setOstElement - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap _prop = _props[pPropertyName].toMap();

    if (_prop.contains("elements"))
    {
        if (_prop["elements"].toMap().contains(pElementName))
        {
            QVariantMap _elements = _prop["elements"].toMap();
            if (!_elements.contains(pElementName) )
            {
                sendMessage(" setOstElement - property " + pPropertyName + " : element " + pElementName + " not found.");
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
    _props[pPropertyName] = _prop;
    mOstProperties["properties"] = _props;
    if (mEmitEvent) emit moduleEvent("setpropvalue", getName(), pPropertyName, _prop);


    return true; // should return false when request is invalid, we'll see that later

}
bool Basemodule::pushOstElements         (const QString &pPropertyName)
{
    QVariantMap _props = mOstProperties["properties"].toMap();
    QVariantMap _prop = _props[pPropertyName].toMap();
    if (!_prop.contains("grid") )
    {
        qDebug() << this->getName() << "no grid defined for property " << pPropertyName;
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
    _props[pPropertyName] = _prop;
    mOstProperties["properties"] = _props;
    QVariantMap _mess;
    _mess["values"] = _cols;
    emit moduleEvent("pushvalues", getName(), pPropertyName, _mess);
    return true;
}
bool Basemodule::resetOstElements      (const QString &pPropertyName)
{

    QVariantMap _props = mOstProperties["properties"].toMap();
    QVariantMap _prop = _props[pPropertyName].toMap();
    if (!_prop.contains("grid") )
    {
        qDebug() << this->getName() << "no grid defined for property " << pPropertyName;
        return false;
    }
    _prop["grid"].clear();
    _props[pPropertyName] = _prop;
    mOstProperties["properties"] = _props;
    emit moduleEvent("resetvalues", getName(), pPropertyName, QVariantMap());
    return true;
}

void Basemodule::setOstPropertyAttribute   (const QString &pPropertyName, const QString &pAttributeName, QVariant _value,
        bool emitEvent)
{
    //BOOST_LOG_TRIVIAL(debug) << "setOstPropertyAttribute  - " << mModulename.toStdString() << "-" << pPropertyName.toStdString();
    QVariantMap _props = mOstProperties["properties"].toMap();
    QVariantMap _prop = _props[pPropertyName].toMap();

    _prop[pAttributeName] = _value;
    _props[pPropertyName] = _prop;
    mOstProperties["properties"] = _props;
    if (emitEvent)  emit moduleEvent("setattributes", getName(), pPropertyName, _prop);

}
bool Basemodule::setOstElementAttribute(const QString &pPropertyName, const QString &pElementName,
                                        const  QString &pAttributeName,
                                        const QVariant &pValue,
                                        bool mEmitEvent)
{
    QVariantMap _props = mOstProperties["properties"].toMap();
    QVariantMap _prop = _props[pPropertyName].toMap();
    if (_prop.contains("elements"))
    {
        if (_prop["elements"].toMap().contains(pElementName))
        {
            QVariantMap elements = _prop["elements"].toMap();
            QVariantMap element = elements[pElementName].toMap();
            element[pAttributeName] = pValue;
            elements[pElementName] = element;
            _prop["elements"] = elements;
        }
    }
    _props[pPropertyName] = _prop;
    mOstProperties["properties"] = _props;
    if (mEmitEvent) emit moduleEvent("setpropvalue", getName(), pPropertyName, _prop);
    return true; // should return false when request is invalid, we'll see that later


}

void Basemodule::loadPropertiesFromFile(const QString &pFileName)
{
    QVariantMap _props = mOstProperties["properties"].toMap();

    QString val;
    QFile file;
    file.setFileName(pFileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject props = d.object();
    foreach(const QString &key, props.keys())
    {
        if (key != "baseVersion" && key != "baseVersion")
        {
            _props[key] = props[key].toVariant();
        }
    }
    mOstProperties["properties"] = _props;

    //QByteArray docByteArray = d.toJson(QJsonDocument::Compact);
    //QString strJson = QLatin1String(docByteArray);
    //BOOST_LOG_TRIVIAL(debug) << "loadPropertiesFromFile  - " << mModulename.toStdString() << " - filename=" << fileName.toStdString() << " - " << strJson.toStdString();

}

void Basemodule::savePropertiesToFile(const QString &pFileName)
{
    QJsonObject obj = QJsonObject::fromVariantMap(mOstProperties);
    QJsonDocument doc(obj);

    QFile jsonFile(pFileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(doc.toJson());
    jsonFile.close();

}
void Basemodule::requestProfile(QString profileName)
{
    emit moduleEvent("profilerequest", getName(), profileName, QVariantMap());
}

void Basemodule::setProfile(QVariantMap profiledata)
{
    QVariantMap _props = mOstProperties["properties"].toMap();

    foreach(const QString &key, profiledata.keys())
    {
        if (_props.contains(key))
        {
            QVariantMap data = profiledata[key].toMap();
            if (_props[key].toMap().contains("hasprofile"))
            {
                setOstProperty(key, data["value"], true);
                if (_props[key].toMap().contains("elements")
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
    mAvailableProfiles = profilesdata;
}

QVariantMap Basemodule::getProfile(void)
{
    QVariantMap _res;
    QVariantMap _props = mOstProperties["properties"].toMap();

    foreach(const QString &keyprop, _props.keys())
    {
        if (_props[keyprop].toMap().contains("hasprofile"))
        {
            QVariantMap property;
            if (_props[keyprop].toMap().contains("value"))
            {
                property["value"] = _props[keyprop].toMap()["value"];
            }
            if (_props[keyprop].toMap().contains("elements"))
            {
                QVariantMap element, elements;
                foreach(const QString &keyelt, _props[keyprop].toMap()["elements"].toMap().keys())
                {
                    if (_props[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("value"))
                    {
                        element["value"] = _props[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
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
void Basemodule::OnExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                 const QVariantMap &pEventData)
{
    if ( (pEventType == "readall") && ((pEventModule == "*") || (pEventModule == getName())) )
    {
        sendDump();
        return;
    }
    if ( (pEventType == "setpropvalue") && (pEventModule == getName()) )
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (!mOstProperties["properties"].toMap().contains(keyprop) )
            {
                sendMessage(" setpropvalue - property " + keyprop + " not found");
                return;
            }
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (!mOstProperties["properties"].toMap()[keyprop].toMap()["elements"].toMap().contains(keyprop) )
                {
                    sendMessage(" setpropvalue - property " + keyprop + " - element " + keyelt +  " not found");
                    return;
                }
            }
        }
    }


    if (getName() == pEventModule)
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (keyprop == "profileactions")
            {
                foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
                {
                    QVariant val = pEventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                    if (keyelt == "load")
                    {
                        //setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                        if (val.toBool()) emit moduleEvent("modloadprofile", this->metaObject()->className(), getOstElementValue("profileactions",
                                                               "name").toString(),
                                                               QVariantMap());
                    }
                    if (keyelt == "save")
                    {
                        //setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                        if (val.toBool()) emit moduleEvent("modsaveprofile", this->metaObject()->className(), getOstElementValue("profileactions",
                                                               "name").toString(),
                                                               getProfile());
                    }
                    if (keyelt == "name")
                    {
                        setOstElement("profileactions", "name", val, true);
                    }

                }
                return;
            }

            if (keyprop == "moduleactions")
            {
                foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
                {
                    if (keyelt == "kill")
                    {
                        this->~Basemodule();
                        return;
                    }

                }
            }
        }
    }
    /* dispatch any message to children */
    OnMyExternalEvent(pEventType, pEventModule, pEventKey, pEventData);
    OnDispatchToIndiExternalEvent(pEventType, pEventModule, pEventKey, pEventData);


}
QVariantMap Basemodule::getModuleInfo(void)
{
    QVariantMap temp;
    foreach (QString key, mOstProperties.keys())
    {
        //if (mOstProperties[key].toMap()["devcat"].toString() == "Info")
        if (key != "ostproperties")
        {
            temp[key] = mOstProperties[key];
        }
    }
    return temp;
}

void Basemodule::sendDump(void)
{
    emit moduleEvent("moduledump", getName(), "*", mOstProperties);
}
