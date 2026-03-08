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
                       OST::Logger *logger, OST::TranslateManager *translate, const QString &banner)
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

    //pMainControl = new Maincontrol(QString("mainctl"), QString("Main control"), QString(), QVariantMap());
    //connect(pMainControl, &Maincontrol::moduleEvent, this, &Controller::onModuleEvent);
    //connect(pMainControl, &Maincontrol::moduleEvent, wshandler, &WShandler::onModuleEvent);
    //connect(pMainControl, &Maincontrol::loadOtherModule, this, &Controller::loadModule);
    //connect(pMainControl, &Maincontrol::mainCtlEvent, this, &Controller::OnMainCtlEvent);
    //connect(this, &Controller::controllerEvent, pMainControl, &Maincontrol::OnExternalEvent);

    //pMainControl->setParent(this);
    //pMainControl->setWebroot(_webroot);
    //pMainControl->setObjectName("mainctl");
    //pMainControl->dbInit(_dbpath, "mainctl");
    //pMainControl->OnExternalEvent({"refreshConfigurations", "mainctl", QString(), QString(), 0, QVariantMap()});
    //pMainControl->setAvailableModuleLibs(_availableModuleLibs);
    //pMainControl->setIndiDriverList(_availableIndiDrivers);
    //pMainControl->sendDump();

    // Connect MainControl to log system
    //connect(pMainControl, &Basemodule::logSignal, mLogger, &OST::Logger::onLog);
    //connect(pMainControl, &Basemodule::logSignal, wshandler, &WShandler::onLog);

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

}


Controller::~Controller()
{
}


