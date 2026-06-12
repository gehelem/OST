#include "controller.h"
#include <QNetworkInterface>
#include <QDirIterator>
#include <QFileSystemWatcher>
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
                       const int systemWatchInterval)
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
            mSystemWatchInterval(systemWatchInterval)
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

    if (mSystemWatchInterval > 0)
    {
        connect(&mSystemWatchTimer, &QTimer::timeout, this, &Controller::onSystemWatch);
        mSystemWatchTimer.start(mSystemWatchInterval * 1000);
        logInfo("System watch started, interval: %1s", {mSystemWatchInterval});
    }
}


Controller::~Controller()
{
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
    // This is redundant with global data, these lovs can (must? => yes) be managed at frontend level
    //QString lovname = "loadedModules-" + lowerlib;
    //if (!getControllerLovs().contains(lovname))
    //{
    //    OST::LovString* ls = new OST::LovString(lovname);
    //    createControllerLov(lovname, ls);
    //}
    //OST::LovString* ls = static_cast<OST::LovString*>(getControllerLovs()[lovname]);
    //if (!ls->contains(lovname)) ls->lovAdd(name, name);


    return true;

}
void Controller::loadConf(const QString &pConf)
{
    QVariantMap result;
    OST::LovString* l = static_cast<OST::LovString*>(getControllerLovs()["availableconfs"]);
    l->lovClear();

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

    QStringList driverLabels;
    if (dbmanager->getIndiConfiguration(pConf, driverLabels) && !driverLabels.isEmpty())
    {
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

    updateControllerData("currentconf", QVariant(pConf));
    QVariantMap confs;
    dbmanager->getDbConfigurations(confs);
    for(QVariantMap::const_iterator iter = confs.begin(); iter != confs.end(); ++iter)
    {
        l->lovAdd(iter.key(), iter.key());
    }
    updateControllerData("availableconfs", QVariant(confs));

}
void Controller::saveConf(const QString &pConf)
{
    OST::LovString* l = static_cast<OST::LovString*>(getControllerLovs()["availableconfs"]);
    l->lovClear();
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
        wshandler->sendJsonMessage(getModulesDump(clientgrant), client);
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
                    // This is redundant with global data, these lovs can (must? => yes) be managed at frontend level
                    //QString lovname = "loadedModules-" + m->getClassName().toLower();
                    //OST::LovString* ls = static_cast<OST::LovString*>(getControllerLovs()[lovname]);
                    //ls->lovDel(n);
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
            for ( const auto &d : mFileWatcher.directories() )
            {
                mFileWatcher.removePath(d);
            }
            mSelectedFolder = _webroot + event.data["folder"].toString();
            mFileWatcher.addPath(mSelectedFolder);
            OnFileWatcherEvent(QString());
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
        return;
    }
    _indiPid = _indiProcess->processId();
    mLogger->info("indiserver started (PID " + QString::number(_indiPid) + ")");

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

    auto it = std::find_if(_indiDrivers.begin(), _indiDrivers.end(),
                           [&](const IndiDriverInfo & d)
    {
        return d.binary == pDriver;
    });
    if (it != _indiDrivers.end() &&
            std::find_if(_activeIndiDrivers.begin(), _activeIndiDrivers.end(),
                         [&](const IndiDriverInfo & d)
{
    return d.binary == pDriver;
}) == _activeIndiDrivers.end())
    _activeIndiDrivers.append(*it);

    mLogger->info("Sent start command for driver: " + pDriver);
    updateControllerData("indiactivedrivers", activeIndiDriversToVariant());
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

        auto it = std::find_if(_indiDrivers.begin(), _indiDrivers.end(),
                               [&](const IndiDriverInfo & d)
        {
            return d.label == deviceLabel;
        });
        if (it != _indiDrivers.end())
        {
            _activeIndiDrivers.append(*it);
            mLogger->info("Recovered active driver: " + it->label + " (" + it->binary + ")");
        }
        else
        {
            mLogger->warning("Active driver not found in XML registry: " + deviceLabel);
        }
    }

    mLogger->info(QString::number(_activeIndiDrivers.size()) + " active INDI driver(s) recovered");
    updateControllerData("indiactivedrivers", activeIndiDriversToVariant());
}
void Controller::OnFileWatcherEvent(const QString &pEvent)
{
    QStringList files;
    QStringList folders;
    QDir dir(_webroot);
    QDirIterator it(_webroot, QDirIterator::Subdirectories);

    //check existing folders
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
    //check existing files
    QDirIterator itFiles(mSelectedFolder, QDirIterator::NoIteratorFlags);
    while (itFiles.hasNext())
    {
        QString d = itFiles.next();
        d.replace(_webroot, "");
        if (!d.endsWith("/.") && !d.endsWith("/..") && !folders.contains(d))
        {
            files.append(d);
        }
    }

    // compare to already known folders
    for (const auto &i : folders)
    {
        if (!mFoldersList.contains(i) )
        {
            wshandler->processFileEvent("folderadd", QStringList(i));
            mFoldersList.append(i);
            //mFileWatcher.addPath(_webroot + i);
        }
    }
    for (const auto &i : mFoldersList)
    {
        if (!folders.contains(i) )
        {
            wshandler->processFileEvent("folderdel", QStringList(i));
            mFoldersList.removeOne(i);
            //mFileWatcher.removePath(_webroot + i);
        }
    }

    // compare to already known files
    for (const auto &i : files)
    {
        if (!mFilesList.contains(i) )
        {
            wshandler->processFileEvent("fileadd", QStringList(i));
            mFilesList.append(i);
        }
    }
    for (const auto &i : mFilesList)
    {
        if (!files.contains(i) )
        {
            wshandler->processFileEvent("filedel", QStringList(i));
            mFilesList.removeOne(i);
        }
    }
    mControllerData["files"] = mFilesList;
    mControllerData["folders"] = mFoldersList;

    wshandler->onControllerEvent(OST::EvType::uc, "files", QVariant(mFilesList), nullptr);
    wshandler->onControllerEvent(OST::EvType::uc, "folders", QVariant(mFoldersList), nullptr);
    QVariantMap r;
    dbmanager->getDbProfiles(r);
    wshandler->onControllerEvent(OST::EvType::uc, "profiles", r, nullptr);
    updateControllerData("profiles", r);


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
QJsonObject Controller::getModulesDump(QString clientgrant)
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
    dump["logs"] = mLogger->getJsonHistory();
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
