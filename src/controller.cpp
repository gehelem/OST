#include "controller.h"
#include <QNetworkInterface>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QHostInfo>
#include <QDomDocument>
#include <QThread>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <sys/statvfs.h>
#include <QRegularExpression>
#include <QSet>
#include "model/element/common.h"
#include "libs/utils/modulejsondumper.h"
/*!
 * ... ...
 */

#ifdef Q_OS_IOS
#define	OS_NAME		"iOS"
#elif defined(Q_OS_MAC)
#define	OS_NAME		"Mac"
#elif defined(Q_OS_ANDROID)
#define	OS_NAME		"Android"
#elif defined(Q_OS_LINUX)
#define	OS_NAME		"Linux"
#elif defined(Q_OS_WIN)
#define	OS_NAME		"Windows"
#elif defined(Q_OS_FREEBSD)
#define	OS_NAME		"FreeBSD"
#else
#define	OS_NAME		"Some OS"
#endif


Controller::Controller(const QString &webroot, const QString &dbpath,
                       const QString &libpath, const QString &conf, const QString &indiserver,
                       const QString &ssl, const QString &sslCert, const QString &sslKey, const QString &lng, const QString &grant,
                       OST::Logger *logger, OST::TranslateManager *translate, const QString &banner, const QString &setAdminPassword,
                       const QString &gitSha, const QString &gitDate, const QString &gitMessage, const QString &gitTag,
                       const int systemWatchInterval, const int minFreePercent)
    :       _webroot(webroot),
            _dbpath(dbpath),
            _libpath(libpath),
            _conf(conf),
            _indiserver(indiserver),
            _lng(lng),
            _grant(grant),
            mLogger(logger),
            mTranslater(translate),
            mBanner(banner),
            mSetAdminPassword(setAdminPassword),
            mSystemWatchInterval(systemWatchInterval),
            mMinFreePercent(minFreePercent)
{
    // Register meta types for queued signal/slot connections
    // Must be done before any connect() calls that use these types
    qRegisterMetaType<OST::LogLevel>("OST::LogLevel");
    qRegisterMetaType<OST::EvType>("OST::EvType");

    QVariantMap g;
    g["Githash"] = gitSha;
    g["Gitdate"] = gitDate;
    g["Gitmessage"] = gitMessage;
    g["Gittag"] = gitTag;
    mControllerData["git"] = g;
    mControllerData["banner"] = mBanner;
    mControllerData["indiserver"] = indiserver;


    startPublish();

    // Configure Logger with TranslateManager
    mLogger->setTranslateManager(mTranslater, _lng);

    // Configure WShandler
    wshandler = new WShandler(this, ssl, sslCert, sslKey, _grant);
    wshandler->setTranslateManager(mTranslater);
    connect(wshandler, &WShandler::externalEvent, this, &Controller::onExternalEvent);
    connect(wshandler, &WShandler::clientEvent, this, &Controller::OnClientEvent);

    // Configure DBManager
    dbmanager = new DBManager();
    dbmanager->dbInit(_dbpath, "controller");
    forceAdminPassword(mSetAdminPassword);
    wshandler->dbmanager = dbmanager;

    // Connect DBManager to log system
    connect(dbmanager, &Basemodule::logSignal, mLogger, &OST::Logger::onLog);
    connect(dbmanager, &Basemodule::logSignal, wshandler, &WShandler::onLog);

    // create controller lovs
    OST::LovString* ls = new OST::LovString("availableconfs");
    createControllerLov("availableconfs", ls);

    ls = new OST::LovString("indiconfigurations");
    createControllerLov("indiconfigurations", ls);

    ls = new OST::LovString("optics");
    createControllerLov("optics", ls);
    ls = new OST::LovString("servers");
    createControllerLov("servers", ls);
    ls = new OST::LovString("equipments");
    createControllerLov("equipments", ls);
    ls = new OST::LovString("locations");
    createControllerLov("locations", ls);

    // Shared datastore — static, always available, created before any module
    mGlobalDatastore = new GlobalDatastore("GlobalDatastore", "Global data", "default", {});
    mGlobalDatastore->setParent(this);
    mGlobalDatastore->dbInit(_dbpath, "GlobalDatastore");
    mGlobalDatastore->loadProfile("default");
    connect(mGlobalDatastore, &Basemodule::moduleEvent,  this,       &Controller::onModuleEvent);
    connect(mGlobalDatastore, &Basemodule::moduleEvent,  wshandler,  &WShandler::onModuleEvent);
    connect(mGlobalDatastore, &Basemodule::logSignal,    mLogger,    &OST::Logger::onLog);
    connect(mGlobalDatastore, &Basemodule::logSignal,    wshandler,  &WShandler::onLog);
    connect(this, &Controller::controllerEvent, mGlobalDatastore, &Basemodule::onExternalEventRoot);
    mGlobalDatastore->onAfterInit();
    mGlobalDatastore->registerLov("optics", "name", "name",
                                  static_cast<OST::LovString*>(mControllerLovs["optics"]));
    mGlobalDatastore->registerLov("servers", "name", "name",
                                  static_cast<OST::LovString*>(mControllerLovs["servers"]));
    mGlobalDatastore->registerLov("equipments", "name", "name",
                                  static_cast<OST::LovString*>(mControllerLovs["equipments"]));
    mGlobalDatastore->registerLov("locations", "name", "name",
                                  static_cast<OST::LovString*>(mControllerLovs["locations"]));

    wshandler->onModuleEvent(OST::EvType::aa, QVariant(), nullptr, nullptr, nullptr, mGlobalDatastore);

    if (_libpath == "")
    {
        //_libpath=QCoreApplication::applicationDirPath();
    }
    else
    {
        QCoreApplication::addLibraryPath(_libpath);
    }
    QCoreApplication::addLibraryPath("/usr/lib/ost");
    QCoreApplication::addLibraryPath("/usr/lib");
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

    checkModules();
    checkIndiDrivers();

    updateControllerData("libraries", _availableModuleLibs);
    {
        QVariantList driverList;
        for (const auto &drv : _indiDrivers)
        {
            QVariantMap m;
            m["label"]  = drv.label;
            m["binary"] = drv.binary;
            m["family"] = drv.family;
            m["mdpd"]   = drv.mdpd;
            driverList.append(m);
        }
        updateControllerData("indidrivers", driverList);
    }

    if (_indiserver != "N")
    {
        this->startIndi();
    }
    // "N": OST doesn't manage an embedded indiserver, so there is no startup
    // or ongoing hook of its own to query the bus from here - onSystemWatch()
    // checks it on every tick instead, same as the rest of its metrics.

    loadConf(_conf);

    //check existing folders
    mSelectedFolder = webroot;
    mFileWatcher.addPath(mSelectedFolder);
    QDir dir(webroot);
    QDirIterator itFolders(webroot, QDirIterator::Subdirectories);
    while (itFolders.hasNext())
    {
        QString d = itFolders.next();
        d.replace(webroot, "");
        if (d.endsWith("/.") || d.endsWith("/.."))
        {
            QString dd = d;
            dd.replace("/..", "");
            dd.replace("/.", "");
            if (!mFoldersList.contains(dd))
            {
                //mFileWatcher.addPath(webroot + dd);
                mFoldersList.append(dd);
            }
        }
    }
    //check existing files
    QDirIterator itFiles(mSelectedFolder, QDirIterator::NoIteratorFlags);
    while (itFiles.hasNext())
    {
        QString d = itFiles.next();
        d.replace(mSelectedFolder, "");
        if (!d.endsWith("/.") && !d.endsWith("/..") && !mFoldersList.contains(d))
        {
            mFilesList.append(d);
        }
    }
    wshandler->processFileEvent("foldersdump", mFoldersList);
    wshandler->processFileEvent("filesdump", mFilesList);

    //watch modifications
    QObject::connect(&mFileWatcher, &QFileSystemWatcher::directoryChanged, this, &Controller::OnFileWatcherEvent);
    QObject::connect(&mFileWatcher, &QFileSystemWatcher::fileChanged, this, &Controller::OnFileChangeEvent);
    QVariantMap r;
    dbmanager->getDbProfiles(r);
    updateControllerData("profiles", r);
    updateProfilesLovs(r);

    if (mSystemWatchInterval > 0)
    {
        connect(&mSystemWatchTimer, &QTimer::timeout, this, &Controller::onSystemWatch);
        mSystemWatchTimer.start(mSystemWatchInterval * 1000);
        mNetTimer.start();
        logInfo("System watch started, interval: %1s", {mSystemWatchInterval});
    }
}


