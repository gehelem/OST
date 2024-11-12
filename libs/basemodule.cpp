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
    mModuleDesc = getEltString("moduleInfo", "moduleDescription");
    getEltString("moduleInfo", "moduleLabel")->setValue(label, false);
    getEltString("moduleInfo", "moduleName")->setValue(name, false);
    getEltString("moduleInfo", "moduleVersion")->setValue(0, false);
    getEltString("baseGit", "hash")->setValue(QString::fromStdString(Version::GIT_SHA1), true);
    getEltString("baseGit", "date")->setValue(QString::fromStdString(Version::GIT_DATE), true);
    getEltString("baseGit", "message")->setValue(QString::fromStdString(Version::GIT_COMMIT_SUBJECT), true);

    setModuleDescription("base module description - developer should change this message");
    mModuleDesc->setValue("base module description - developer should change this message", false);
    setModuleVersion("0.1");
    mStatus.ts = QDateTime::currentDateTime();
    mStatus.message = "init";
    mStatus.state = OST::Idle;

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
                if (data.contains("elements"))
                {
                    foreach(const QString &eltkey, props[key].toMap()["elements"].toMap().keys())
                    {
                        QVariant v = props[key].toMap()["elements"].toMap()[eltkey].toMap()["value"];
                        if (getEltBase(key, eltkey)->getType() == "int")
                        {
                            getEltInt(key, eltkey)->setValue(v.toInt(), true);
                        }
                        if (getEltBase(key, eltkey)->getType() == "float")
                        {
                            getEltFloat(key, eltkey)->setValue(v.toDouble(), true);
                        }
                        if (getEltBase(key, eltkey)->getType() == "string")
                        {
                            getEltString(key, eltkey)->setValue(v.toString(), true);
                        }
                        if (getEltBase(key, eltkey)->getType() == "bool")
                        {
                            getEltBool(key, eltkey)->setValue(v.toBool(), true);
                        }
                        if (props[key].toMap()["elements"].toMap()[eltkey].toMap().contains("gridvalues"))
                        {
                            //setOstElementGrid (key, eltkey, profiledata[key].toMap()["elements"].toMap()[eltkey].toMap()["gridvalues"].toList(), true);
                            OST::ElementUpdate v;
                            QVariantMap m;
                            QString a = "cleargrid";
                            getProperty(key)->getElt(eltkey)->accept(&v, a, m);
                            int size = props[key].toMap()["elements"].toMap()[eltkey].toMap()["gridvalues"].toList().size();
                            a = "newline";
                            for (int i = 0; i < size; i++)
                            {
                                m["val"] = props[key].toMap()["elements"].toMap()[eltkey].toMap()["gridvalues"].toList()[i];
                                getProperty(key)->getElt(eltkey)->accept(&v, a, m);
                            }
                        }
                    }
                }

                if (data.contains("elements") && getStore()[key]->hasGrid() && data.contains("grid") && data.contains("gridheaders"))
                {
                    getStore()[key]->clearGrid();
                    QVariantList lines = props[key].toMap()["grid"].toList();
                    foreach (const QVariant &vline, lines)
                    {
                        QVariantList line = vline.toList();
                        int icol = 0;
                        foreach(const QVariant &vv, line)
                        {
                            QString eltkey = data["gridheaders"].toList().at(icol).toString();
                            if (getEltBase(key, eltkey)->getType() == "int")
                            {
                                getEltInt(key, eltkey)->setValue(vv.toInt(), true);
                            }
                            if (getEltBase(key, eltkey)->getType() == "float")
                            {
                                getEltFloat(key, eltkey)->setValue(vv.toDouble(), true);
                            }
                            if (getEltBase(key, eltkey)->getType() == "string")
                            {
                                getEltString(key, eltkey)->setValue(vv.toString(), true);
                            }
                            if (getEltBase(key, eltkey)->getType() == "bool")
                            {
                                getEltBool(key, eltkey)->setValue(vv.toBool(), true);
                            }
                            if (getEltBase(key, eltkey)->getType() == "prg")
                            {
                                getEltPrg(key, eltkey)->setPrgValue(vv.toMap()["value"].toInt(), true);
                                getEltPrg(key, eltkey)->setDynLabel(vv.toMap()["dynlabel"].toString(), true);

                            }
                            if (getEltBase(key, eltkey)->getType() == "img")
                            {
                                OST::ImgData i;
                                i.mUrlJpeg = vv.toMap()["urljpeg"].toString();
                                i.mUrlFits = vv.toMap()["urlfits"].toString();
                                i.height = vv.toMap()["height"].toInt();
                                i.width = vv.toMap()["width"].toInt();
                                getEltImg(key, eltkey)->setValue(i, true);
                            }
                            if (getEltBase(key, eltkey)->getType() == "message")
                            {
                                OST::MsgData m;
                                m.level = OST::IntToMsgLevel(vv.toMap()["level"].toInt());
                                m.ts = vv.toMap()["ts"].toDateTime();
                                m.message = vv.toMap()["message"].toString();
                                getEltMsg(key, eltkey)->setValue(m, true);
                            }
                            icol++;
                        }
                        getStore()[key]->push();
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
    getEltString("loadprofile", "name")->lovClear();
    for(QVariantMap::const_iterator iter = profs.begin(); iter != profs.end(); ++iter)
    {
        getEltString("loadprofile", "name")->lovAdd(iter.key(), iter.key());
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
            if (!getStore()[keyprop]->isEnabled() )
            {
                sendWarning(" Fsetproperty - property " + keyprop + " is disabled - can't update");
                return;
            }
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (!getStore()[keyprop]->getElts()->contains(keyelt) )
                {
                    sendWarning(" Fsetproperty - property " + keyprop + " - element " + keyelt +  " not found");
                    return;
                }
            }
        }
    }

    if ( (pEventType == "Flup") && (pEventModule == getModuleName()) )
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (getStore()[keyprop]->autoUpDown() && getStore()[keyprop]->isEnabled() )
            {
                int l1 = pEventData[keyprop].toMap()["line"].toInt();
                int l2 = l1 + 1;
                getStore()[keyprop]->swapLines(l1, l2);
            }
        }
    }
    if ( (pEventType == "Fldown") && (pEventModule == getModuleName()) )
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (getStore()[keyprop]->autoUpDown() && getStore()[keyprop]->isEnabled())
            {
                int l1 = pEventData[keyprop].toMap()["line"].toInt();
                int l2 = l1 - 1;
                getStore()[keyprop]->swapLines(l1, l2);
            }
        }
    }
    if ( (pEventType == "Flselect")  && (pEventModule == getModuleName()) )
    {
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (getStore()[keyprop]->autoSelect() && getStore()[keyprop]->isEnabled())
            {
                double line = pEventData[keyprop].toMap()["line"].toDouble();
                getStore()[keyprop]->fetchLine(line);
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
                if (getStore()[keyprop]->getElt(keyelt)->autoUpdate() && getStore()[keyprop]->isEnabled() )
                {
                    QVariant v = pEventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                    if (getEltBase(keyprop, keyelt)->getType() == "int")
                    {
                        getEltInt(keyprop, keyelt)->setValue(v.toInt(), true);
                    }
                    if (getEltBase(keyprop, keyelt)->getType() == "float")
                    {
                        getEltFloat(keyprop, keyelt)->setValue(v.toDouble(), true);
                    }
                    if (getEltBase(keyprop, keyelt)->getType() == "string")
                    {
                        getEltString(keyprop, keyelt)->setValue(v.toString(), true);
                    }
                    if (getEltBase(keyprop, keyelt)->getType() == "bool")
                    {
                        getEltBool(keyprop, keyelt)->setValue(v.toBool(), true);
                    }
                    if (getEltBase(keyprop, keyelt)->getType() == "date")
                    {
                        QDate d;
                        d.setDate(v.toMap()["year"].toInt(), v.toMap()["month"].toInt(), v.toMap()["day"].toInt());
                        getEltDate(keyprop, keyelt)->setValue(d, true);
                    }
                    if (getEltBase(keyprop, keyelt)->getType() == "time")
                    {
                        QTime t;
                        t.setHMS(v.toMap()["hh"].toInt(), v.toMap()["mm"].toInt(), v.toMap()["ss"].toInt(), v.toMap()["ms"].toInt());
                        getEltTime(keyprop, keyelt)->setValue(t, true);
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
            getEltString("saveprofile", "name")->setValue(getString("loadprofile", "name"), true);
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
void Basemodule::setWebroot(QString webroot)
{
    mWebroot = webroot;
}

QString Basemodule::getWebroot(void)
{
    return mWebroot;
}
QVariantMap Basemodule::getAvailableModuleLibs(void)
{
    return mAvailableModuleLibs;
}
QString Basemodule::getModuleName()
{
    return mModuleName;
}
QString Basemodule::getModuleLabel()
{
    return mModuleLabel;
}
QString Basemodule::getModuleDescription()
{
    return mModuleDescription;
}
QString Basemodule::getModuleVersion()
{
    return mModuleVersion;
}
QString Basemodule::getClassName()
{
    return mClassName;
}
QString Basemodule::getHelpContent(QString language)
{
    QString f = ":" + getClassName() + "." + language + ".md";
    if (!QFile::exists(f))
    {
        return "No help content available";
    }
    QFile file;
    file.setFileName(f);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = QTextStream(&file).readAll();
    file.close();
    return content;
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
    dump["help"] = getHelpContent("fr");
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
void Basemodule::setModuleDescription(QString description)
{
    mModuleDescription = description;
    getEltString("moduleInfo", "moduleDescription")->setValue(description, true);
}
void Basemodule::setModuleVersion(QString version)
{
    mModuleVersion = version;
    getEltString("moduleInfo", "moduleVersion")->setValue(version, true);

}
void Basemodule::OnModuleStatusRequest()
{
    emit moduleStatusAnswer(this->getModuleName(), this->mStatus);
}