bool Controller::loadModule(QString lib, QString name, QString label, QString profile)
{
    if (mModulesMap.contains(name ))
    {
        //pMainControl->sendMainError("Module " + name + " already loaded - can't load twice");
        return false;
    }
    QLibrary library("libost" + lib);
    if (!library.load())
    {
        //pMainControl->sendMainError(name + " " + library.errorString());
        return false;
    }
    //pMainControl->sendMainMessage(name + " library loaded");

    typedef Basemodule *(*CreateModule)(QString, QString, QString, QVariantMap);
    CreateModule createmodule = (CreateModule)library.resolve("initialize");
    if (!createmodule)
    {
        //pMainControl->sendMainError("Could not initialize module from library : " + lib);
        return false;
    }
    Basemodule *mod = createmodule(name, label, profile, _availableModuleLibs);
    //QPointer<Basemodule> mod = createmodule(name,label,profile,_availableModuleLibs);
    if (!mod)
    {
        // pMainControl->sendMainError("Could not instanciate module from library : " + lib);
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
    //connect(mod, &Datastore::moduleEvent, wshandler, &WShandler::processModuleEvent);
    connect(mod, &Basemodule::moduleEvent, wshandler, &WShandler::onModuleEvent);

    connect(mod, &Basemodule::loadOtherModule, this, &Controller::loadModule);
    connect(this, &Controller::controllerEvent, mod, &Basemodule::onExternalEventRoot);
    //connect(wshandler, &WShandler::externalEvent, mod, &Basemodule::OnExternalEvent);

    // Connect module to log system
    connect(mod, &Basemodule::logSignal, mLogger, &OST::Logger::onLog);
    connect(mod, &Basemodule::logSignal, wshandler, &WShandler::onLog);

    //mod->OnExternalEvent({"afterinit", name, QString(), QString(), 0, QVariantMap()});
    //mod->sendDump();

    QList<Basemodule *> othermodules = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
    for (Basemodule *othermodule : othermodules)
    {
        if (othermodule->getModuleName() != mod->getModuleName())
        {
            //connect(othermodule, &Basemodule::moduleStatusRequest, mod, &Basemodule::OnModuleStatusRequest);
            //connect(othermodule, &Basemodule::moduleStatusAnswer, mod, &Basemodule::OnModuleStatusAnswer);
            //connect(mod, &Basemodule::moduleStatusRequest, othermodule, &Basemodule::OnModuleStatusRequest);
            //connect(mod, &Basemodule::moduleStatusAnswer, othermodule, &Basemodule::OnModuleStatusAnswer);
            //connect(othermodule, &Datastore::moduleEvent, mod, &Basemodule::OnExternalEvent);
            //connect(mod, &Datastore::moduleEvent, othermodule, &Basemodule::OnExternalEvent);
        }
    }
    QMap<QString, QString> l;
    l["label"] = label;
    l["type"] = lib;
    l["profile"] = profile;
    mModulesMap[name] = l;

    //pMainControl->addModuleData(name, label, lib, profile);
    //pMainControl->sendMainMessage("Module  " + label + " successfully loaded");

    updateGlobalModulesLov();

    return true;

}
void Controller::loadConf(const QString &pConf)
{
    QVariantMap result;
    if (!dbmanager->getDbConfiguration(pConf, result))
    {
        //pMainControl->sendMainError("loadConf " + pConf + " failed");
        return;
    }
    for(QVariantMap::const_iterator iter = result.begin(); iter != result.end(); ++iter)
    {
        QVariantMap line = iter.value().toMap();
        QString namewithoutblanks = iter.key();
        namewithoutblanks.replace(" ", "");
        loadModule(line["moduletype"].toString(), namewithoutblanks, iter.key(), line["profilename"].toString());
    }
    //pMainControl->sendMainMessage("loadConf " + pConf + " successful");
}
void Controller::saveConf(const QString &pConf)
{
    QVariantMap result;
    if (!dbmanager->saveDbConfiguration(pConf, mModulesMap))
    {
        //pMainControl->sendMainError("saveDbConfiguration " + pConf + " failed");
        return;
    }
    //pMainControl->sendMainMessage("saveDbConfiguration " + pConf + " sucessfull");

}

void Controller::onModuleEvent(OST::EvType evt, QVariant data, OST::ElementBase* elt, OST::PropertyBase* prp,
                               OST::LovBase* lov,
                               Datastore* mod)

{

    //    if (event.type == "moduledelete")
    //    {
    //        if (!mModulesMap.contains(event.module))
    //        {
    //            pMainControl->sendMainWarning("moduledelete Module " + event.module + " not in module map");
    //            return;
    //        }
    //        mModulesMap.remove(event.module);
    //        pMainControl->deldModuleData(event.module);
    //        updateGlobalModulesLov();
    //    }
    //    if (event.type == "modulesavedprofile")
    //    {
    //        //mModulesMap[event.module]["profile"] = pEventKey;
    //        //pMainControl->setModuleData(event.module, "", "", pEventKey);
    //
    //    }
    //    if (event.type == "moduleloadedprofile")
    //    {
    //        //mModulesMap[event.module]["profile"] = pEventKey;
    //        //pMainControl->setModuleData(event.module, "", "", pEventKey);
    //
    //    }

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
    //qDebug() << "Controller::onExternalEvent" << event.data;
    //    ZZ = 0,    /*!< invalid request */
    //    DU,        /*!< request dump */
    //    LO,        /*!< login request */
    //    IL,        /*!< set client language request */
    //    PL,        /*!< Load profile */
    //    PS,        /*!< Save profile */
    //    CL,        /*!< Load configuration */
    //    CS,        /*!< Save configuration*/
    //    SV,        /*!< set a value of a property */
    //    SA,        /*!< set all values of a property */
    //    GC,        /*!< grid new line */
    //    GU,        /*!< grid update line  */
    //    GF,        /*!< grid fetch line  */
    //    GD,        /*!< grid delete line  */
    //    GR,        /*!< grid reset */
    switch (event.ev)
    {
        case OST::ExtEvType::ZZ:
        case OST::ExtEvType::DU:
        case OST::ExtEvType::LO:
        case OST::ExtEvType::IL:
            logError("Controller::onExternalEvent - invalid event here - %1", {OST::ExtEvToString(event.ev)});
            return;
        case OST::ExtEvType::FS:
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
        default:
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
            if (!mModulesMap.contains(event.data["m"].toObject().begin().key()))
            {
                logError("Controller::onExternalEvent - unknown module - %1", {event.data["m"].toObject().begin().key()});
                return;
            };

    }

    /* basic data validity checks below */
    QString mod = event.data["m"].toObject().begin().key();
    QJsonObject o = event.data["m"].toObject()[mod].toObject();
    QJsonObject p, e;
    event.line = -1;

    /* data must contain a unique "p" key for these */
    if (event.ev == OST::ExtEvType::SV || event.ev == OST::ExtEvType::SA || event.ev == OST::ExtEvType::GC
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
    if (event.ev == OST::ExtEvType::SV)
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
//void Controller::OnMainCtlEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
//                                const QVariantMap &pEventData)
//{
//    Q_UNUSED(pEventModule);
//    Q_UNUSED(pEventData);
//
//    if (pEventType == "loadconf")
//    {
//        loadConf(pEventKey);
//    }
//    if (pEventType == "saveconf")
//    {
//        saveConf(pEventKey);
//    }
//    if (pEventType == "startindidriver")
//    {
//        startIndiDriver(pEventKey);
//    }
//    if (pEventType == "stopindidriver")
//    {
//        stopIndiDriver(pEventKey);
//    }
//    if (pEventType == "indiserver")
//    {
//        if (pEventKey == "start")
//        {
//            startIndi();
//        }
//        if (pEventKey == "stop")
//        {
//            stopIndi();
//        }
//    }
//    if (pEventType == "killall")
//    {
//        QList<Basemodule *> othermodules = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
//        for (Basemodule *othermodule : othermodules)
//        {
//            if (othermodule->getModuleName() != "mainctl")
//            {
//                othermodule->killMe();
//            }
//        }
//
//    }
//
//}
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



}
void Controller::OnFileChangeEvent(const QString &pEvent)
{
    qDebug() << "****************************************** FileChanged" << pEvent;
}

void Controller::updateGlobalModulesLov(void)
{
    QVariantMap lovData;
    QVariantList values;

    // Add all loaded modules (excluding mainctl)
    for(QMap<QString, QMap<QString, QString>>::const_iterator iter = mModulesMap.begin();
            iter != mModulesMap.end(); ++iter)
    {
        QVariantMap item;
        item["key"] = iter.key();
        item["label"] = iter.value()["label"];
        values.append(item);
    }

    lovData["values"] = values;

    // Emit event to update all modules with the new list
    // emit controllerEvent({"globallovupdate", "*", "loadedModules", QString(), 0, lovData});
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
    dump["banner"] = mBanner;
    dump["serverlng"] = _lng;

    result["d"] = dump;
    return result;
}