bool Controller::loadModule(QString lib, QString label, QString profile)
{
    QString name = label;
    name.replace(" ", "");

    QList<Basemodule *> mods = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
    for (Basemodule *m : mods)
    {
        if (m->getModuleName() == name)
        {
            logError("Module %1 already loaded - can't load twice", {name});
            return false;
        }
    }

    QString lowerlib = lib.toLower();
    QLibrary library("libost" + lowerlib);
    if (!library.load())
    {
        logError("Error loadin library %1 - %2", {name, library.errorString()});
        return false;
    }

    typedef Basemodule *(*CreateModule)(QString, QString, QString, QVariantMap);
    CreateModule createmodule = (CreateModule)library.resolve("initialize");
    if (!createmodule)
    {
        logError("Could not initialize module from library %1 ", {lowerlib});
        return false;
    }
    Basemodule *mod = createmodule(name, label, profile, _availableModuleLibs);
    if (!mod)
    {
        logError("Could not instanciate module from library %1 ", {lowerlib});
        return false;
    }
    mod->setParent(this);
    mod->setWebroot(_webroot);
    mod->setMinFreePercent(mMinFreePercent);
    mod->setGlobalDatastore(mGlobalDatastore);
    mod->setObjectName(name);
    mod->dbInit(_dbpath, name);
    mod->loadProfile(profile);
    mod->updateProfilesLov();
    QVariantMap profs;
    dbmanager->getDbProfiles(mod->metaObject()->className(), profs);
    connect(mod, &Basemodule::moduleEvent, this, &Controller::onModuleEvent);
    connect(mod, &Basemodule::interModuleRequest, this, &Controller::onInterModuleRequest);
    connect(mod, &Basemodule::moduleEvent, wshandler, &WShandler::onModuleEvent);

    //connect(mod, &Basemodule::loadOtherModule, this, &Controller::loadModule);
    connect(this, &Controller::controllerEvent, mod, &Basemodule::onExternalEventRoot);
    connect(this, &Controller::otherModuleEvent, mod, &Basemodule::onOtherModuleEvent);

    // Connect module to log system
    connect(mod, &Basemodule::logSignal, mLogger, &OST::Logger::onLog);
    connect(mod, &Basemodule::logSignal, wshandler, &WShandler::onLog);

    QList<Basemodule *> othermodules = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
    for (Basemodule *othermodule : othermodules)
    {
        if (othermodule->getModuleName() != mod->getModuleName())
        {
            //connect(othermodule, &Basemodule::moduleStatusRequest, mod, &Basemodule::OnModuleStatusRequest);
            //connect(othermodule, &Basemodule::moduleStatusAnswer, mod, &Basemodule::OnModuleStatusAnswer);
            //connect(mod, &Basemodule::moduleStatusRequest, othermodule, &Basemodule::OnModuleStatusRequest);
            //connect(mod, &Basemodule::moduleStatusAnswer, othermodule, &Basemodule::OnModuleStatusAnswer);
            //connect(othermodule, &Basemodule::moduleEvent, mod, &Basemodule::onOtherModuleEvent);
            //connect(mod, &Basemodule::moduleEvent, othermodule, &Basemodule::onOtherModuleEvent);
            //connect(mod, &Datastore::moduleEvent, othermodule, &Basemodule::OnExternalEvent);
        }
    }

    mod->onAfterInit();
    wshandler->onModuleEvent(OST::EvType::aa, QVariant(), nullptr, nullptr, nullptr, mod);

    // Keyed by module TYPE (the library name, e.g. libostfocus -> "focus"),
    // NOT infos.template: a template is only a display hint ("use this
    // rendering"), entirely optional and orthogonal to what a module
    // actually is - two different libraries can share one generic template
    // (e.g. anything using "default"), and grouping "loaded modules" by
    // template would wrongly merge them. Gilles, 2026-09-26, catching this
    // exact conflation: "on est tombe dans un piege qui casse tout ...
    // c'est sur cette clef [le type] qu'on doit enregistrer les profils ...
    // et pas du template !!!". lowerlib is already the module type here.
    QString lovname = "loadedModules-" + lowerlib;
    if (!mControllerLovs.contains(lovname))
    {
        OST::LovString* ls = new OST::LovString(lovname);
        createControllerLov(lovname, ls);
    }
    OST::LovString* ls = static_cast<OST::LovString*>(mControllerLovs[lovname]);
    if (!ls->contains(name)) ls->lovAdd(name, name);

    // One "family-<name>" lov per family the module declared for itself at
    // init (Basemodule::declareFamily(), called from e.g. IndiModule's
    // defineMeAsGuider()) - lets another module's slot pick any loaded
    // module with a given capability, not just a given type (Gilles,
    // 2026-09-26: Sequencer should be able to use a BlindPEC instance as
    // its guiding slave exactly like a Guider one, since both declare
    // family "guiding").
    for (const QString &family : mod->getFamilies())
    {
        QString flovname = "loadedFamilies-" + family;
        if (!mControllerLovs.contains(flovname))
        {
            OST::LovString* fls = new OST::LovString(flovname);
            createControllerLov(flovname, fls);
        }
        OST::LovString* fls = static_cast<OST::LovString*>(mControllerLovs[flovname]);
        if (!fls->contains(name)) fls->lovAdd(name, name);
    }

    return true;

}
void Controller::loadConf(const QString &pConf)
{
    QVariantMap result;
    OST::LovString* l = static_cast<OST::LovString*>(getControllerLovs()["availableconfs"]);
    l->lovClear();
    OST::LovString* il = static_cast<OST::LovString*>(getControllerLovs()["indiconfigurations"]);
    il->lovClear();

    // List every known configuration regardless of whether pConf itself
    // resolves below, so a missing startup configuration doesn't leave
    // availableconfs/indiconfigurations empty.
    QVariantMap confs;
    dbmanager->getDbConfigurations(confs);
    for(QVariantMap::const_iterator iter = confs.begin(); iter != confs.end(); ++iter)
    {
        l->lovAdd(iter.key(), iter.key());
    }
    updateControllerData("availableconfs", QVariant(confs));

    QVariantMap indiConfs;
    dbmanager->getIndiConfigurations(indiConfs);
    for(QVariantMap::const_iterator iter = indiConfs.begin(); iter != indiConfs.end(); ++iter)
    {
        il->lovAdd(iter.key(), iter.key());
    }
    updateControllerData("indiconfigurations", QVariant(indiConfs));

    if (!dbmanager->getDbConfiguration(pConf, result))
    {
        logError("Load configuration %1 failed", {pConf});
        return;
    }
    if (result.size() == 0)
    {
        logError("Configuration %1 not found", {pConf});
        return;
    }
    for(QVariantMap::const_iterator iter = result.begin(); iter != result.end(); ++iter)
    {
        QVariantMap line = iter.value().toMap();
        QString namewithoutblanks = iter.key();
        namewithoutblanks.replace(" ", "");
        loadModule(line["moduletype"].toString(), iter.key(), line["profilename"].toString());
    }
    logInfo("Load configuration %1 sucessfull", {pConf});

    loadIndiConfiguration(pConf);

    updateControllerData("currentconf", QVariant(pConf));
}
// Starts every INDI driver saved under pConf (INDICONFIGURATIONS), resolving
// each saved driver label against the XML registry (_indiDrivers). Also
// usable standalone (YK), independent of loading a full module
// configuration.
void Controller::loadIndiConfiguration(const QString &pConf)
{
    QStringList driverLabels;
    if (!dbmanager->getIndiConfiguration(pConf, driverLabels)) return;
    for (const QString &label : driverLabels)
    {
        auto it = std::find_if(_indiDrivers.begin(), _indiDrivers.end(),
                               [&](const IndiDriverInfo & d)
        {
            return d.label == label;
        });
        if (it != _indiDrivers.end())
            startIndiDriver(it->binary);
        else
            logError("INDI driver not found in XML registry: %1", {label});
    }
}
void Controller::saveConf(const QString &pConf)
{
    OST::LovString* l = static_cast<OST::LovString*>(getControllerLovs()["availableconfs"]);
    l->lovClear();
    OST::LovString* il = static_cast<OST::LovString*>(getControllerLovs()["indiconfigurations"]);
    il->lovClear();
    QVariantMap ms, result;
    QList<Basemodule *> mods = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
    for (Basemodule *m : mods)
    {
        if (m->getModuleName() != "GlobalDatastore")
        {
            QVariantMap n;
            n["label"] = m->getModuleLabel();
            n["profile"] = m->getCurrentProfile();
            n["type"] = m->getClassName();
            ms[m->getModuleName()] = n;
        }
    }
    if (!dbmanager->saveDbConfiguration(pConf, ms))
    {
        logError("Save configuration %1 failed", {pConf});
        return;
    }
    logInfo("Save configuration %1 sucessfull", {pConf});

    QStringList driverLabels;
    for (const IndiDriverInfo &drv : _activeIndiDrivers)
        driverLabels.append(drv.label);
    if (!dbmanager->saveIndiConfiguration(pConf, driverLabels))
        logError("Save INDI configuration %1 failed", {pConf});

    updateControllerData("currentconf", QVariant(pConf));
    QVariantMap confs;
    dbmanager->getDbConfigurations(confs);
    for(QVariantMap::const_iterator iter = confs.begin(); iter != confs.end(); ++iter)
    {
        l->lovAdd(iter.key(), iter.key());
    }
    updateControllerData("availableconfs", QVariant(confs));

    QVariantMap indiConfs;
    dbmanager->getIndiConfigurations(indiConfs);
    for(QVariantMap::const_iterator iter = indiConfs.begin(); iter != indiConfs.end(); ++iter)
    {
        il->lovAdd(iter.key(), iter.key());
    }
    updateControllerData("indiconfigurations", QVariant(indiConfs));
}

