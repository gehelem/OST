#include <QtCore>
#include <basedevice.h>
#include "basemodule.h"
#include "version.cc"
#include "model/element/common.h"
#include "libs/utils/modulejsondumper.h"

Basemodule::Basemodule(QString name, QString label, QString profile, QVariantMap params)
    : mAvailableModuleLibs(params),
      mModuleName(name),
      mModuleLabel(label)
{
    Q_INIT_RESOURCE(basemodule);
    Q_UNUSED(profile)

    // Register meta types for queued signal/slot connections across modules
    qRegisterMetaType<OST::LogLevel>("OST::LogLevel");
    qRegisterMetaType<OST::EvType>("OST::EvType");

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
    emit moduleEvent(OST::EvType::dm, this->getModuleName(), nullptr, nullptr, nullptr, nullptr);
    Q_CLEANUP_RESOURCE(basemodule);
}


/**
 * @brief Root event orchestrator (FINAL - Template Method pattern)
 *
 * This method cannot be overridden. It calls the virtual hooks in order:
 * 1. onExternalEventBase() - Common base module handling
 * 2. onExternalEventIndi() - INDI-specific handling (IndiModule only)
 * 3. onExternalEvent() - Custom module handling
 *
 * Called by signal/slot connections. Developers should NEVER call or override this!
 */
void Basemodule::onExternalEventRoot(OST::ExtEvent event)
{
    onExternalEventBase(event);

    switch (event.ev)
    {
        case OST::ExtEvType::ZZ:
        case OST::ExtEvType::DU:
        case OST::ExtEvType::LO:
        case OST::ExtEvType::IL:
        case OST::ExtEvType::FS:
        case OST::ExtEvType::CL:
        case OST::ExtEvType::CS:
        case OST::ExtEvType::PL:
        case OST::ExtEvType::PS:
            return;
        default:
            // Call hooks in order - each class overrides its own hook
            onExternalEventIndi(event);
            onExternalEvent(event);
    }

}

/**
 * @brief Base module event handler (Hook 1/3)
 *
 * Handles common module events:
 * - Profile load/save (PL, PS)
 * - Set value operations (SV, SA)
 * - Grid operations (GC, GU, GD, GF, GR)
 *
 * Called first in the event chain.
 */
