#include <QtCore>
#include <basedevice.h>
#include "basemodule.h"
#include "version.cc"
#include "model/element/common.h"

Basemodule::Basemodule(QString name, QString label, QString profile, QVariantMap params)
    : mAvailableModuleLibs(params),
      mModuleName(name),
      mModuleLabel(label)
{
    Q_INIT_RESOURCE(basemodule);
    Q_UNUSED(profile)

    // Register meta types for queued signal/slot connections across modules
    qRegisterMetaType<OST::LogLevel>("OST::LogLevel");
    qRegisterMetaType<OST::Event>("OST::Event");

    connect(this, &Datastore::datastoreEvent, this, &Basemodule::onDatastoreEvent);

    loadOstPropertiesFromFile(":basemodule.json");
    setMetadata("label", label);
    setMetadata("baseGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("baseGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("baseGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("baseversion", "0.1");
    setMetadata("startdatetime", QDateTime::currentDateTime());
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
    emit moduleEvent(this, {"moduledelete", this->getModuleName(), "", "", 0, QVariantMap()});
    Q_CLEANUP_RESOURCE(basemodule);
}



void Basemodule::setProfile(const QString &pProfileName)
{
    QVariantMap prof;
    if (!getDbProfile(getClassName(), pProfileName, prof))
    {
        logWarning("Can't get %1 profile", {pProfileName});
        return;
    }
    setProfile(prof);
    emit moduleEvent(this, {"moduleSetProfile", this->getModuleName(), "", "", 0, QVariantMap()});

    logInfo("%1 profile sucessfully loaded", {pProfileName});
    QJsonObject ob;
    ob.fromVariantMap(prof);
}



void Basemodule::setProfile(QVariantMap profiledata)
{
    QVariantMap props = profiledata["p"].toMap();
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
                        if (getEltBase(key, eltkey)->getType() == "time")
                        {
                            int hh = props[key].toMap()["elements"].toMap()[eltkey].toMap()["hh"].toInt();
                            int mm = props[key].toMap()["elements"].toMap()[eltkey].toMap()["mm"].toInt();
                            int ss = props[key].toMap()["elements"].toMap()[eltkey].toMap()["ss"].toInt();
                            int ms = props[key].toMap()["elements"].toMap()[eltkey].toMap()["ms"].toInt();
                            QTime tt;
                            tt.setHMS(hh, mm, ss, ms);
                            getEltTime(key, eltkey)->setValue(tt, true);
                        }
                        if (getEltBase(key, eltkey)->getType() == "date")
                        {
                            int d = props[key].toMap()["elements"].toMap()[eltkey].toMap()["day"].toInt();
                            int m = props[key].toMap()["elements"].toMap()[eltkey].toMap()["month"].toInt();
                            int y = props[key].toMap()["elements"].toMap()[eltkey].toMap()["year"].toInt();
                            QDate dd;
                            dd.setDate(y, m, d);
                            getEltDate(key, eltkey)->setValue(dd, true);
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
                                getEltPrg(key, eltkey)->setPrgValue(vv.toMap()["value"].toInt(), false);
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
    logInfo("Available profiles refreshed");
}

void Basemodule::OnExternalEvent(OST::Event e)
{
    // Handle global lov updates from controller
    //if ( (e.type == "globallovupdate") && (e.module == "*") )
    //{
    //    // pEventKey contains the lov name (e.g., "loadedModules")
    //    // pEventData contains the lov data with "values" list
    //    if (!getGlobLovs().contains(pEventKey))
    //    {
    //        // Create the globallov if it doesn't exist
    //        OST::LovString* newLov = new OST::LovString(pEventKey);
    //        createGlobLov(pEventKey, newLov);
    //    }

    //    // Clear and repopulate the lov
    //    OST::LovString* lov = getGlovString(pEventKey);
    //    if (lov != nullptr)
    //    {
    //        lov->lovClear();
    //        QVariantList values = pEventData["values"].toList();
    //        for (const QVariant &item : values)
    //        {
    //            QVariantMap itemMap = item.toMap();
    //            lov->lovAdd(itemMap["key"].toString(), itemMap["label"].toString());
    //        }
    //    }
    //    return;
    //}

    if  (e.type == "Freadall")
    {
        sendDump();
        return;
    }

    /* just check if requested modification is possible */
    if ( (e.type == "Fsetproperty") && (e.module == getModuleName()) )
    {
        foreach(const QString &keyprop, e.data.keys())
        {
            if (!getStore().contains(keyprop) )
            {
                logWarning("Fsetproperty - property %1 not found", {keyprop});
                return;
            }
            if (!getStore()[keyprop]->isEnabled() )
            {
                logWarning("Fsetproperty - property %1 is disabled - can't update", {keyprop});
                return;
            }
            foreach(const QString &keyelt,  e.data[keyprop].toMap()["elements"].toMap().keys())
            {
                if (!getStore()[keyprop]->getElts()->contains(keyelt) )
                {
                    logWarning("Fsetproperty - property %1 - element %2 not found", {keyprop, keyelt});
                    return;
                }
            }
        }
    }

    if ( (e.type == "Flup") && (e.module == getModuleName()) )
    {
        foreach(const QString &keyprop, e.data.keys())
        {
            if (getStore()[keyprop]->autoUpDown() && getStore()[keyprop]->isEnabled() )
            {
                int l1 = e.data[keyprop].toMap()["line"].toInt();
                int l2 = l1 - 1;
                getStore()[keyprop]->swapLines(l1, l2);
            }
        }
    }
    if ( (e.type == "Fldown") && (e.module == getModuleName()) )
    {
        foreach(const QString &keyprop, e.data.keys())
        {
            if (getStore()[keyprop]->autoUpDown() && getStore()[keyprop]->isEnabled())
            {
                int l1 = e.data[keyprop].toMap()["line"].toInt();
                int l2 = l1 + 1;
                getStore()[keyprop]->swapLines(l1, l2);
            }
        }
    }
    if ( (e.type == "Flselect")  && (e.module == getModuleName()) )
    {
        foreach(const QString &keyprop, e.data.keys())
        {
            if (getStore()[keyprop]->autoSelect() && getStore()[keyprop]->isEnabled())
            {
                double line = e.data[keyprop].toMap()["line"].toDouble();
                getStore()[keyprop]->fetchLine(line);
            }
        }
    }

    /* autoupdate if wanted */

    if ( (e.type == "Fsetproperty") && (e.module == getModuleName()) )
    {
        foreach(const QString &keyprop, e.data.keys())
        {
            if (e.data[keyprop].toMap()["elements"].toMap().size() > 1)
            {
                getProperty(keyprop)->setAll(e.data[keyprop].toMap()["elements"].toMap());
            }
            else
            {
                foreach(const QString &keyelt, e.data[keyprop].toMap()["elements"].toMap().keys())
                {
                    if (getStore()[keyprop]->getElt(keyelt)->autoUpdate() && getStore()[keyprop]->isEnabled() )
                    {
                        QVariant v = e.data[keyprop].toMap()["elements"].toMap()[keyelt];
                        QVariantMap m = v.toMap();
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
                            //a boolean need to be updated on property level because of SwitchRule's flag behaviour
                            getProperty(keyprop)->setElt(keyelt, v, true);

                        }
                        if (getEltBase(keyprop, keyelt)->getType() == "date")
                        {
                            QDate d;
                            d.setDate(m["year"].toInt(), m["month"].toInt(), m["day"].toInt());
                            getEltDate(keyprop, keyelt)->setValue(d, true);
                        }
                        if (getEltBase(keyprop, keyelt)->getType() == "time")
                        {
                            QTime t;
                            t.setHMS(m["hh"].toInt(), m["mm"].toInt(), m["ss"].toInt(), m["ms"].toInt());
                            getEltTime(keyprop, keyelt)->setValue(t, true);
                        }
                        //sendMessage("Autoupdate - property " + keyprop + " - element " + keyelt);
                    }
                }
            }
        }
    }
    if ((e.type == "Fbadge") && e.module == getModuleName())
    {
        foreach(const QString &keyprop, e.data.keys())
        {
            if (!getStore().contains(keyprop) )
            {
                logWarning("Fbadge - property %1 not found", {keyprop});
                return;
            }
            getStore()[keyprop]->setBadge(!getStore()[keyprop]->getBadge());
        }
        return;
    }



    if ((e.type == "Fposticon") && (e.data.contains("saveprofile")) && e.module == getModuleName())
    {
        getProperty("saveprofile")->setState(OST::Busy, true);
        QVariantMap prof = getProfile();
        if (setDbProfile(getClassName(), getString("saveprofile", "name"), prof))
        {
            getProperty("saveprofile")->setState(OST::Ok, true);
            logInfo("%1 profile sucessfully saved", {getString("saveprofile", "name")});
            emit moduleEvent(this, {"modulesavedprofile", this->getModuleName(), "", "", 0, QVariantMap()});

        }
        else
        {
            getProperty("saveprofile")->setState(OST::Error, true);
            logWarning("Can't save %1 profile", {getString("saveprofile", "name")});
        }
        return;
    }

    if ((e.type == "Fposticon") && (e.data.contains("loadprofile")) && e.module == getModuleName())
    {
        getProperty("loadprofile")->setState(OST::Busy, true);
        QVariantMap prof;
        if (getDbProfile(getClassName(), getString("loadprofile", "name"), prof))
        {
            setProfile(prof);
            getProperty("loadprofile")->setState(OST::Ok, true);
            logInfo("%1 profile sucessfully loaded", {getString("loadprofile", "name")});
            emit moduleEvent(this, {"moduleloadedprofile", this->getModuleName(), "", "", 0, QVariantMap()});

            getEltString("saveprofile", "name")->setValue(getString("loadprofile", "name"), true);
            sendDump();
        }
        else
        {
            logWarning("Can't load %1 profile", {getString("loadprofile", "name")});
            getProperty("loadprofile")->setState(OST::Error, true);
        }
        return;

    }

    if ((e.type == "Fpreicon") && (e.data.contains("loadprofile")) && e.module == getModuleName())
    {
        getProperty("loadprofile")->setState(OST::Busy, true);
        setProfiles();
        getProperty("loadprofile")->setState(OST::Ok, true);
        return;
    }


    if ((getModuleName() == e.module ) && (e.type == "Fsetproperty") )
    {
        foreach(const QString &keyprop, e.data.keys())
        {
            if (keyprop == "moduleactions")
            {
                foreach(const QString &keyelt, e.data[keyprop].toMap()["elements"].toMap().keys())
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
    OnMyExternalEvent(e);
    OnDispatchToIndiExternalEvent(e);


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
QString Basemodule::getModuleName() const
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
    return getAllMetadata();
}

void Basemodule::sendDump(void)
{
    return;
    OST::Event e;
    e.type = "moduledump";
    e.module = getModuleName();
    QVariantMap dump;
    QVariantMap state;
    QVariantMap infos;
    infos["name"] = getModuleName();
    infos["label"] = getModuleLabel();
    infos["description"] = getModuleDescription();
    //dump["properties"] = getProperties();
    dump["p"] = getPropertiesDump(e);;
    dump["globallovs"] = getGlobalLovsDump();;
    dump["state"] = state;
    dump["infos"] = infos;
    dump["help"] = getHelpContent("fr");
    //getQtProperties();

    emit moduleEvent(this, e);
}
void Basemodule::OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                               const QVariantMap &eventData)
{
    return;
    Q_UNUSED(eventModule);
    emit moduleEvent(this, {"moduledump", this->getModuleName(), "", "", 0, QVariantMap()});
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
        logWarning("ClassName already set (%1) - this method must be called only once, at the begin of class constructor", {mClassName});
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
void Basemodule::onDatastoreEvent(Datastore* datastore, OST::Event e)
{
    Q_UNUSED(datastore)
    e.module = this->getModuleName();
    emit moduleEvent(this, e);
}

