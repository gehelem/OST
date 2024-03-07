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
    mModuleDesc = getValueString("moduleInfo", "moduleDescription");
    getValueString("moduleInfo", "moduleLabel")->setValue(label, false);
    getValueString("moduleInfo", "moduleName")->setValue(name, false);
    getValueString("moduleInfo", "moduleVersion")->setValue(0, false);
    getValueString("baseGit", "hash")->setValue(QString::fromStdString(Version::GIT_SHA1), true);
    getValueString("baseGit", "date")->setValue(QString::fromStdString(Version::GIT_DATE), true);
    getValueString("baseGit", "message")->setValue(QString::fromStdString(Version::GIT_COMMIT_SUBJECT), true);

    setModuleDescription("base module description - developer should change this message");
    mModuleDesc->setValue("base module description - developer should change this message", false);
    setModuleVersion("0.1");

}
Basemodule::~Basemodule()
{
    foreach(const QString &key, getStore().keys())
    {
        deleteOstProperty(key);
    }
    emit moduleEvent("moduledelete", getModuleName(), "*", QVariantMap());
    Q_CLEANUP_RESOURCE(basemodule);
}



void Basemodule::setProfile(const QString &pProfileName)
{
    QVariantMap prof;
    if (!getDbProfile(getClassName(), pProfileName, prof))
    {
        sendWarning("Can't get " + pProfileName + " profile");
        return;
    }
    setProfile(prof);
    emit moduleEvent("moduleSetProfile", getModuleName(), pProfileName, QVariantMap());
    sendMessage(pProfileName + " profile sucessfully loaded");
    QJsonObject ob;
    ob.fromVariantMap(prof);
}



