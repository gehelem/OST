#include "controller.h"
#include <QNetworkInterface>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QHostInfo>
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
                       OST::Logger *logger, OST::TranslateManager *translate, const QString &banner,
                       const QString &gitSha, const QString &gitDate, const QString &gitMessage)
    :       _webroot(webroot),
            _dbpath(dbpath),
            _libpath(libpath),
            _conf(conf),
            _indiserver(indiserver),
            _lng(lng),
            _grant(grant),
            mLogger(logger),
            mTranslater(translate),
            mBanner(banner)
{
    // Register meta types for queued signal/slot connections
    // Must be done before any connect() calls that use these types
    qRegisterMetaType<OST::LogLevel>("OST::LogLevel");
    qRegisterMetaType<OST::EvType>("OST::EvType");

    QVariantMap g;
    g["Githash"] = gitSha;
    g["Gitdate"] = gitDate;
    g["Gitmessage"] = gitMessage;
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
    wshandler->dbmanager = dbmanager;

    // Connect DBManager to log system
    connect(dbmanager, &Basemodule::logSignal, mLogger, &OST::Logger::onLog);
    connect(dbmanager, &Basemodule::logSignal, wshandler, &WShandler::onLog);

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
    updateControllerData("indidrivers", _availableIndiDrivers);

    loadConf(_conf);

    if (_indiserver != "N")
    {
        this->startIndi();
    }

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
    return true;

}
void Controller::loadConf(const QString &pConf)
{
    QVariantMap result;
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
    updateControllerData("currentconf", QVariant(pConf));
    QVariantMap confs;
    dbmanager->getDbConfigurations(confs);
    updateControllerData("availableconfs", QVariant(confs));

}
void Controller::saveConf(const QString &pConf)
{
    QVariantMap ms, result;
    QList<Basemodule *> mods = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
    for (Basemodule *m : mods)
    {
        QVariantMap n;
        n["label"] = m->getModuleLabel();
        n["profile"] = m->getCurrentProfile();
        n["type"] = m->getClassName();
        ms[m->getModuleName()] = n;
    }
    if (!dbmanager->saveDbConfiguration(pConf, ms))
    {
        logError("Save configuration %1 failed", {pConf});
        return;
    }
    logInfo("Save configuration %1 sucessfull", {pConf});
    updateControllerData("currentconf", QVariant(pConf));
    QVariantMap confs;
    dbmanager->getDbConfigurations(confs);
    updateControllerData("availableconfs", QVariant(confs));
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
    QJsonValue v;
    if (mod) m = mod->getModuleName();
    if (prp) p = prp->key();
    if (elt) e = elt->key();

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

    emit otherModuleEvent(evt, m, p, e, v);
}
void Controller::OnClientEvent(OST::ExtEvent event, QWebSocket* client, QString clientgrant)
{
    if (event.ev == OST::ExtEvType::DU || event.ev == OST::ExtEvType::IL  || event.ev == OST::ExtEvType::LO )
    {
        wshandler->sendJsonMessage(getModulesDump(clientgrant), client);
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
            for (Basemodule *m : mods)
            {
                if (m->getModuleName() == n) m->killMe();
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
    foreach (const QString &path, QCoreApplication::libraryPaths())
    {
        mLogger->info("Check available modules in " + path);
        QDir directory(path);
        directory.setFilter(QDir::Files);
        directory.setNameFilters(QStringList() << "libost*.so");
        QStringList libs = directory.entryList();
        foreach(QString lib, libs)
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
    const QString &path = "/usr/bin";

    mLogger->info("Check available Indi drivers in " + path);
    QDir directory(path);
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList() << "indi_*");
    _availableIndiDrivers = directory.entryList();
    foreach(QString dr, _availableIndiDrivers)
    {
        //mLogger->info("indi driver found " + dr);
    }
}
void Controller::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    //pMainControl->sendMainMessage("PROCESS FINISHED (" + QString::number(exitCode) + ")" + exitStatus);
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
    QString output = _indiProcess->readAllStandardOutput();
    //pMainControl->sendMainMessage("INDI LOG   : " + output);
}
void Controller::processIndiError()
{
    QString output = _indiProcess->readAllStandardError();
    //pMainControl->sendMainError("INDI ERROR   : " + output);
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
    //pMainControl->sendMainMessage("Start Embedded indi server");
    _indiProcess = new QProcess(this);
    connect(_indiProcess, &QProcess::readyReadStandardOutput, this, &Controller::processIndiOutput);
    connect(_indiProcess, &QProcess::readyReadStandardError, this, &Controller::processIndiError);
    connect(_indiProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            &Controller::processFinished);


    if (_indiProcess->state() != 0)
    {
        qDebug() << "can't start process";
    }
    else
    {
        QString program = "rm";
        QStringList arguments;
        arguments << "-f";
        arguments << "/tmp/ostserverIndiFIFO" ;
        _indiProcess->start(program, arguments);
        _indiProcess->waitForFinished();

        program = "mkfifo";
        arguments.clear();
        arguments << "/tmp/ostserverIndiFIFO" ;
        _indiProcess->start(program, arguments);
        _indiProcess->waitForFinished();

        program = "indiserver";
        arguments.clear();
        //arguments << "-v";
        arguments << "-f";
        arguments << "/tmp/ostserverIndiFIFO";
        _indiProcess->start(program, arguments);

    }

}
void Controller::stopIndi(void)
{
    //pMainControl->sendMainMessage("Stop Embedded indi server");
    if (_indiProcess->isOpen()) _indiProcess->kill();

}
void Controller::startIndiDriver(const QString &pDriver)
{
    //pMainControl->sendMainMessage("Start indidriver " + pDriver);
    QFile fifo("/tmp/ostserverIndiFIFO");
    fifo.open(QIODevice::WriteOnly);
    QTextStream txtStream(&fifo);
    txtStream << "start " + pDriver;
    fifo.close();

}
void Controller::stopIndiDriver(const QString &pDriver)
{
    //pMainControl->sendMainMessage("Stop indidriver " + pDriver);
    QFile fifo("/tmp/ostserverIndiFIFO");
    fifo.open(QIODevice::WriteOnly);
    QTextStream txtStream(&fifo);
    txtStream << "stop " + pDriver;
    fifo.close();

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

    wshandler->onControllerEvent(OST::EvType::uc, "files", QVariant(mFilesList));
    wshandler->onControllerEvent(OST::EvType::uc, "folders", QVariant(mFoldersList));
    QVariantMap r;
    dbmanager->getDbProfiles(r);
    wshandler->onControllerEvent(OST::EvType::uc, "profiles", r);
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

    result["d"] = dump;
    return result;
}
void Controller::updateControllerData(QString key, QVariant data)
{
    mControllerData[key] = data;
    wshandler->onControllerEvent(OST::EvType::uc, key, data);
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

    logDebug("Controller::onInterModuleRequest %1 %2 %3 %4", {OST::ExtEvToString(event.ev), event.mod, event.prpkey, event.data});

    Basemodule *pTargetModule = modules[0];
    pTargetModule->onExternalEventRoot(event);


}