void Basemodule::onExternalEventBase(OST::ExtEvent event)
{
    qDebug() << "Basemodule::onExternalEventBase event = " << OST::ExtEvToString(event.ev) << " p=" << event.prpkey << " e=" <<
             event.eltkey << " l=" << event.lovkey << " i=" << event.line;

    switch (event.ev)
    {
        case OST::ExtEvType::ZZ:
        case OST::ExtEvType::DU:
        case OST::ExtEvType::LO:
        case OST::ExtEvType::IL:
        case OST::ExtEvType::FS:
        case OST::ExtEvType::CL:
        case OST::ExtEvType::CS:
            logError("Basemodule::onExternalEvent - invalid event here - %1", {OST::ExtEvToString(event.ev)});
            return;
        default:
            if (!event.data.contains("m") || !event.data["m"].toObject().contains(this->getModuleName()) )
            {
                logError("Basemodule::onExternalEvent - invalid event data content - %1", {OST::ExtEvToString(event.ev)});
                return;
            };
    }

    QJsonObject o = event.data["m"].toObject()[this->getModuleName()].toObject();

    /* global module operations */
    //    PL,        /*!< Load profile */
    //    PS,        /*!< Save profile */

    if (event.ev == OST::ExtEvType::PL)
    {
        if(!o.contains("profile"))
        {
            logError("Basemodule::onExternalEvent - invalid profile load data content - %1", {OST::ExtEvToString(event.ev)});
            return;
        }
        if (this->loadProfile(o["profile"].toString())) logInfo("Profile %1 loaded", {o["profile"].toString()});
        else logError("Error loading profile %1", {o["profile"].toString()});

        return;
    }
    if (event.ev == OST::ExtEvType::PS)
    {
        if(!o.contains("profile"))
        {
            logError("Basemodule::onExternalEvent - invalid profile save data content - %1", {OST::ExtEvToString(event.ev)});
            return;
        }
        if (this->saveProfile(o["profile"].toString())) logInfo("Profile %1 saved", {o["profile"].toString()});
        else logError("Error saving profile %1", {o["profile"].toString()});
        return;
    }

    /* property target operations */
    //    SV,        /*!< set a value of a property */
    //    SA,        /*!< set all values of a property */
    //    GC,        /*!< grid new line */
    //    GU,        /*!< grid update line  */
    //    GF,        /*!< grid fetch line  */
    //    GD,        /*!< grid delete line  */
    //    GR,        /*!< grid reset */
    if (!getStore().contains(event.prpkey) )
    {
        logError("Basemodule::onExternalEvent - property %1 not found", {event.prpkey});
        return;
    }

    if (!getStore()[event.prpkey]->isEnabled())
    {
        logError("Basemodule::onExternalEvent - property %1 is disabled, can't update", {event.prpkey});
        return;
    }

    if (getStore()[event.prpkey]->permission() == OST::Permission::ReadOnly)
    {
        logError("Basemodule::onExternalEvent - property %1 is readonly, can't update", {event.prpkey});
        return;
    }
    QJsonObject p = o["p"].toObject();
    QJsonObject e = p[event.prpkey].toObject()["e"].toObject();

    if (event.ev == OST::ExtEvType::SV)
    {
        if (!getStore()[event.prpkey]->getElts()->contains(event.eltkey ))
        {
            logError("Basemodule::onExternalEvent - element %1-%2 not found", {event.prpkey, event.eltkey});
            return;
        }
        QVariantMap eltval;
        eltval["value"] = e.begin().value().toVariant();
        if (getStore()[event.prpkey]->getElt(event.eltkey)->autoUpdate())
        {
            OST::ElementUpdate u;
            bool b = true;
            getStore()[event.prpkey]->getElt(event.eltkey)->accept(&u, eltval, b);
        }
    }

    if (event.ev == OST::ExtEvType::SA)
    {
        getStore()[event.prpkey]->setAll(e.toVariantMap());
    }

    //    // Handle global lov updates from controller
    //    //if ( (e.type == "globallovupdate") && (e.module == "*") )
    //    //{
    //    //    // pEventKey contains the lov name (e.g., "loadedModules")
    //    //    // pEventData contains the lov data with "values" list
    //    //    if (!getGlobLovs().contains(pEventKey))
    //    //    {
    //    //        // Create the globallov if it doesn't exist
    //    //        OST::LovString* newLov = new OST::LovString(pEventKey);
    //    //        createGlobLov(pEventKey, newLov);
    //    //    }
    //
    //    //    // Clear and repopulate the lov
    //    //    OST::LovString* lov = getGlovString(pEventKey);
    //    //    if (lov != nullptr)
    //    //    {
    //    //        lov->lovClear();
    //    //        QVariantList values = pEventData["values"].toList();
    //    //        for (const QVariant &item : values)
    //    //        {
    //    //            QVariantMap itemMap = item.toMap();
    //    //            lov->lovAdd(itemMap["key"].toString(), itemMap["label"].toString());
    //    //        }
    //    //    }
    //    //    return;
    //    //}
    //
    //    if  (e.type == "Freadall")
    //    {
    //        sendDump();
    //        return;
    //    }
    //
    //    /* just check if requested modification is possible */
    //    if ( (e.type == "Fsetproperty") && (e.module == getModuleName()) )
    //    {
    //        foreach(const QString &keyprop, e.data.keys())
    //        {
    //            if (!getStore().contains(keyprop) )
    //            {
    //                logWarning("Fsetproperty - property %1 not found", {keyprop});
    //                return;
    //            }
    //            if (!getStore()[keyprop]->isEnabled() )
    //            {
    //                logWarning("Fsetproperty - property %1 is disabled - can't update", {keyprop});
    //                return;
    //            }
    //            foreach(const QString &keyelt,  e.data[keyprop].toMap()["elements"].toMap().keys())
    //            {
    //                if (!getStore()[keyprop]->getElts()->contains(keyelt) )
    //                {
    //                    logWarning("Fsetproperty - property %1 - element %2 not found", {keyprop, keyelt});
    //                    return;
    //                }
    //            }
    //        }
    //    }
    //
    //    if ( (e.type == "Flup") && (e.module == getModuleName()) )
    //    {
    //        foreach(const QString &keyprop, e.data.keys())
    //        {
    //            if (getStore()[keyprop]->autoUpDown() && getStore()[keyprop]->isEnabled() )
    //            {
    //                int l1 = e.data[keyprop].toMap()["line"].toInt();
    //                int l2 = l1 - 1;
    //                getStore()[keyprop]->swapLines(l1, l2);
    //            }
    //        }
    //    }
    //    if ( (e.type == "Fldown") && (e.module == getModuleName()) )
    //    {
    //        foreach(const QString &keyprop, e.data.keys())
    //        {
    //            if (getStore()[keyprop]->autoUpDown() && getStore()[keyprop]->isEnabled())
    //            {
    //                int l1 = e.data[keyprop].toMap()["line"].toInt();
    //                int l2 = l1 + 1;
    //                getStore()[keyprop]->swapLines(l1, l2);
    //            }
    //        }
    //    }
    //    if ( (e.type == "Flselect")  && (e.module == getModuleName()) )
    //    {
    //        foreach(const QString &keyprop, e.data.keys())
    //        {
    //            if (getStore()[keyprop]->autoSelect() && getStore()[keyprop]->isEnabled())
    //            {
    //                double line = e.data[keyprop].toMap()["line"].toDouble();
    //                getStore()[keyprop]->fetchLine(line);
    //            }
    //        }
    //    }
    //

    //    if ((e.type == "Fbadge") && e.module == getModuleName())
    //    {
    //        foreach(const QString &keyprop, e.data.keys())
    //        {
    //            if (!getStore().contains(keyprop) )
    //            {
    //                logWarning("Fbadge - property %1 not found", {keyprop});
    //                return;
    //            }
    //            getStore()[keyprop]->setBadge(!getStore()[keyprop]->getBadge());
    //        }
    //        return;
    //    }
    //
    //
    //
    //    if ((e.type == "Fposticon") && (e.data.contains("saveprofile")) && e.module == getModuleName())
    //    {
    //        getProperty("saveprofile")->setState(OST::Busy, true);
    //        QVariantMap prof = getProfile();
    //        if (setDbProfile(getClassName(), getString("saveprofile", "name"), prof))
    //        {
    //            getProperty("saveprofile")->setState(OST::Ok, true);
    //            logInfo("%1 profile sucessfully saved", {getString("saveprofile", "name")});
    //            emit moduleEvent(this, {"modulesavedprofile", this->getModuleName(), "", "", 0, QVariantMap()});
    //
    //        }
    //        else
    //        {
    //            getProperty("saveprofile")->setState(OST::Error, true);
    //            logWarning("Can't save %1 profile", {getString("saveprofile", "name")});
    //        }
    //        return;
    //    }
    //
    //    if ((e.type == "Fposticon") && (e.data.contains("loadprofile")) && e.module == getModuleName())
    //    {
    //        getProperty("loadprofile")->setState(OST::Busy, true);
    //        QVariantMap prof;
    //        if (getDbProfile(getClassName(), getString("loadprofile", "name"), prof))
    //        {
    //            setProfile(prof);
    //            getProperty("loadprofile")->setState(OST::Ok, true);
    //            logInfo("%1 profile sucessfully loaded", {getString("loadprofile", "name")});
    //            emit moduleEvent(this, {"moduleloadedprofile", this->getModuleName(), "", "", 0, QVariantMap()});
    //
    //            getEltString("saveprofile", "name")->setValue(getString("loadprofile", "name"), true);
    //            sendDump();
    //        }
    //        else
    //        {
    //            logWarning("Can't load %1 profile", {getString("loadprofile", "name")});
    //            getProperty("loadprofile")->setState(OST::Error, true);
    //        }
    //        return;
    //
    //    }
    //
    //    if ((e.type == "Fpreicon") && (e.data.contains("loadprofile")) && e.module == getModuleName())
    //    {
    //        getProperty("loadprofile")->setState(OST::Busy, true);
    //        setProfiles();
    //        getProperty("loadprofile")->setState(OST::Ok, true);
    //        return;
    //    }
    //
    //
    //    if ((getModuleName() == e.module ) && (e.type == "Fsetproperty") )
    //    {
    //        foreach(const QString &keyprop, e.data.keys())
    //        {
    //            if (keyprop == "moduleactions")
    //            {
    //                foreach(const QString &keyelt, e.data[keyprop].toMap()["elements"].toMap().keys())
    //                {
    //                    if (keyelt == "kill")
    //                    {
    //                        this->~Basemodule();
    //                        return;
    //                    }
    //
    //                }
    //            }
    //        }
    //    }
    //    /* dispatch any message to children */
    //    OnMyExternalEvent(e);
    //    OnDispatchToIndiExternalEvent(e);


}