void Basemodule::setProfile(QVariantMap profiledata)
{
    QVariantMap props = profiledata["properties"].toMap();
    foreach(const QString &key, props.keys())
    {
        if (getStore().contains(key))
        {
            QVariantMap data = props[key].toMap();
            if (getStore()[key]->hasProfile())
            {
                //setOstPropertyValue(key, data["value"], true);
                if (data.contains("elements"))
                {
                    foreach(const QString &eltkey, props[key].toMap()["elements"].toMap().keys())
                    {
                        QVariant v = props[key].toMap()["elements"].toMap()[eltkey].toMap()["value"];
                        if (getValueBase(key, eltkey)->getType() == "int")
                        {
                            getValueInt(key, eltkey)->setValue(v.toInt(), true);
                        }
                        if (getValueBase(key, eltkey)->getType() == "float")
                        {
                            getValueFloat(key, eltkey)->setValue(v.toDouble(), true);
                        }
                        if (getValueBase(key, eltkey)->getType() == "string")
                        {
                            getValueString(key, eltkey)->setValue(v.toString(), true);
                        }
                        if (getValueBase(key, eltkey)->getType() == "bool")
                        {
                            getValueBool(key, eltkey)->setValue(v.toBool(), true);
                        }


                        if (props[key].toMap()["elements"].toMap()[eltkey].toMap().contains("gridvalues"))
                        {
                            //setOstElementGrid (key, eltkey, profiledata[key].toMap()["elements"].toMap()[eltkey].toMap()["gridvalues"].toList(), true);
                            OST::ValueUpdate v;
                            QVariantMap m;
                            QString a = "cleargrid";
                            getProperty(key)->getValue(eltkey)->accept(&v, a, m);
                            int size = props[key].toMap()["elements"].toMap()[eltkey].toMap()["gridvalues"].toList().size();
                            a = "newline";
                            for (int i = 0; i < size; i++)
                            {
                                m["val"] = props[key].toMap()["elements"].toMap()[eltkey].toMap()["gridvalues"].toList()[i];
                                getProperty(key)->getValue(eltkey)->accept(&v, a, m);
                            }


                        }
                    }
                }
            }
        }
    }

}
void Basemodule::setProfiles()
{
    QVariantMap profs;
    getDbProfiles(getClassName(), profs);
    getValueString("loadprofile", "name")->lovClear();
    for(QVariantMap::const_iterator iter = profs.begin(); iter != profs.end(); ++iter)
    {
        getValueString("loadprofile", "name")->lovAdd(iter.key(), iter.key());
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
            if (!getStore().contains(keyprop) )
            {
                sendWarning(" Fsetproperty - property " + keyprop + " not found");
                return;
            }
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (!getStore()[keyprop]->getValues()->contains(keyelt) )
                {
                    sendWarning(" Fsetproperty - property " + keyprop + " - element " + keyelt +  " not found");
                    return;
                }
            }
        }
    }
    /* autoupdate if wanted */

    if ( (pEventType == "Fsetproperty") && (pEventModule == getModuleName()) )
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (getStore()[keyprop]->getValue(keyelt)->autoUpdate() )
                {
                    QVariant v = pEventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                    if (getValueBase(keyprop, keyelt)->getType() == "int")
                    {
                        getValueInt(keyprop, keyelt)->setValue(v.toInt(), true);
                    }
                    if (getValueBase(keyprop, keyelt)->getType() == "float")
                    {
                        getValueFloat(keyprop, keyelt)->setValue(v.toDouble(), true);
                    }
                    if (getValueBase(keyprop, keyelt)->getType() == "string")
                    {
                        getValueString(keyprop, keyelt)->setValue(v.toString(), true);
                    }
                    if (getValueBase(keyprop, keyelt)->getType() == "bool")
                    {
                        getValueBool(keyprop, keyelt)->setValue(v.toBool(), true);
                    }
                    //sendMessage("Autoupdate - property " + keyprop + " - element " + keyelt);
                }
            }
        }
    }
    if ((pEventType == "Fbadge") && pEventModule == getModuleName())
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (!getStore().contains(keyprop) )
            {
                sendWarning(" Fbadge - property " + keyprop + " not found");
                return;
            }
            getStore()[keyprop]->setBadge(!getStore()[keyprop]->getBadge());
        }
        return;
    }



    if ((pEventType == "Fposticon") && (pEventData.contains("saveprofile")) && pEventModule == getModuleName())
    {
        getProperty("saveprofile")->setState(OST::Busy);
        QVariantMap prof = getProfile();
        if (setDbProfile(getClassName(), getString("saveprofile", "name"), prof))
        {
            getProperty("saveprofile")->setState(OST::Ok);
            sendMessage(getString("saveprofile", "name") + " profile sucessfully saved");
            emit moduleEvent("modulesavedprofile", getModuleName(), getString("saveprofile", "name"),
                             QVariantMap());
        }
        else
        {
            getProperty("saveprofile")->setState(OST::Error);
            sendWarning("Can't save " + getString("saveprofile", "name") + " profile");
        }
        return;
    }

    if ((pEventType == "Fposticon") && (pEventData.contains("loadprofile")) && pEventModule == getModuleName())
    {
        getProperty("loadprofile")->setState(OST::Busy);
        QVariantMap prof;
        if (getDbProfile(getClassName(), getString("loadprofile", "name"), prof))
        {
            setProfile(prof);
            getProperty("loadprofile")->setState(OST::Ok);
            sendMessage(getString("loadprofile", "name") + " profile sucessfully loaded");
            emit moduleEvent("moduleloadedprofile", getModuleName(), getString("loadprofile", "name"),
                             QVariantMap());
            getValueString("saveprofile", "name")->setValue(getString("loadprofile", "name"), true);
            sendDump();
        }
        else
        {
            sendWarning("Can't load " + getString("loadprofile", "name") + " profile");
            getProperty("loadprofile")->setState(OST::Error);
        }
        return;

    }

    if ((pEventType == "Fpreicon") && (pEventData.contains("loadprofile")) && pEventModule == getModuleName())
    {
        getProperty("loadprofile")->setState(OST::Busy);
        setProfiles();
        getProperty("loadprofile")->setState(OST::Ok);
        return;
    }


    if ((getModuleName() == pEventModule ) && (pEventType == "Fsetproperty") )
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
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
    if ((getModuleName() == pEventModule ) && (pEventType == "Fclearmessages") )
    {
        resetMessages();
        resetWarnings();
        resetErrors();
    }
    /* dispatch any message to children */
    OnMyExternalEvent(pEventType, pEventModule, pEventKey, pEventData);
    OnDispatchToIndiExternalEvent(pEventType, pEventModule, pEventKey, pEventData);


}
void Basemodule::killMe()
{
    this->~Basemodule();
}
QVariantMap Basemodule::getModuleInfo(void)
{
    return getPropertiesDump()["moduleInfo"].toVariant().toMap();
}

void Basemodule::sendDump(void)
{

    QVariantMap dump;
    QVariantMap state;
    QVariantMap infos;
    infos["name"] = getModuleName();
    infos["label"] = getModuleLabel();
    infos["description"] = getModuleDescription();
    //dump["properties"] = getProperties();
    dump["properties"] = getPropertiesDump();;
    dump["globallovs"] = getGlobalLovsDump();;
    dump["state"] = state;
    dump["infos"] = infos;
    dump["messages"] = getMessages();
    dump["warnings"] = getWarnings();
    dump["errors"] = getErrors();
    //getQtProperties();
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
        sendWarning("ClassName already set (" + mClassName +
                    ") - this method must be called only once, at the begin of class constructor");
        return false;
    }
}
