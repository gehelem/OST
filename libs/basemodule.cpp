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
    loadOstPropertiesFromFile(":basemodule.json");
    setOstPropertyValue("moduleLabel", label, false);
    setOstPropertyValue("baseGitHash", QString::fromStdString(Version::GIT_SHA1), false);
    setOstPropertyValue("baseGitDate", QString::fromStdString(Version::GIT_DATE), false);
    setOstPropertyValue("baseGitMessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT), false);

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
                setOstPropertyValue(key, data["value"], true);
                if (_props[key].toMap().contains("elements")
                        && data.contains("elements"))
                {
                    foreach(const QString &eltkey, profiledata[key].toMap()["elements"].toMap().keys())
                    {
                        setOstElementValue(key, eltkey, profiledata[key].toMap()["elements"].toMap()[eltkey].toMap()["value"], true);
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
                        setOstElementValue("profileactions", "name", val, true);
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
    QVariantMap dump;
    dump["properties"] = getProperties();
    emit moduleEvent("moduledump", getName(), "*", dump);
}
void Basemodule::OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                               const QVariantMap &eventData)
{
    qDebug() << "BM OnDataStoreEvent " << eventType;
    emit moduleEvent(eventType, this->getName(), eventKey, eventData);
}