bool Controller::createControllerLov(const QString &lovName, OST::LovBase* pLov)
{
    if (mControllerLovs.contains(lovName))
    {
        logError("createControllerLov - lov %1 already exists", {lovName});
        return false;
    }
    pLov->setKey(lovName);
    mControllerLovs[lovName] = pLov;
    connect(mControllerLovs[lovName], &OST::LovBase::lovChanged, this, &Controller::onControllerLovChanged);
    wshandler->onControllerEvent(OST::EvType::lc, QString(), QString(), pLov);
    return true;
}

bool Controller::deleteControllerLov(const QString &lovName)
{
    if (!mControllerLovs.contains(lovName))
    {
        logError("deleteControllerLov - lov %1 not found", {lovName});
        return false;
    }
    OST::LovBase* pLov = mControllerLovs[lovName];
    disconnect(pLov, &OST::LovBase::lovChanged, this, &Controller::onControllerLovChanged);
    mControllerLovs.remove(lovName);
    wshandler->onControllerEvent(OST::EvType::ld, lovName, QString(), nullptr);
    delete pLov;
    return true;
}

// Builds/refreshes one "profiles-<Type>" controller lov per key of `r`
// (the same QVariantMap already used for controllerdata.profiles, see both
// call sites) — <Type> keeps the exact case DBManager::getDbProfiles()
// returns (the module class name), matching every real listOfValues
// reference (e.g. sequencer.json's "profiles-Focus") and Rooster's own
// rebuildGlobalLovs(). Previously never implemented server-side at all —
// controllerdata.profiles carried the data, but no client-facing lov
// exposed it generically.
void Controller::updateProfilesLovs(const QVariantMap &r)
{
    for (QVariantMap::const_iterator iter = r.begin(); iter != r.end(); ++iter)
    {
        QString lovname = "profiles-" + iter.key();
        if (!mControllerLovs.contains(lovname))
        {
            OST::LovString* ls = new OST::LovString(lovname);
            createControllerLov(lovname, ls);
        }
        OST::LovString* ls = static_cast<OST::LovString*>(mControllerLovs[lovname]);
        ls->lovClear();
        for (const QVariant &name : iter.value().toList())
        {
            ls->lovAdd(name.toString(), name.toString());
        }
    }
}

void Controller::onControllerLovChanged()
{
    OST::LovBase* lov = qobject_cast<OST::LovBase*>(sender());
    wshandler->onControllerEvent(OST::EvType::lu, QString(), QString(), lov);
}

void Controller::onModuleEvent(OST::EvType evt, QVariant data, OST::ElementBase* elt, OST::PropertyBase* prp,
                               OST::LovBase* lov,
                               Datastore* mod)

{

    /* refresh global profiles data when a module gives en profile event */
    if (evt == OST::EvType::fs)
    {
        QVariantMap r;
        dbmanager->getDbProfiles(r);
        updateControllerData("profiles", r);
        updateProfilesLovs(r);
    }

    /* dispatch event to other modules */
    QString m, p, e;
    int line = 0;
    QJsonValue v;
    if (mod) m = mod->getModuleName();
    if (prp) p = prp->key();
    if (elt) e = elt->key();
    if (evt == OST::EvType::gu || evt == OST::EvType::gu  )
    {
        line = data.toInt();
    }

    switch (evt)
    {
        case OST::EvType::dm: // "dm-delete/remove module"
        {
            v = m;
            break;
        }
        case OST::EvType::ps: // "ps-only property state"
        {
            v = static_cast<int>(prp->state());
            break;
        }
        case OST::EvType::dp: // "dp-delete/remove property"
        {
            p = data.toString();
            break;
        }
        case OST::EvType::de: // "de-delete/remove element"
        case OST::EvType::ee: // "ee"
        case OST::EvType::gc: // "gc-grid new line "
        case OST::EvType::gu: // "gu-grid update line"
        case OST::EvType::gd: // "gd-grid delete line"
        case OST::EvType::gr: // "gr-grid reset"
        case OST::EvType::aa: // "aa-dump all data"
        case OST::EvType::ev: // "ev-set one element value/min/max/format "
        case OST::EvType::el: // "el-one element's own local lov changed"
        case OST::EvType::ap: // "ap-dump all property data"
        case OST::EvType::ea: // "ea"
        {
            OST::PropertyJsonDumper d(evt, data, elt, prp);
            mod->getStore()[prp->key()]->accept(&d);
            v = d.getResult();
            break;
        }
        case OST::EvType::zz: // "zz-no dump"
        case OST::EvType::av: // "av-dump profile data"
        case OST::EvType::lc: // "lc-lov create"
        case OST::EvType::lu: // "lu-lov update"
        case OST::EvType::ld: // "ld-lov delete"
        case OST::EvType::am: // "am-add module"
        case OST::EvType::fs: // "fs-profile saved"
        case OST::EvType::fl: // "fl-profile loaded"
        case OST::EvType::fc: // "fc-profile changed"
        case OST::EvType::uc: // "uc-update controller data"
        case OST::EvType::qa: // "qa-query answer"
        {
            v = OST::ModuleJsonDumper(evt, data, elt, prp, lov, static_cast<Basemodule*>(mod));
            break;
        }
        default:
        {
            v = OST::ModuleJsonDumper(evt, data, elt, prp, lov, static_cast<Basemodule*>(mod));
        }
    };

    emit otherModuleEvent(evt, m, p, e, v, line);
}
void Controller::OnClientEvent(OST::ExtEvent event, QWebSocket* client, QString clientgrant)
{
    if (event.ev == OST::ExtEvType::DU || event.ev == OST::ExtEvType::IL  || event.ev == OST::ExtEvType::LO )
    {
        wshandler->sendJsonMessage(getModulesDump(clientgrant, wshandler->getClientLanguage(client)), client);
    }
    if (event.ev == OST::ExtEvType::XX )
    {
        QJsonObject o;
        o[EvToString(OST::EvType::xx)] = QJsonObject();
        wshandler->sendJsonMessage(o, client);
    }

}

