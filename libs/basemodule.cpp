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
    qRegisterMetaType<OST::ExtEvent>("OST::ExtEvent");

    connect(this, &Datastore::datastoreEvent, this, &Basemodule::onDatastoreEvent);

    loadOstPropertiesFromFile(":basemodule.json");
    setMetadata("modulelabel", label);
    setMetadata("baseGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("baseGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("baseGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("baseversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("startdatetime", QDateTime::currentDateTime());
    mStatus.ts = QDateTime::currentDateTime();
    mStatus.message = "init";
    mStatus.state = OST::Idle;

}
Basemodule::~Basemodule()
{
    for(const QString &key : getStore().keys())
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
    if (!onExternalEventBase(event)) return;

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
        case OST::ExtEvType::QY:
            return;
        default:
        {
            // Call hooks in order - each class overrides its own hook
            if (!onExternalEventIndi(event)) return;
            onExternalEvent(event);
        }
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
bool Basemodule::onExternalEventBase(OST::ExtEvent event)
{
    //qDebug() << "Basemodule::onExternalEventBase event = " << OST::ExtEvToString(event.ev) << " p=" << event.prpkey << " e=" <<
    //         event.eltkey << " l=" << event.lovkey << " i=" << event.line;

    switch (event.ev)
    {
        case OST::ExtEvType::ZZ:
        case OST::ExtEvType::DU:
        case OST::ExtEvType::LO:
        case OST::ExtEvType::IL:
        case OST::ExtEvType::FS:
        case OST::ExtEvType::CL:
        case OST::ExtEvType::CS:
        {
            logError("Basemodule::onExternalEventBase - invalid event here - %1", {OST::ExtEvToString(event.ev)});
            return false;
        }
        default:
            if (!event.data.contains("m") || !event.data["m"].toObject().contains(this->getModuleName()) )
            {
                logError("Basemodule::onExternalEventBase - invalid event data content - %1", {OST::ExtEvToString(event.ev)});
                return false;
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
            logError("Basemodule::onExternalEventBase - invalid profile load data content - %1", {OST::ExtEvToString(event.ev)});
            return false;
        }
        if (this->loadProfile(o["profile"].toString()))
        {
            logInfo("Profile %1 loaded", {o["profile"].toString()});
            return true;
        }
        else
        {
            logError("Error loading profile %1", {o["profile"].toString()});
            return false;
        }
    }
    if (event.ev == OST::ExtEvType::PS)
    {
        if(!o.contains("profile"))
        {
            logError("Basemodule::onExternalEventBase - invalid profile save data content - %1", {OST::ExtEvToString(event.ev)});
            return false;
        }
        if (this->saveProfile(o["profile"].toString()))
        {
            logInfo("Profile %1 saved", {o["profile"].toString()});
            return true;
        }
        else
        {
            logError("Error saving profile %1", {o["profile"].toString()});
            return false;
        }
    }

    if (event.ev == OST::ExtEvType::QY)
    {
        if (!o.contains("query"))
        {
            logError("Basemodule::onExternalEventBase - invalid query data content - %1", {OST::ExtEvToString(event.ev)});
            return false;
        }
        QString queryName = o["query"].toString();
        QVariantMap params = o["params"].toObject().toVariantMap();
        QVariantMap result = this->onModuleQuery(queryName, params);
        QVariantMap answer;
        answer["query"]  = queryName;
        answer["result"] = result;
        emit moduleEvent(OST::EvType::qa, answer, nullptr, nullptr, nullptr, this);
        return true;
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
        logError("Basemodule::onExternalEventBase - property %1 not found", {event.prpkey});
        return false;
    }

    if (event.ev == OST::ExtEvType::DP)
    {
        getStore()[event.prpkey]->sendDump();
    }

    if (!getStore()[event.prpkey]->isEnabled())
    {
        logError("Basemodule::onExternalEventBase - property %1 is disabled, can't update", {event.prpkey});
        return false;
    }

    if (getStore()[event.prpkey]->permission() == OST::Permission::ReadOnly)
    {
        logError("Basemodule::onExternalEventBase - property %1 is readonly, can't update", {event.prpkey});
        return false;
    }
    QJsonObject p = o["p"].toObject();
    QJsonObject e = p[event.prpkey].toObject()["e"].toObject();

    if (event.ev == OST::ExtEvType::SV)
    {
        if (!getStore()[event.prpkey]->getElts()->contains(event.eltkey ))
        {
            logError("Basemodule::onExternalEventBase - element %1-%2 not found", {event.prpkey, event.eltkey});
            return false;
        }
        if (getStore()[event.prpkey]->getElt(event.eltkey)->autoUpdate() || getStore()[event.prpkey]->autoUpdate())
        {
            getStore()[event.prpkey]->setElt(event.eltkey, e.begin().value().toVariant(), true);
        }
    }

    if (event.ev == OST::ExtEvType::SA)
    {
        getStore()[event.prpkey]->setAll(e.toVariantMap());
    }

    /* grid operations */
    if (event.ev == OST::ExtEvType::GC || event.ev == OST::ExtEvType::GU || event.ev == OST::ExtEvType::GF
            || event.ev == OST::ExtEvType::GD || event.ev == OST::ExtEvType::GH || event.ev == OST::ExtEvType::GB
            || event.ev == OST::ExtEvType::GR)
    {
        /* property must have a grid */
        if (!getStore()[event.prpkey]->hasGrid())
        {
            logError("Basemodule::onExternalEventBase - property %1 has no grid", {event.prpkey});
            return false;
        }
        /* these events need a line */
        if (event.line < 0 && (event.ev == OST::ExtEvType::GU || event.ev == OST::ExtEvType::GF || event.ev == OST::ExtEvType::GD
                               || event.ev == OST::ExtEvType::GH || event.ev == OST::ExtEvType::GB))
        {
            logError("Basemodule::onExternalEventBase - missing line value %1", {event.prpkey});
            return false;
        }

        if (getStore()[event.prpkey]->autoUpdate())
        {
            if (event.ev == OST::ExtEvType::GC)
            {
                getStore()[event.prpkey]->newLine(e.toVariantMap());
            }
            if (event.ev == OST::ExtEvType::GU)
            {
                getStore()[event.prpkey]->updateLine(event.line, e.toVariantMap());
            }
            if (event.ev == OST::ExtEvType::GF)
            {
                getStore()[event.prpkey]->fetchLine(event.line);
            }
            if (event.ev == OST::ExtEvType::GD)
            {
                getStore()[event.prpkey]->deleteLine(event.line);
            }
            if (event.ev == OST::ExtEvType::GH)
            {
                getStore()[event.prpkey]->swapLines(event.line, event.line - 1);
            }
            if (event.ev == OST::ExtEvType::GB)
            {
                getStore()[event.prpkey]->swapLines(event.line, event.line + 1);
            }
            if (event.ev == OST::ExtEvType::GR)
            {
                getStore()[event.prpkey]->clearGrid();
            }
        }

    }

    return true;
}
void Basemodule::onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line)
{

}

/**
 * @brief INDI module event handler (Hook 2/3)
 *
 * Empty implementation in Basemodule.
 * Overridden by IndiModule to handle INDI-specific events.
 *
 * Called second in the event chain.
 */
bool Basemodule::onExternalEventIndi(OST::ExtEvent event)
{
    Q_UNUSED(event);
    // Empty by default - IndiModule will override this
    return false;
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

QVariantMap Basemodule::onModuleQuery(const QString &queryName, const QVariantMap &params)
{
    Q_UNUSED(queryName);
    Q_UNUSED(params);
    // Empty by default - custom modules override this to answer the query names they support
    return QVariantMap();
}

void Basemodule::onAfterInit(void)
{
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

void Basemodule::setMinFreePercent(int percent)
{
    mMinFreePercent = percent;
}

int Basemodule::getMinFreePercent() const
{
    return mMinFreePercent;
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

    /* catch profile changes */
    if (prp)
    {
        if (prp->hasProfile() && !this->mCurrentProfileChanged && evt != OST::EvType::ps )
        {
            mCurrentProfileChanged = true;
            emit moduleEvent(OST::EvType::fc, QVariant(), nullptr, nullptr, nullptr, this);

        }
    }
}

bool Basemodule::saveProfile(const QString &pProfileName)
{
    /* build a values only dedicated dump */
    QJsonObject obj = OST::ModuleJsonDumper(OST::EvType::pr, QVariant(), nullptr, nullptr, nullptr, this).toObject();
    /* save it */
    if (this->setDbProfile(this->getClassName(), pProfileName, obj))
    {
        mCurrentProfile = pProfileName;
        mCurrentProfileChanged = false;
        if (!getGlovString("profiles")->getLov().contains(pProfileName))
            getGlovString("profiles")->lovAdd(pProfileName, pProfileName);

        emit moduleEvent(OST::EvType::fs, QVariant(), nullptr, nullptr, nullptr, this);
        return true;
    }
    return false;
}
bool Basemodule::loadProfile(const QString &pProfileName)
{
    QJsonObject obj;
    if (!this->getDbProfile(this->getClassName(), pProfileName, obj)) return false;
    QVariantMap props = obj["p"].toVariant().toMap();
    for(const QString &key : props.keys())
    {
        if (getStore().contains(key))
        {
            QVariantMap data = props[key].toMap();
            if (getStore()[key]->hasProfile())
            {
                if (data.contains("e"))
                {
                    for(const QString &eltkey : props[key].toMap()["e"].toMap().keys())
                    {
                        QVariant v = props[key].toMap()["e"].toMap()[eltkey].toMap()["value"];
                        if (getProperty(key)->getElts()->contains(eltkey))
                        {
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
                            if (getEltBase(key, eltkey)->getType() == "datetime")
                            {
                                int hh = props[key].toMap()["e"].toMap()[eltkey].toMap()["hh"].toInt();
                                int mm = props[key].toMap()["e"].toMap()[eltkey].toMap()["mm"].toInt();
                                int ss = props[key].toMap()["e"].toMap()[eltkey].toMap()["ss"].toInt();
                                int ms = props[key].toMap()["e"].toMap()[eltkey].toMap()["ms"].toInt();
                                int d = props[key].toMap()["e"].toMap()[eltkey].toMap()["day"].toInt();
                                int m = props[key].toMap()["e"].toMap()[eltkey].toMap()["month"].toInt();
                                int y = props[key].toMap()["e"].toMap()[eltkey].toMap()["year"].toInt();
                                QDate dd;
                                QTime tt;
                                QDateTime dt;
                                dd.setDate(y, m, d);
                                tt.setHMS(hh, mm, ss, ms);
                                dt.setDate(dd);
                                dt.setTime(tt);
                                getEltDateTime(key, eltkey)->setValue(dt, true);
                            }
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
                        for(const QVariant &vv : line)
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
                            if (getEltBase(key, eltkey)->getType() == "time")
                            {
                                int hh = vv.toMap()["hh"].toInt();
                                int mm = vv.toMap()["mm"].toInt();
                                int ss = vv.toMap()["ss"].toInt();
                                int ms = vv.toMap()["ms"].toInt();
                                QTime tt;
                                tt.setHMS(hh, mm, ss, ms);
                                getEltTime(key, eltkey)->setValue(tt, true);
                            }
                            if (getEltBase(key, eltkey)->getType() == "date")
                            {
                                int d = vv.toMap()["day"].toInt();
                                int m = vv.toMap()["month"].toInt();
                                int y = vv.toMap()["year"].toInt();
                                QDate dd;
                                dd.setDate(y, m, d);
                                getEltDate(key, eltkey)->setValue(dd, true);
                            }
                            if (getEltBase(key, eltkey)->getType() == "datetime")
                            {
                                int hh = vv.toMap()["hh"].toInt();
                                int mm = vv.toMap()["mm"].toInt();
                                int ss = vv.toMap()["ss"].toInt();
                                int ms = vv.toMap()["ms"].toInt();
                                int d =  vv.toMap()["day"].toInt();
                                int m =  vv.toMap()["month"].toInt();
                                int y =  vv.toMap()["year"].toInt();
                                QDate dd;
                                QTime tt;
                                QDateTime dt;
                                dd.setDate(y, m, d);
                                tt.setHMS(hh, mm, ss, ms);
                                dt.setDate(dd);
                                dt.setTime(tt);
                                getEltDateTime(key, eltkey)->setValue(dt, true);
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

    mCurrentProfile = pProfileName;
    mCurrentProfileChanged = false;
    emit moduleEvent(OST::EvType::fl, QVariant(), nullptr, nullptr, nullptr, this);
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
QString Basemodule::getCurrentProfile()
{
    return mCurrentProfile;
}
bool Basemodule::getCurrentProfileChanged()
{
    return mCurrentProfileChanged;
};
void Basemodule::otherModuleSetValue(QString mod, QString prop, QString elt, QVariant value)
{
    OST::ExtEvent event;
    event.ev = OST::ExtEvType::SV;
    event.mod = mod;
    event.prpkey = prop;
    event.eltkey = elt;
    QJsonObject e, E, p, P, m, M;
    e[elt] = QJsonValue::fromVariant(value);
    E["e"] = e;
    p[prop] = E;
    P["p"] = p;
    m[mod] = P;
    M["m"] = m;
    event.data = M;
    emit interModuleRequest(event);
};
void Basemodule::otherModuleCreateLine(QString mod, QString prop, QVariantMap values)
{
    OST::ExtEvent event;
    event.ev = OST::ExtEvType::GC;
    event.mod = mod;
    event.prpkey = prop;
    QJsonObject  E, p, P, m, M;
    E["e"] = QJsonValue::fromVariant(values);
    p[prop] = E;
    P["p"] = p;
    m[mod] = P;
    M["m"] = m;
    event.data = M;
    emit interModuleRequest(event);
};
void Basemodule::otherModuleRequestPropertyDump(QString mod, QString prop)
{
    OST::ExtEvent event;
    event.ev = OST::ExtEvType::DP;
    event.mod = mod;
    event.prpkey = prop;
    QJsonObject p, P, m, M;
    p[prop] = QJsonObject();
    P["p"] = p;
    m[mod] = P;
    M["m"] = m;
    event.data = M;
    emit interModuleRequest(event);
}
void Basemodule::setGlobalDatastore(Basemodule* gds)
{
    mGlobalDatastore = gds;
}

int Basemodule::findGridRow(const QString &propName, const QString &keyCol, const QString &keyValue)
{
    int size = getGridSize(propName);
    for (int i = 0; i < size; i++)
    {
        if (getString(propName, keyCol, (long)i) == keyValue) return i;
    }
    return -1;
}

int Basemodule::getGridSize(const QString &propName)
{
    auto store = getStore();
    if (!store.contains(propName)) return 0;
    return store[propName]->getGrid().size();
}

QString Basemodule::getGridString(const QString &propName, const QString &colName, int line)
{
    return getString(propName, colName, (long)line);
}

double Basemodule::getGridFloat(const QString &propName, const QString &colName, int line)
{
    return getFloat(propName, colName, (long)line);
}

long Basemodule::getGridInt(const QString &propName, const QString &colName, int line)
{
    return getInt(propName, colName, (long)line);
}

QString Basemodule::getGridString(const QString &propName, const QString &colName, const QString &keyCol,
                                  const QString &keyValue)
{
    int row = findGridRow(propName, keyCol, keyValue);
    if (row < 0) return QString();
    return getGridString(propName, colName, row);
}

double Basemodule::getGridFloat(const QString &propName, const QString &colName, const QString &keyCol,
                                const QString &keyValue)
{
    int row = findGridRow(propName, keyCol, keyValue);
    if (row < 0) return 0.0;
    return getGridFloat(propName, colName, row);
}

long Basemodule::getGridInt(const QString &propName, const QString &colName, const QString &keyCol, const QString &keyValue)
{
    int row = findGridRow(propName, keyCol, keyValue);
    if (row < 0) return 0;
    return getGridInt(propName, colName, row);
}

void Basemodule::otherModuleRequestProfileLoad(QString mod, QString profile)
{
    OST::ExtEvent event;
    //{"PL":{"m":{"seq":{"profile":"default"}}}}
    event.ev = OST::ExtEvType::PL;
    event.mod = mod;
    QJsonObject m, M, p;
    p["profile"] = profile;
    m[mod] = p;
    M["m"] = m;
    event.data = M;
    emit interModuleRequest(event);
};
void Basemodule::otherModuleQuery(QString mod, QString queryName, QVariantMap params)
{
    OST::ExtEvent event;
    //{"QY":{"m":{"seq":{"query":"profileduration","params":{"profile":"default"}}}}}
    event.ev = OST::ExtEvType::QY;
    event.mod = mod;
    QJsonObject m, M, p;
    p["query"] = queryName;
    p["params"] = QJsonObject::fromVariantMap(params);
    m[mod] = p;
    M["m"] = m;
    event.data = M;
    emit interModuleRequest(event);
};
void Basemodule::setStateEvent(OST::State state, QString statedescription, QString event, QString eventdescription, double valNum, int valInt, QString valStr)
{
    QVariantMap m;
    m["state"] = OST::StateToInt(state);
    m["statedescription"] = statedescription;
    m["event"] = event;
    m["eventdescription"] = eventdescription;
    m["val_num"] = valNum;
    m["val_int"] = valInt;
    m["val_str"] = valStr;
    getProperty("signals")->setAll(m);
}
bool Basemodule::giveMeAState()
{
    if (!getStore().contains("signals"))
    {
        OST::PropertyMulti* pm = new OST::PropertyMulti("signals", "State and signals", OST::ReadOnly, "Module", "", "",
                false,
                false);
        createProperty(pm);
    }
    if (!getProperty("signals")->getElts()->contains("state"))
    {
        OST::ElementLight* e = new  OST::ElementLight("state", "Global state", "10", "");
        getProperty("signals")->addElt(e);
    }
    if (!getProperty("signals")->getElts()->contains("statedescription"))
    {
        OST::ElementString* e = new  OST::ElementString("statedescription", "State description", "20", "");
        getProperty("signals")->addElt(e);
    }
    if (!getProperty("signals")->getElts()->contains("event"))
    {
        OST::ElementString* e = new  OST::ElementString("event", "Global event", "30", "");
        getProperty("signals")->addElt(e);
    }
    if (!getProperty("signals")->getElts()->contains("eventdescription"))
    {
        OST::ElementString* e = new  OST::ElementString("eventdescription", "Event description", "40", "");
        getProperty("signals")->addElt(e);
    }
    if (!getProperty("signals")->getElts()->contains("val_num"))
    {
        OST::ElementFloat* e = new OST::ElementFloat("val_num", "Float value", "50", "");
        getProperty("signals")->addElt(e);
    }
    if (!getProperty("signals")->getElts()->contains("val_int"))
    {
        OST::ElementInt* e = new OST::ElementInt("val_int", "Integer value", "60", "");
        getProperty("signals")->addElt(e);
    }
    if (!getProperty("signals")->getElts()->contains("val_str"))
    {
        OST::ElementString* e = new OST::ElementString("val_str", "String value", "70", "");
        getProperty("signals")->addElt(e);
    }

    return true;
}

bool Basemodule::giveMeAParms()
{
    if (!getStore().contains("parms"))
    {
        OST::PropertyMulti* pm = new OST::PropertyMulti("parms", "Parameters", OST::ReadWrite, "Parameters", "", "",
                true,
                false);
        createProperty(pm);
    }

    return true;
}
