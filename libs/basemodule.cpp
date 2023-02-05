#include <QtCore>
#include <basedevice.h>
#include "basemodule.h"
#include "version.cc"

Basemodule::Basemodule(QString name, QString label, QString profile, QVariantMap params)
    : mAvailableModuleLibs(params),
      mModuleName(name),
      mModuleLabel(label)
{
    Q_INIT_RESOURCE(basemodule);
    Q_UNUSED(profile)
    loadOstPropertiesFromFile(":basemodule.json");
    setOstPropertyValue("moduleLabel", label, false);
    setOstPropertyValue("baseGitHash", QString::fromStdString(Version::GIT_SHA1), false);
    setOstPropertyValue("baseGitDate", QString::fromStdString(Version::GIT_DATE), false);
    setOstPropertyValue("baseGitMessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT), false);

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

void Basemodule::deleteOstProperty(const QString &pPropertyName)
{
    //BOOST_LOG_TRIVIAL(debug) << "deleteOstProperty  - " << mModulename.toStdString() << "-" << propertyName.toStdString();
    QVariantMap _props = mOstProperties["properties"].toMap();
    _props.remove(pPropertyName);
    mOstProperties["properties"] = _props;
    emit moduleEvent("delprop", getModuleName(), pPropertyName, QVariantMap());

}

bool Basemodule::pushOstElements         (const QString &pPropertyName)
{
    QVariantMap _props = mOstProperties["properties"].toMap();
    QVariantMap _prop = _props[pPropertyName].toMap();
    if (!_prop.contains("grid") )
    {
        qDebug() << getModuleName() << "no grid defined for property " << pPropertyName;
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
    emit moduleEvent("pushvalues", getModuleName(), pPropertyName, _mess);
    return true;
}
bool Basemodule::resetOstElements      (const QString &pPropertyName)
{

    QVariantMap _props = mOstProperties["properties"].toMap();
    QVariantMap _prop = _props[pPropertyName].toMap();
    if (!_prop.contains("grid") )
    {
        qDebug() << this->getModuleName() << "no grid defined for property " << pPropertyName;
        return false;
    }
    _prop["grid"].clear();
    _props[pPropertyName] = _prop;
    mOstProperties["properties"] = _props;
    emit moduleEvent("resetvalues", getModuleName(), pPropertyName, QVariantMap());
    return true;
}




void Basemodule::requestProfile(QString profileName)
{
    emit moduleEvent("profilerequest", getModuleName(), profileName, QVariantMap());
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

    if ( (pEventType == "Freadall") && ((pEventModule == "*") || (pEventModule == getModuleName())) )
    {
        sendDump();
        return;
    }

    /* just check if requested modification is possible */
    if ( (pEventType == "Fsetpropvalue") && (pEventModule == getModuleName()) )
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (!getProperties()["properties"].toMap().contains(keyprop) )
            {
                sendMessage(" Fsetpropvalue - property " + keyprop + " not found");
                return;
            }
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (!getProperties()["properties"].toMap()[keyprop].toMap()["elements"].toMap().contains(keyprop) )
                {
                    sendMessage(" Fsetpropvalue - property " + keyprop + " - element " + keyelt +  " not found");
                    return;
                }
            }
        }
    }


    if (getModuleName() == pEventModule)
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
    foreach (QString key, getProperties().keys())
    {
        //if (mOstProperties[key].toMap()["devcat"].toString() == "Info")
        if (key != "ostproperties")
        {
            temp[key] = getProperties()[key];
        }
    }
    return temp;
}

void Basemodule::sendDump(void)
{
    QVariantMap dump;
    QVariantMap state;
    QVariantMap infos;
    infos["name"] = getModuleName();
    infos["label"] = getModuleLabel();
    infos["description"] = getModuleDescription();
    dump["properties"] = getProperties();
    dump["state"] = state;
    dump["infos"] = infos;
    emit moduleEvent("moduledump", getModuleName(), "*", dump);
}
void Basemodule::OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                               const QVariantMap &eventData)
{
    emit moduleEvent(eventType, this->getModuleName(), eventKey, eventData);
}
bool Basemodule::setClassName(const QString &pClassName)
{
    if (getClassName() == "")
    {
        mClassName = pClassName;
        return true;
    }
    else
    {
        sendMessage("ClassName already set (" + mClassName +
                    ") - this method must be called only once, at the beginn of class constructor");
        return false;
    }
}