void Controller::onExternalEvent(OST::ExtEvent event)
{
    switch (event.ev)
    {
        case OST::ExtEvType::ZZ:
        case OST::ExtEvType::DU:
        case OST::ExtEvType::LO:
        case OST::ExtEvType::IL:
        {
            logError("Controller::onExternalEvent - invalid event here - %1", {OST::ExtEvToString(event.ev)});
            return;
        }
        case OST::ExtEvType::CS:
        {
            saveConf(event.data["name"].toString());
            return;
        }
        case OST::ExtEvType::CL:
        {
            loadConf(event.data["name"].toString());
            return;
        }
        case OST::ExtEvType::ML:
        {
            loadModule(event.data["lib"].toString(), event.data["label"].toString(), event.data["profile"].toString());
            return;
        }
        case OST::ExtEvType::MK:
        {
            QList<Basemodule *> mods = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
            QString n =  event.data["name"].toString();
            if (n == mGlobalDatastore->getModuleName())
            {
                logError("GlobalDatastore cannot be killed", {});
                return;
            }
            for (Basemodule *m : mods)
            {
                if (m->getModuleName() == n)
                {
                    // Module TYPE (library name), not infos.template - see
                    // the matching note in loadModule() above.
                    QString lovname = "loadedModules-" + m->getClassName().toLower();
                    if (mControllerLovs.contains(lovname))
                    {
                        OST::LovString* ls = static_cast<OST::LovString*>(mControllerLovs[lovname]);
                        if (ls->contains(n)) ls->lovDel(n);
                    }
                    for (const QString &family : m->getFamilies())
                    {
                        QString flovname = "loadedFamilies-" + family;
                        if (mControllerLovs.contains(flovname))
                        {
                            OST::LovString* fls = static_cast<OST::LovString*>(mControllerLovs[flovname]);
                            if (fls->contains(n)) fls->lovDel(n);
                        }
                    }
                    m->killMe();
                };
            }


            return;

        }
        case OST::ExtEvType::FS:
        {
            if (!event.data.contains("folder"))
            {
                logError("Controller::onExternalEvent - invalid event data content - %1", {OST::ExtEvToString(event.ev)});
                return;
            };
            QFileInfo requestedInfo(_webroot + event.data["folder"].toString());
            if (!requestedInfo.isDir())
            {
                logError("Controller::onExternalEvent - folder %1 does not exist", {event.data["folder"].toString()});
                return;
            }
            QString canonicalWebroot = QFileInfo(_webroot).canonicalFilePath();
            QString canonicalRequested = requestedInfo.canonicalFilePath();
            if (canonicalRequested != canonicalWebroot && !canonicalRequested.startsWith(canonicalWebroot + "/"))
            {
                logError("Controller::onExternalEvent - folder %1 is outside the webroot", {event.data["folder"].toString()});
                return;
            }
            // The webroot itself stays watched permanently (folder tree
            // visibility is global, see OnFileWatcherEvent) - only the
            // previously selected folder's own watch, if different, is
            // swapped out for the newly selected one.
            if (mSelectedFolder != canonicalWebroot && mFileWatcher.directories().contains(mSelectedFolder))
            {
                mFileWatcher.removePath(mSelectedFolder);
            }
            if (canonicalRequested != canonicalWebroot && !mFileWatcher.directories().contains(canonicalRequested))
            {
                mFileWatcher.addPath(canonicalRequested);
            }
            selectFolder(canonicalRequested);
            return;
        }
        case OST::ExtEvType::YA:
        {
            startIndi();
            return;
        }
        case OST::ExtEvType::YZ:
        {
            stopIndi();
            return;
        }
        case OST::ExtEvType::YL:
        {
            if (!event.data.contains("driver"))
            {
                logError("Controller::onExternalEvent - invalid event data content - %1", {OST::ExtEvToString(event.ev)});
                return;
            };
            startIndiDriver(event.data["driver"].toString());
            return;
        }
        case OST::ExtEvType::YS:
        {
            if (!event.data.contains("driver"))
            {
                logError("Controller::onExternalEvent - invalid event data content - %1", {OST::ExtEvToString(event.ev)});
                return;
            };
            stopIndiDriver(event.data["driver"].toString());
            return;
        }
        case OST::ExtEvType::YR:
        {
            if (!event.data.contains("driver"))
            {
                logError("Controller::onExternalEvent - invalid event data content - %1", {OST::ExtEvToString(event.ev)});
                return;
            };
            reloadIndiDriver(event.data["driver"].toString());
            return;
        }
        case OST::ExtEvType::YC:
        {
            if (!event.data.contains("driver"))
            {
                logError("Controller::onExternalEvent - invalid event data content - %1", {OST::ExtEvToString(event.ev)});
                return;
            };
            connectIndiDriver(event.data["driver"].toString());
            return;
        }
        case OST::ExtEvType::YD:
        {
            if (!event.data.contains("driver"))
            {
                logError("Controller::onExternalEvent - invalid event data content - %1", {OST::ExtEvToString(event.ev)});
                return;
            };
            disconnectIndiDriver(event.data["driver"].toString());
            return;
        }
        case OST::ExtEvType::YK:
        {
            loadIndiConfiguration(event.data["name"].toString());
            return;
        }
        default:
        {
            if (!event.data.contains("m"))
            {
                logError("Controller::onExternalEvent - invalid event data content - %1", {OST::ExtEvToString(event.ev)});
                return;
            };
            if (event.data["m"].toObject().size() != 1)
            {
                logError("Controller::onExternalEvent - invalid event data size - %1", {OST::ExtEvToString(event.ev)});
                return;
            };
            QList<Basemodule *> mods = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
            bool r = false;
            for (Basemodule *m : mods)
            {
                if (m->getModuleName() == event.data["m"].toObject().begin().key()) r = true;
            }
            if (!r)
            {
                logError("Controller::onExternalEvent - unknown module - %1", {event.data["m"].toObject().begin().key()});
                return;
            };
        }

    }

    /* basic data validity checks below */
    QString mod = event.data["m"].toObject().begin().key();
    QJsonObject o = event.data["m"].toObject()[mod].toObject();
    QJsonObject p, e;
    event.line = -1;

    /* data must contain a unique "p" key for these */
    if (event.ev == OST::ExtEvType::SV || event.ev == OST::ExtEvType::SA || event.ev == OST::ExtEvType::GC
            || event.ev == OST::ExtEvType::I1 || event.ev == OST::ExtEvType::I2 || event.ev == OST::ExtEvType::I3
            || event.ev == OST::ExtEvType::I4 || event.ev == OST::ExtEvType::J1 || event.ev == OST::ExtEvType::J2
            || event.ev == OST::ExtEvType::GU || event.ev == OST::ExtEvType::GF || event.ev == OST::ExtEvType::GD
            || event.ev == OST::ExtEvType::GR || event.ev == OST::ExtEvType::GH || event.ev == OST::ExtEvType::GB)
    {
        if (!o.contains("p"))
        {
            logError("Controller::onExternalEvent - invalid event data content - data must contain a 'p' key %1", {OST::ExtEvToString(event.ev)});
            return;
        }
        p = o["p"].toObject();
        if (o["p"].toObject().size() != 1)
        {
            logError("Controller::onExternalEvent - invalid event data content - 'p' key must be unique %1", {OST::ExtEvToString(event.ev)});
            return;
        }
        event.prpkey = o["p"].toObject().begin().key();
    }

    /* data must contain a "e" key for these */
    if (event.ev == OST::ExtEvType::SV || event.ev == OST::ExtEvType::SA || event.ev == OST::ExtEvType::GC
            || event.ev == OST::ExtEvType::J1 || event.ev == OST::ExtEvType::J2
            || event.ev == OST::ExtEvType::GU)
    {
        if (!p[event.prpkey].toObject().contains("e"))
        {
            logError("Controller::onExternalEvent - invalid event data content - data must contain a 'e' key %1", {OST::ExtEvToString(event.ev)});
            return;
        }
        e = p[event.prpkey].toObject()["e"].toObject();
    }

    /* "e" key must be unique for these */
    if (event.ev == OST::ExtEvType::SV || event.ev == OST::ExtEvType::J1 || event.ev == OST::ExtEvType::J2)
    {
        if (e.size() != 1)
        {
            logError("Controller::onExternalEvent - invalid event data content size - 'e' key must be unique %1", {OST::ExtEvToString(event.ev)});
            return;
        }
        event.eltkey = e.begin().key();
    }

    /* data must contain a "i" key for these */
    if (event.ev == OST::ExtEvType::GU || event.ev == OST::ExtEvType::GF || event.ev == OST::ExtEvType::GD
            || event.ev == OST::ExtEvType::GH || event.ev == OST::ExtEvType::GB)
    {
        if (!p[event.prpkey].toObject().contains("i"))
        {
            logError("Controller::onExternalEvent - invalid event data content - data must contain a 'i' key  %1", {OST::ExtEvToString(event.ev)});
            return;
        }
        event.line = p[event.prpkey].toObject()["i"].toInt();
    }

    /* checks ok : dispatch event only to targeted module */
    QList<Basemodule *> moduleinstances = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
    for (Basemodule *moduleinstance : moduleinstances)
    {
        if (moduleinstance->getModuleName() == mod)
        {
            moduleinstance->onExternalEventRoot(event);
        }
    }



}