/**
 * @brief INDI module event handler (Hook 2/3)
 *
 * Empty implementation in Basemodule.
 * Overridden by IndiModule to handle INDI-specific events.
 *
 * Called second in the event chain.
 */
void Basemodule::onExternalEventIndi(OST::ExtEvent event)
{
    Q_UNUSED(event);
    // Empty by default - IndiModule will override this
}

/**
 * @brief Custom module event handler (Hook 3/3)
 *
 * Empty implementation in Basemodule.
 * Overridden by custom modules (Dummy, Focus, Guider, etc.) to handle
 * module-specific events.
 *
 * Called last in the event chain.
 *
 * This is the PRIMARY method for module developers to override.
 */
void Basemodule::onExternalEvent(OST::ExtEvent event)
{
    Q_UNUSED(event);
    // Empty by default - custom modules will override this
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
QString Basemodule::getClassName()
{
    return metaObject()->className();
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

void Basemodule::onDatastoreEvent(OST::EvType evt, QVariant data, OST::ElementBase* elt, OST::PropertyBase* prp,
                                  OST::LovBase* lov, Datastore* mod)
{
    emit moduleEvent(evt, data, elt, prp, lov, this);
}

bool Basemodule::saveProfile(const QString &pProfileName)
{
    /* build a values only dedicated dump */
    QJsonObject obj = OST::ModuleJsonDumper(OST::EvType::pr, QVariant(), nullptr, nullptr, nullptr, this).toObject();
    /* save it */
    if (this->setDbProfile(this->getClassName(), pProfileName, obj)) emit moduleEvent(OST::EvType::fs, pProfileName, nullptr,
                nullptr, nullptr, this);
}
bool Basemodule::loadProfile(const QString &pProfileName)
{
    QJsonObject obj;
    if (!this->getDbProfile(this->getClassName(), pProfileName, obj)) return false;
    QVariantMap props = obj["p"].toVariant().toMap();
    foreach(const QString &key, props.keys())
    {
        if (getStore().contains(key))
        {
            QVariantMap data = props[key].toMap();
            if (getStore()[key]->hasProfile())
            {
                if (data.contains("e"))
                {
                    foreach(const QString &eltkey, props[key].toMap()["e"].toMap().keys())
                    {
                        QVariant v = props[key].toMap()["e"].toMap()[eltkey].toMap()["value"];
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
                            int hh = props[key].toMap()["e"].toMap()[eltkey].toMap()["hh"].toInt();
                            int mm = props[key].toMap()["e"].toMap()[eltkey].toMap()["mm"].toInt();
                            int ss = props[key].toMap()["e"].toMap()[eltkey].toMap()["ss"].toInt();
                            int ms = props[key].toMap()["e"].toMap()[eltkey].toMap()["ms"].toInt();
                            QTime tt;
                            tt.setHMS(hh, mm, ss, ms);
                            getEltTime(key, eltkey)->setValue(tt, true);
                        }
                        if (getEltBase(key, eltkey)->getType() == "date")
                        {
                            int d = props[key].toMap()["e"].toMap()[eltkey].toMap()["day"].toInt();
                            int m = props[key].toMap()["e"].toMap()[eltkey].toMap()["month"].toInt();
                            int y = props[key].toMap()["e"].toMap()[eltkey].toMap()["year"].toInt();
                            QDate dd;
                            dd.setDate(y, m, d);
                            getEltDate(key, eltkey)->setValue(dd, true);
                        }

                    }
                }

                if (data.contains("e") && getStore()[key]->hasGrid() && data.contains("grid") && data.contains("gridheaders"))
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

    return true;
}
void Basemodule::updateProfilesLov()
{
    QVariantMap profs;
    getDbProfiles(getClassName(), profs);
    getGlovString("profiles")->lovClear();
    for(QVariantMap::const_iterator iter = profs.begin(); iter != profs.end(); ++iter)
    {
        getGlovString("profiles")->lovAdd(iter.key(), iter.key());
    }
}
