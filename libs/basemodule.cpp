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
    foreach(const QString &key, getProperties().keys())
    {
        deleteOstProperty(key);
    }
    Q_CLEANUP_RESOURCE(basemodule);
}



void Basemodule::setProfile(const QString &pProfileName)
{
    QVariantMap prof;
    if (getDbProfile(getClassName(), pProfileName, prof))
    {
        setProfile(prof);
        sendMessage(pProfileName + " profile sucessfully loaded");
    }
    else
    {
        sendWarning("Can't load " + pProfileName + " profile");
    }


}



void Basemodule::setProfile(QVariantMap profiledata)
{
    QVariantMap _props = getProperties();
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
                        if (_props[key].toMap().contains("grid"))
                        {
                            setOstElementGrid (key, eltkey, profiledata[key].toMap()["elements"].toMap()[eltkey].toMap()["gridvalues"].toList(), true);
                        }
                    }
                }
            }
        }
    }

}
void Basemodule::setProfiles(QVariantMap profilesdata)
{
    Q_UNUSED(profilesdata);
    //mAvailableProfiles = profilesdata;
    QVariantMap profs;
    getDbProfiles(getClassName(), profs);
    clearOstLov("loadprofile");
    for(QVariantMap::const_iterator iter = profs.begin(); iter != profs.end(); ++iter)
    {
        //qDebug() << iter.key() << iter.value();
        addOstLov("loadprofile", iter.key(), iter.key() );
    }
    sendMessage("Available profiles refreshed");
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
    if ( (pEventType == "Fsetproperty") && (pEventModule == getModuleName()) )
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (!getProperties().contains(keyprop) )
            {
                sendMessage(" Fsetproperty - property " + keyprop + " not found");
                return;
            }
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (!getProperties()[keyprop].toMap()["elements"].toMap().contains(keyelt) )
                {
                    sendMessage(" Fsetproperty - property " + keyprop + " - element " + keyelt +  " not found");
                    return;
                }
            }
        }
    }


    if ((getModuleName() == pEventModule ) && (pEventType == "Fsetproperty") )
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (keyprop == "loadprofile")
            {
                if (pEventData[keyprop].toMap().contains("value"))
                {
                    QVariant val = pEventData[keyprop].toMap()["value"];
                    setOstPropertyValue(keyprop, val, true);
                    setOstPropertyValue("saveprofile", val, true);
                }
                if (pEventData[keyprop].toMap().contains("elements"))
                {
                    foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
                    {
                        QVariant val = pEventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                        if (keyelt == "load" && val.toBool())
                        {
                            setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                            QVariantMap prof;
                            if (getDbProfile(getClassName(), getOstPropertyValue("loadprofile").toString(), prof))
                            {
                                setProfile(prof);
                                setOstPropertyAttribute(keyprop, "status", IPS_OK, true);
                                sendMessage(getOstPropertyValue("loadprofile").toString() + " profile sucessfully loaded");
                                sendDump();
                            }
                            else
                            {
                                sendMessage("Can't load " + getOstElementValue("loadprofile", "name").toString() + " profile");
                                setOstPropertyAttribute(keyprop, "status", IPS_ALERT, true);
                            }
                            setOstElementValue("loadprofile", "load", false, false);
                            setOstElementValue("loadprofile", "refresh", false, true);
                            return;
                        }
                        if (keyelt == "refresh" && val.toBool())
                        {
                            setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                            QVariantMap prof;
                            setProfiles(QVariantMap());
                            setOstElementValue("loadprofile", "load", false, false);
                            setOstElementValue("loadprofile", "refresh", false, false);
                            setOstPropertyAttribute(keyprop, "status", IPS_OK, true);
                            return;
                        }

                    }
                }
            }

            if (keyprop == "saveprofile")
            {
                if (pEventData[keyprop].toMap().contains("value"))
                {
                    QVariant val = pEventData[keyprop].toMap()["value"];
                    setOstPropertyValue(keyprop, val, true);
                }


                foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
                {
                    QVariant val = pEventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                    if (keyelt == "save"  && val.toBool())
                    {
                        setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                        QVariantMap prof = getProfile();
                        if (setDbProfile(getClassName(), getOstPropertyValue("saveprofile").toString(), prof))
                        {
                            setOstPropertyAttribute(keyprop, "status", IPS_OK, true);
                            sendMessage(getOstPropertyValue("saveprofile").toString() + " profile sucessfully saved");
                        }
                        else
                        {
                            setOstPropertyAttribute(keyprop, "status", IPS_ALERT, true);
                            sendMessage("Can't save " + getOstPropertyValue("saveprofile").toString() + " profile");
                        }
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
        if (key == "moduleDescription")
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
    dump["messages"] = getMessages();
    dump["warnings"] = getWarnings();
    dump["errors"] = getErrors();
    emit moduleEvent("moduledump", getModuleName(), "*", dump);
}
void Basemodule::OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                               const QVariantMap &eventData)
{
    Q_UNUSED(eventModule);
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