void Controller::checkModules(void)
{
    for(const QString &path : QCoreApplication::libraryPaths())
    {
        mLogger->info("Check available modules in " + path);
        QDir directory(path);
        directory.setFilter(QDir::Files);
        directory.setNameFilters(QStringList() << "libost*.so");
        QStringList libs = directory.entryList();
        for(QString lib : libs)
        {
            QString tt = lib.replace(".so", "");
            if (!((tt == "libostmaincontrol" ) || (tt == "libostbasemodule" ) || (tt == "libostindimodule" )))
            {
                QLibrary library(path + "/" + lib);
                if (!library.load())
                {
                    mLogger->info(lib + " " + library.errorString());
                }
                else
                {
                    typedef Basemodule *(*CreateModule)(QString, QString, QString, QVariantMap);
                    CreateModule createmodule = (CreateModule)library.resolve("initialize");


                    if (createmodule)
                    {
                        Basemodule *mod = createmodule(tt, tt, QString(), QVariantMap());
                        if (mod)
                        {
                            mod->setParent(this);
                            mod->setObjectName(lib);
                            QVariantMap info = mod->getModuleInfo();
                            _availableModuleLibs[tt] = info;
                            mLogger->info("found library " + path + "/" + lib) ;
                            delete mod;
                        }
                    }
                    else
                    {
                        mLogger->warning("Could not initialize module from the loaded library : " + lib);
                    }
                }

            }


        }
    }


}
void Controller::checkIndiDrivers(void)
{
    const QString path = "/usr/share/indi";
    mLogger->info("Scanning INDI drivers in " + path);
    _indiDrivers.clear();

    QDir directory(path);
    if (!directory.exists())
    {
        mLogger->warning("INDI driver directory not found: " + path);
        return;
    }
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList() << "*.xml");

    for (const QString &fname : directory.entryList())
    {
        if (fname.contains("_sk"))
            continue;

        QFile file(path + "/" + fname);
        if (!file.open(QIODevice::ReadOnly))
        {
            mLogger->warning("Cannot open INDI driver file: " + fname);
            continue;
        }

        QDomDocument doc;
        QString errMsg;
        int errLine = 0;
        if (!doc.setContent(&file, &errMsg, &errLine))
        {
            mLogger->warning("XML parse error in " + fname + " line " + QString::number(errLine) + ": " + errMsg);
            file.close();
            continue;
        }
        file.close();

        QDomNodeList groups = doc.documentElement().elementsByTagName("devGroup");
        for (int i = 0; i < groups.count(); ++i)
        {
            QDomElement group = groups.at(i).toElement();
            QString family = group.attribute("group");

            QDomNodeList devices = group.elementsByTagName("device");
            for (int j = 0; j < devices.count(); ++j)
            {
                QDomElement device = devices.at(j).toElement();
                QString label    = device.attribute("label");
                QString skelAttr = device.attribute("skel");
                bool    mdpd     = (device.attribute("mdpd") == "true");

                QDomElement driverElt = device.firstChildElement("driver");
                if (driverElt.isNull()) continue;
                QString binary = driverElt.text().trimmed();

                if (label.isEmpty() || binary.isEmpty()) continue;

                QString skeleton;
                if (!skelAttr.isEmpty())
                    skeleton = skelAttr.startsWith("/") ? skelAttr : path + "/" + skelAttr;

                IndiDriverInfo info;
                info.label    = label;
                info.binary   = binary;
                info.family   = family;
                info.skeleton = skeleton;
                info.mdpd     = mdpd;
                _indiDrivers.append(info);
            }
        }
    }

    std::sort(_indiDrivers.begin(), _indiDrivers.end(),
              [](const IndiDriverInfo & a, const IndiDriverInfo & b)
    {
        return a.label < b.label;
    });

    mLogger->info("Found " + QString::number(_indiDrivers.size()) + " INDI drivers");
}
void Controller::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit)
        mLogger->error("indiserver crashed");
    else
        mLogger->warning("indiserver exited (code " + QString::number(exitCode) + ")");
    setIndiServerState(false);
}
// "Y" mode also carries live state: "U" once indiserver is actually
// running, "D" once it isn't. "N" mode is left untouched.
void Controller::setIndiServerState(bool running)
{
    if (_indiserver != "Y") return;
    updateControllerData("indiserver", QVariant(running ? QString("U") : QString("D")));
}
void Controller::processOutput()
{
    QString output = _process->readAllStandardOutput();
    //pMainControl->sendMainMessage("PROCESS LOG   : " + output);
}
void Controller::processError()
{
    QString output = _process->readAllStandardError();
    //pMainControl->sendMainError("PROCESS ERROR   : " + output);
}
void Controller::processIndiOutput()
{
    for (const QString &line : QString(_indiProcess->readAllStandardOutput()).split('\n'))
        if (!line.trimmed().isEmpty())
            mLogger->info("[indiserver] " + line.trimmed());
}
void Controller::processIndiError()
{
    for (const QString &line : QString(_indiProcess->readAllStandardError()).split('\n'))
        if (!line.trimmed().isEmpty())
            mLogger->warning("[indiserver] " + line.trimmed());
}
// ---------- ZeroConf ----------

void Controller::startPublish()
{
    zeroConf.clearServiceTxtRecords();
    zeroConf.addServiceTxtRecord("OstServer", "Observatoire Sans Tete");
    zeroConf.startServicePublish(QHostInfo::localHostName().toUtf8(), "_ostserver_ws._tcp", "local", 9624);
}
void Controller::startIndi(void)
{
    // Clean up our own QProcess if still alive
    if (_indiProcess)
    {
        if (_indiProcess->state() != QProcess::NotRunning)
        {
            _indiProcess->kill();
            _indiProcess->waitForFinished(3000);
        }
        delete _indiProcess;
        _indiProcess = nullptr;
    }
    _indiPid = 0;

    // Check for an orphaned indiserver (ostserver crashed, indiserver survived)
    QProcess pgrep;
    pgrep.start("pgrep", {"-x", "indiserver"});
    pgrep.waitForFinished(1000);
    QString pidStr = pgrep.readAllStandardOutput().trimmed().split('\n').first();
    if (!pidStr.isEmpty())
    {
        int fd = ::open("/tmp/ostserverIndiFIFO", O_WRONLY | O_NONBLOCK);
        if (fd >= 0)
        {
            ::close(fd);
            _indiPid = pidStr.toInt();
            mLogger->info("Reusing orphaned indiserver (PID " + pidStr + ")");
            queryActiveIndiDrivers();
            setIndiServerState(true);
            return;
        }
        mLogger->warning("Orphaned indiserver found but FIFO unusable — restarting");
        ::kill(pidStr.toInt(), SIGKILL);
        QThread::msleep(300);
    }

    QProcess::execute("rm",    {"-f", "/tmp/ostserverIndiFIFO"});
    QProcess::execute("mkfifo", {"/tmp/ostserverIndiFIFO"});

    _indiProcess = new QProcess(this);
    connect(_indiProcess, &QProcess::readyReadStandardOutput, this, &Controller::processIndiOutput);
    connect(_indiProcess, &QProcess::readyReadStandardError,  this, &Controller::processIndiError);
    connect(_indiProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Controller::processFinished);

    _indiProcess->start("indiserver", {"-f", "/tmp/ostserverIndiFIFO", "-m", "1000"});

    if (!_indiProcess->waitForStarted(3000))
    {
        mLogger->error("indiserver failed to start");
        setIndiServerState(false);
        return;
    }
    _indiPid = _indiProcess->processId();
    mLogger->info("indiserver started (PID " + QString::number(_indiPid) + ")");
    setIndiServerState(true);

    bool fifoReady = false;
    for (int i = 0; i < 30 && !fifoReady; ++i)
    {
        QThread::msleep(100);
        int fd = ::open("/tmp/ostserverIndiFIFO", O_WRONLY | O_NONBLOCK);
        if (fd >= 0)
        {
            ::close(fd);
            fifoReady = true;
        }
    }
    if (!fifoReady)
        mLogger->warning("indiserver FIFO not ready after 3 seconds");
    else
        mLogger->info("indiserver FIFO ready");
}
void Controller::stopIndi(void)
{
    if (_indiProcess && _indiProcess->state() != QProcess::NotRunning)
    {
        _indiProcess->kill();
        _indiProcess->waitForFinished(3000);
        mLogger->info("indiserver stopped");
    }
    else if (_indiPid > 0)
    {
        ::kill(_indiPid, SIGKILL);
        mLogger->info("Orphaned indiserver stopped (PID " + QString::number(_indiPid) + ")");
    }
    setIndiServerState(false);
    _indiPid = 0;
    _activeIndiDrivers.clear();
    updateControllerData("indiactivedrivers", QVariantList());
}
void Controller::startIndiDriver(const QString &pDriver)
{
    if (std::find_if(_activeIndiDrivers.begin(), _activeIndiDrivers.end(),
                     [&](const IndiDriverInfo & d)
{
    return d.binary == pDriver;
}) != _activeIndiDrivers.end())
    {
        mLogger->info("Driver already active, skipping: " + pDriver);
        return;
    }

    int fd = ::open("/tmp/ostserverIndiFIFO", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        mLogger->error("Cannot start driver " + pDriver + ": FIFO not available (" + QString::fromLocal8Bit(strerror(errno)) + ")");
        return;
    }
    QByteArray cmd = ("start " + pDriver + "\n").toLocal8Bit();
    ::write(fd, cmd.constData(), cmd.size());
    ::close(fd);
    mLogger->info("Sent start command for driver: " + pDriver);

    QList<IndiDriverInfo> candidates;
    for (const IndiDriverInfo &d : _indiDrivers)
        if (d.binary == pDriver) candidates.append(d);

    if (candidates.isEmpty())
    {
        mLogger->warning("Started driver not found in XML registry: " + pDriver);
        return;
    }

    if (candidates.size() == 1)
    {
        // The common, unambiguous case: this binary maps to exactly one
        // catalog entry, resolved and published right away.
        _activeIndiDrivers.append(candidates.first());
        updateControllerData("indiactivedrivers", activeIndiDriversToVariant());
        return;
    }

    // Some mdpd ("multiple devices per driver") drivers register several
    // catalog device entries under the SAME binary - confirmed live:
    // indi_qhy.xml lists both "QHY CCD" and "Orion SSAG" for
    // indi_qhy_ccd. The catalog alone can't tell which of them the server
    // is actually about to run (picking the first match would be an
    // alphabetical accident, not a real answer - this is exactly how
    // starting "QHY CCD" ended up reported as "Orion SSAG"), so the right
    // entry is only resolved after the fact, from the live bus (same
    // technique as queryActiveIndiDrivers(), via DRIVER_INFO.DRIVER_EXEC -
    // available as soon as the driver registers, no need to wait for an
    // actual device connection). A 500ms delay, same as reloadIndiDriver()
    // already uses, gives it time to show up.
    QTimer::singleShot(500, this, [this, pDriver]()
    {
        bool resolved = false;
        for (const QString &device : resolveIndiDeviceNames(pDriver))
        {
            const IndiDriverInfo *best = bestCatalogMatchForLiveName(device);
            if (!best) continue;
            bool alreadyThere = std::find_if(_activeIndiDrivers.begin(), _activeIndiDrivers.end(),
                                              [&](const IndiDriverInfo & d)
            {
                return d.binary == best->binary && d.label == best->label;
            }) != _activeIndiDrivers.end();
            if (!alreadyThere)
            {
                _activeIndiDrivers.append(*best);
                mLogger->info("Resolved started driver " + pDriver + " as live device " + device + " -> catalog entry " + best->label);
                resolved = true;
            }
        }
        if (resolved)
            updateControllerData("indiactivedrivers", activeIndiDriversToVariant());
        else
            mLogger->warning("Could not resolve which catalog entry driver " + pDriver + " actually started as");
    });
}
void Controller::stopIndiDriver(const QString &pDriver)
{
    int fd = ::open("/tmp/ostserverIndiFIFO", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        mLogger->error("Cannot stop driver " + pDriver + ": FIFO not available (" + QString::fromLocal8Bit(strerror(errno)) + ")");
        return;
    }
    QByteArray cmd = ("stop " + pDriver + "\n").toLocal8Bit();
    ::write(fd, cmd.constData(), cmd.size());
    ::close(fd);

    _activeIndiDrivers.erase(
        std::remove_if(_activeIndiDrivers.begin(), _activeIndiDrivers.end(),
                       [&](const IndiDriverInfo & d)
    {
        return d.binary == pDriver;
    }),
    _activeIndiDrivers.end());

    mLogger->info("Sent stop command for driver: " + pDriver);
    updateControllerData("indiactivedrivers", activeIndiDriversToVariant());
}
void Controller::reloadIndiDriver(const QString &pDriver)
{
    stopIndiDriver(pDriver);
    QTimer::singleShot(500, this, [this, pDriver]()
    {
        startIndiDriver(pDriver);
    });
    mLogger->info("Reload requested for driver: " + pDriver);
}

// Resolves a driver binary (what YL/YR/YS/YC/YD identify a driver by, same
// as controllerdata.indidrivers' own "binary" field) to the live INDI
// device name(s) it is actually running under on the bus right now - not
// the same string for mdpd ("multiple devices per driver") drivers, which
// rename themselves to "<catalog label> <model>" once connected to real
// hardware (see queryActiveIndiDrivers()'s own comment for the confirmed
// example: indi_qhy_ccd → "QHY CCD QHYminiCam8M"). Queried live via
// DRIVER_INFO.DRIVER_EXEC rather than reused from _activeIndiDrivers,
// which only ever stores the generic catalog label, not the live name
// CONNECTION.CONNECT/DISCONNECT actually needs. A driver can in principle
// expose more than one device at once (mdpd), hence a list.
QStringList Controller::resolveIndiDeviceNames(const QString &pDriver)
{
    QStringList names;

    QProcess proc;
    proc.start("indi_getprop", {"-t", "2", "*.DRIVER_INFO.DRIVER_EXEC"});
    proc.waitForFinished(4000);

    for (const QString &line : QString(proc.readAllStandardOutput()).split('\n'))
    {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;

        int eq = trimmed.indexOf('=');
        if (eq < 0) continue;
        if (trimmed.mid(eq + 1).trimmed() != pDriver) continue;

        QString deviceName = trimmed.left(eq).section('.', 0, 0);
        if (!deviceName.isEmpty()) names << deviceName;
    }

    return names;
}

void Controller::connectIndiDriver(const QString &pDriver)
{
    const QStringList devices = resolveIndiDeviceNames(pDriver);
    if (devices.isEmpty())
    {
        mLogger->warning("Cannot connect " + pDriver + ": no live INDI device found for it");
        return;
    }
    for (const QString &device : devices)
    {
        QProcess::execute("indi_setprop", {"-t", "2", device + ".CONNECTION.CONNECT=On"});
        mLogger->info("Sent connect command for device: " + device);
    }
}

void Controller::disconnectIndiDriver(const QString &pDriver)
{
    const QStringList devices = resolveIndiDeviceNames(pDriver);
    if (devices.isEmpty())
    {
        mLogger->warning("Cannot disconnect " + pDriver + ": no live INDI device found for it");
        return;
    }
    for (const QString &device : devices)
    {
        QProcess::execute("indi_setprop", {"-t", "2", device + ".CONNECTION.DISCONNECT=On"});
        mLogger->info("Sent disconnect command for device: " + device);
    }
}

// mdpd ("multiple devices per driver") drivers rename themselves at
// runtime to "<catalog label> <model>" once they know which actual
// hardware they're talking to - confirmed live: indi_qhy_ccd connected to
// a real camera reports itself on the bus as "QHY CCD QHYminiCam8M", not
// the XML catalog's generic "QHY CCD" label (indi_qhy.xml: <device
// label="QHY CCD" mdpd="true">). An exact match only ever catches
// non-mdpd drivers (simulators, most single-model devices). Picks the
// longest-matching catalog label among candidates, to stay unambiguous if
// two mdpd families' generic labels could otherwise both prefix-match the
// same live device name. Shared by queryActiveIndiDrivers() (orphaned
// indiserver recovery) and startIndiDriver()'s own delayed resolution for
// an ambiguous binary (see there).
const IndiDriverInfo *Controller::bestCatalogMatchForLiveName(const QString &deviceLabel)
{
    const IndiDriverInfo *best = nullptr;
    for (const IndiDriverInfo &d : _indiDrivers)
    {
        bool match = (d.label == deviceLabel) ||
                     (d.mdpd && deviceLabel.startsWith(d.label + " "));
        if (match && (!best || d.label.length() > best->label.length()))
            best = &d;
    }
    return best;
}

void Controller::queryActiveIndiDrivers()
{
    _activeIndiDrivers.clear();

    QProcess proc;
    proc.start("indi_getprop", {"-t", "2", "*.CONNECTION.CONNECT"});
    proc.waitForFinished(4000);

    for (const QString &line : QString(proc.readAllStandardOutput()).split('\n'))
    {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;

        QString deviceLabel = trimmed.section('.', 0, 0);
        if (deviceLabel.isEmpty()) continue;

        const IndiDriverInfo *best = bestCatalogMatchForLiveName(deviceLabel);
        if (best)
        {
            _activeIndiDrivers.append(*best);
            mLogger->info("Recovered active driver: " + deviceLabel + " (" + best->binary + ")");
        }
        else
        {
            mLogger->warning("Active driver not found in XML registry: " + deviceLabel);
        }
    }

    mLogger->info(QString::number(_activeIndiDrivers.size()) + " active INDI driver(s) recovered");
    updateControllerData("indiactivedrivers", activeIndiDriversToVariant());
}
void Controller::selectFolder(const QString &folder)
{
    // A folder selection is not a filesystem mutation: nothing was added or
    // removed on disk, only what the client is watching changed. Rebuild the
    // file list of the newly selected folder from scratch and replace it as a
    // whole (one "files" uc), instead of routing it through OnFileWatcherEvent's
    // diff logic, which would treat every file of the new folder as "added" and
    // every file of the old one as "removed" (a flood of af/df events entirely
    // redundant with the uc replacement that follows them). The folder tree
    // itself (ad/dd, "folders") is unaffected by a selection change and is not
    // rescanned/resent here.
    mSelectedFolder = folder;
    QStringList files;
    QDirIterator itFiles(mSelectedFolder, QDir::Files);
    while (itFiles.hasNext())
    {
        QString d = itFiles.next();
        d.replace(_webroot, "");
        files.append(d);
    }
    mFilesList = files;
    mControllerData["files"] = mFilesList;
    wshandler->onControllerEvent(OST::EvType::uc, "files", QVariant(mFilesList), nullptr);
}
void Controller::OnFileWatcherEvent(const QString &pEvent)
{
    // Folder tree: always a full recursive rescan of the whole webroot,
    // regardless of which watched path (webroot itself or the currently
    // selected folder) raised the signal. Folder visibility is global by
    // design: a client browsing one branch still has to learn about folders
    // appearing or disappearing in another one, so this cannot be scoped to
    // pEvent the way the files scan below is.
    QStringList folders;
    QDirIterator itFolders(_webroot, QDirIterator::Subdirectories);
    while (itFolders.hasNext())
    {
        QString d = itFolders.next();
        d.replace(_webroot, "");
        if (d.endsWith("/.") || d.endsWith("/.."))
        {
            QString dd = d;
            dd.replace("/..", "");
            dd.replace("/.", "");
            if (!folders.contains(dd))
            {
                folders.append(dd);
            }
        }
    }

    // A single real filesystem change can raise more than one directoryChanged
    // signal (the OS/Qt watcher is not deduplicated upstream). Track whether
    // this pass actually found a difference and skip a broadcast when it
    // didn't, instead of resending the same list on every spurious extra
    // signal - folders and files are tracked separately so a files-only
    // change doesn't also resend the (unchanged) folder list, and vice versa.
    bool foldersChanged = false;
    for (const auto &i : folders)
    {
        if (!mFoldersList.contains(i) )
        {
            wshandler->processFileEvent("ad", i);
            mFoldersList.append(i);
            foldersChanged = true;
        }
    }
    for (const auto &i : QStringList(mFoldersList))    // iterate a copy: mFoldersList is mutated below
    {
        if (!folders.contains(i) )
        {
            wshandler->processFileEvent("dd", i);
            mFoldersList.removeOne(i);
            foldersChanged = true;
        }
    }
    if (foldersChanged)
    {
        mControllerData["folders"] = mFoldersList;
        wshandler->onControllerEvent(OST::EvType::uc, "folders", QVariant(mFoldersList), nullptr);
    }

    // Files: only the selected folder's content is watched and broadcast. If
    // the change that raised this signal isn't the selected folder itself
    // (e.g. it's the permanent webroot watch catching a folder appearing
    // elsewhere), there is nothing to do here.
    if (pEvent != mSelectedFolder)
    {
        return;
    }

    QStringList files;
    QDirIterator itFiles(mSelectedFolder, QDir::Files);
    while (itFiles.hasNext())
    {
        QString d = itFiles.next();
        d.replace(_webroot, "");
        files.append(d);
    }

    bool filesChanged = false;
    for (const auto &i : files)
    {
        if (!mFilesList.contains(i) )
        {
            wshandler->processFileEvent("af", i);
            mFilesList.append(i);
            filesChanged = true;
        }
    }
    for (const auto &i : QStringList(mFilesList))    // iterate a copy: mFilesList is mutated below
    {
        if (!files.contains(i) )
        {
            wshandler->processFileEvent("df", i);
            mFilesList.removeOne(i);
            filesChanged = true;
        }
    }
    if (filesChanged)
    {
        mControllerData["files"] = mFilesList;
        wshandler->onControllerEvent(OST::EvType::uc, "files", QVariant(mFilesList), nullptr);
    }
}
void Controller::OnFileChangeEvent(const QString &pEvent)
{
    //qDebug() << "****************************************** FileChanged" << pEvent;
}

void Controller::logDebug(const QString &message)
{
    logDebug(message, {});
}
void Controller::logDebug(const QString &message, const QVariantList &args)
{
    // Log to console (server language)
    mLogger->onLog(OST::LogLevel::Debug, message, args, "CT");

    // Broadcast to WebSocket clients (each in their language)
    wshandler->onLog(OST::LogLevel::Debug, message, args, "CT");
}
void Controller::logInfo(const QString &message)
{
    logInfo(message, {});
}
void Controller::logInfo(const QString &message, const QVariantList &args)
{
    // Log to console (server language)
    mLogger->onLog(OST::LogLevel::Info, message, args, "CT");

    // Broadcast to WebSocket clients (each in their language)
    wshandler->onLog(OST::LogLevel::Info, message, args, "CT");
}
void Controller::logError(const QString &message)
{
    logError(message, {});
}
void Controller::logError(const QString &message, const QVariantList &args)
{
    // Log to console (server language)
    mLogger->onLog(OST::LogLevel::Error, message, args, "CT");

    // Broadcast to WebSocket clients (each in their language)
    wshandler->onLog(OST::LogLevel::Error, message, args, "CT");
}
void Controller::logCritical(const QString &message)
{
    logCritical(message, {});
}
void Controller::logCritical(const QString &message, const QVariantList &args)
{
    // Log to console (server language)
    mLogger->onLog(OST::LogLevel::Critical, message, args, "CT");

    // Broadcast to WebSocket clients (each in their language)
    wshandler->onLog(OST::LogLevel::Critical, message, args, "CT");
}
QJsonObject Controller::getModulesDump(QString clientgrant, QString clientLanguage)
{
    QJsonObject result, dump, files, modules;
    dump["grant-client"] = clientgrant;
    dump["grant-server"] = _grant;

    if (clientgrant == "-1")
    {
        result["d"] = dump;
        return result;
    }

    files["folders"] = QJsonArray::fromStringList(mFoldersList);
    files["files"] = QJsonArray::fromStringList(mFilesList);
    files["selectedfolder"] = mSelectedFolder;

    QList<Basemodule *> allmodules = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
    for (Basemodule *module : allmodules)
    {
        modules[module->getModuleName()] = OST::ModuleJsonDumper(OST::EvType::aa, QVariant(), nullptr, nullptr, nullptr, module);
    }

    dump["m"] = modules;
    dump["files"] = files;
    dump["logs"] = mLogger->getJsonHistory(clientLanguage);
    dump["serverlng"] = _lng;
    dump["controllerdata"] = QJsonObject().fromVariantMap(mControllerData);

    QJsonObject lovs;
    foreach(const QString &key, mControllerLovs.keys())
    {
        OST::LovJsonDumper d;
        mControllerLovs[key]->accept(&d);
        lovs[key] = d.getResult();
    }
    dump["lovs"] = lovs;

    result["d"] = dump;
    return result;
}
void Controller::updateControllerData(QString key, QVariant data)
{
    mControllerData[key] = data;
    wshandler->onControllerEvent(OST::EvType::uc, key, data, nullptr);
}
void Controller::onInterModuleRequest(OST::ExtEvent event)
{
    Basemodule *pSenderModule = qobject_cast<Basemodule *>(sender());

    QList<Basemodule *> modules = findChildren<Basemodule *>(event.mod, Qt::FindChildrenRecursively);

    if (modules.size() == 0)
    {
        logError("Controller::onInterModuleRequest - module not found : %1", {event.mod});
        return;
    }
    if (modules.size() > 1)
    {
        logError("Controller::onInterModuleRequest - problem finding module : %1 %2 ", {event.mod, modules.size()});
        return;
    }

    //logDebug("Controller::onInterModuleRequest %1 %2 %3 %4", {OST::ExtEvToString(event.ev), event.mod, event.prpkey, event.data});

    Basemodule *pTargetModule = modules[0];
    pTargetModule->onExternalEventRoot(event);


}
void Controller::onSystemWatch()
{
    QVariantMap data;

    // --- RAM ---
    {
        QFile f("/proc/meminfo");
        if (f.open(QIODevice::ReadOnly))
        {
            quint64 memTotal = 0, memAvailable = 0;
            const QStringList lines = QString(f.readAll()).split('\n');
            f.close();
            for (const QString &raw : lines)
            {
                const QString     line  = raw.simplified();
                const QStringList parts = line.split(' ');
                if (line.startsWith("MemTotal:"))          memTotal     = parts.value(1).toULongLong();
                else if (line.startsWith("MemAvailable:")) memAvailable = parts.value(1).toULongLong();
            }
            QVariantMap ram;
            ram["used_mb"]  = (quint64)((memTotal - memAvailable) / 1024);
            ram["total_mb"] = (quint64)(memTotal / 1024);
            data["ram"] = ram;
        }
    }

    // --- CPU (load averages) ---
    {
        QFile f("/proc/loadavg");
        if (f.open(QIODevice::ReadOnly))
        {
            const QStringList p = QString(f.readAll()).simplified().split(' ');
            f.close();
            QVariantMap cpu;
            cpu["load_1m"]  = p.value(0).toDouble();
            cpu["load_5m"]  = p.value(1).toDouble();
            cpu["load_15m"] = p.value(2).toDouble();
            data["cpu"] = cpu;
        }
    }

    // --- Network (throughput) ---
    {
        QFile f("/proc/net/dev");
        if (f.open(QIODevice::ReadOnly))
        {
            const double elapsedSec = mNetTimer.restart() / 1000.0;
            const QStringList lines = QString(f.readAll()).split('\n');
            f.close();
            QVariantList interfaces;
            for (const QString &raw : lines)
            {
                const QString line = raw.simplified();
                if (!line.contains(':')) continue;
                const QString iface = line.section(':', 0, 0).simplified();
                if (iface == "lo") continue;
                const QStringList fields = line.section(':', 1).simplified().split(' ');
                if (fields.size() < 9) continue;
                const quint64 rx = fields.value(0).toULongLong();
                const quint64 tx = fields.value(8).toULongLong();
                QVariantMap net;
                net["iface"] = iface;
                if (mNetPrevRx.contains(iface) && elapsedSec > 0)
                {
                    net["rx_kbps"] = (rx - mNetPrevRx[iface]) / 1024.0 / elapsedSec;
                    net["tx_kbps"] = (tx - mNetPrevTx[iface]) / 1024.0 / elapsedSec;
                }
                else
                {
                    net["rx_kbps"] = 0.0;
                    net["tx_kbps"] = 0.0;
                }
                mNetPrevRx[iface] = rx;
                mNetPrevTx[iface] = tx;
                interfaces.append(net);
            }
            data["network"] = interfaces;
        }
    }

    // --- CPU Temperature (hwmon: coretemp=Intel, k10temp=AMD) ---
    {
        static const QStringList cpuSensors = {"coretemp", "k10temp"};
        QDir hwmonDir("/sys/class/hwmon");
        const QStringList hwmons = hwmonDir.entryList(QStringList() << "hwmon*", QDir::Dirs);
        QVariantList temperatures;
        for (const QString &hwmon : hwmons)
        {
            const QString base = "/sys/class/hwmon/" + hwmon;
            QFile nameFile(base + "/name");
            if (!nameFile.open(QIODevice::ReadOnly)) continue;
            const QString name = QString(nameFile.readAll()).simplified();
            nameFile.close();
            if (!cpuSensors.contains(name)) continue;

            // Read all temp*_input files in this hwmon
            QDir dir(base);
            const QStringList inputs = dir.entryList(QStringList() << "temp*_input", QDir::Files);
            for (const QString &inputFile : inputs)
            {
                QFile tempFile(base + "/" + inputFile);
                if (!tempFile.open(QIODevice::ReadOnly)) continue;
                const int milliC = QString(tempFile.readAll()).simplified().toInt();
                tempFile.close();

                // Try to read the matching label (temp1_input -> temp1_label)
                QString label = name;
                const QString labelName = QString(inputFile).replace("_input", "_label");
                QFile labelFile(base + "/" + labelName);
                if (labelFile.open(QIODevice::ReadOnly))
                {
                    label = QString(labelFile.readAll()).simplified();
                    labelFile.close();
                }

                QVariantMap t;
                t["type"]   = label;
                t["temp_c"] = milliC / 1000.0;
                temperatures.append(t);
            }
        }
        data["temperatures"] = temperatures;
    }

    // --- Disks ---
    {
        QFile f("/proc/mounts");
        if (f.open(QIODevice::ReadOnly))
        {
            const QStringList lines = QString(f.readAll()).split('\n');
            f.close();
            QStringList    seen;
            QVariantList   disks;
            for (const QString &raw : lines)
            {
                const QStringList parts = raw.split(' ');
                if (parts.size() < 3) continue;
                const QString device     = parts[0];
                const QString mountpoint = parts[1];
                if (!device.startsWith("/dev/") || device.startsWith("/dev/loop")) continue;
                if (seen.contains(device)) continue;
                seen.append(device);
                struct statvfs st;
                if (::statvfs(mountpoint.toLocal8Bit().constData(), &st) == 0)
                {
                    const quint64 total = (quint64)st.f_blocks * st.f_frsize;
                    const quint64 used  = total - (quint64)st.f_bavail * st.f_frsize;
                    QVariantMap disk;
                    disk["device"]     = device;
                    disk["mountpoint"] = mountpoint;
                    disk["used_gb"]    = QString::number(used  / 1073741824.0, 'f', 1).toDouble();
                    disk["total_gb"]   = QString::number(total / 1073741824.0, 'f', 1).toDouble();
                    disks.append(disk);
                }
            }
            data["disks"] = disks;
        }
    }

    // --- INDI server ---
    // "Y": the process is OST's own, already tracked live via QProcess's
    // finished signal (setIndiServerState()) and updated on every
    // start/stop/reload right where it happens - nothing to poll for here.
    // "N": OST has no event of its own for a server it doesn't own, so this
    // is the only place checking it at all.
    if (_indiserver == "N")
    {
        QProcess pgrep;
        pgrep.start("pgrep", {"-x", "indiserver"});
        pgrep.waitForFinished(1000);
        const bool running = (pgrep.exitCode() == 0);
        data["indiserverrunning"] = running;
        if (running)
        {
            queryActiveIndiDrivers();
        }
        else if (!_activeIndiDrivers.isEmpty())
        {
            _activeIndiDrivers.clear();
            updateControllerData("indiactivedrivers", QVariantList());
        }
    }

    updateControllerData("systemwatcher", data);
}

void Controller::forceAdminPassword(const QString &pw)
{
    /* We do this at controller level to forbid DBmanager inherited instances to do it ... */
    if (pw == "") return;
    if(!QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
        logError("forceAdminPassword - ERROR: QSQLITE driver unavailable");
        return;
    }
    QString dbfile = _dbpath + "ost.db";
    bool mDbExists = QFile::exists(dbfile);
    if (!mDbExists)
    {
        logError("forceAdminPassword - db file not found %1", {dbfile});
        return;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "TEMP");
    db.setDatabaseName(dbfile);

    QSqlQuery query = QSqlQuery(db);
    if(!db.open())
    {
        logError("forceAdminPassword - ERROR: %1 - %2", {db.databaseName(), db.lastError().text()});
        return ;
    }
    QString sql = "UPDATE USERS SET PW='" + pw + "' WHERE LOGIN='ADMIN';";
    if (!query.exec(sql))
    {
        logError("forceAdminPassword - ERROR SQL = %1", {sql});
        logError("forceAdminPassword - ERROR : %1", {query.lastError().text()});
    }
    else
    {
        logInfo("forceAdminPassword - OK");
    }

    db.close();
}
