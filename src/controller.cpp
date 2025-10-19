#include "controller.h"
#include <QNetworkInterface>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QHostInfo>

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
                       const QString &ssl, const QString &sslCert, const QString &sslKey, const QString &lng)
    :       _webroot(webroot),
            _dbpath(dbpath),
            _libpath(libpath),
            _conf(conf),
            _indiserver(indiserver),
            _lng(lng)
{

    startPublish();
    wshandler = new WShandler(this, ssl, sslCert, sslKey);
    connect(wshandler, &WShandler::externalEvent, this, &Controller::OnExternalEvent);
    dbmanager = new DBManager();
    dbmanager->dbInit(_dbpath, "controller");


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

    pMainControl = new Maincontrol(QString("mainctl"), QString("Main control"), QString(), QVariantMap());
    connect(pMainControl, &Maincontrol::moduleEvent, this, &Controller::OnModuleEvent);
    connect(pMainControl, &Maincontrol::moduleEvent, wshandler, &WShandler::processModuleEvent);
    connect(pMainControl, &Maincontrol::loadOtherModule, this, &Controller::loadModule);
    connect(pMainControl, &Maincontrol::mainCtlEvent, this, &Controller::OnMainCtlEvent);
    connect(this, &Controller::controllerEvent, pMainControl, &Maincontrol::OnExternalEvent);
    pMainControl->setParent(this);
    pMainControl->setWebroot(_webroot);
    pMainControl->setObjectName("mainctl");
    pMainControl->dbInit(_dbpath, "mainctl");
    pMainControl->OnExternalEvent("refreshConfigurations", "mainctl", QString(), QVariantMap());
    pMainControl->setAvailableModuleLibs(_availableModuleLibs);
    pMainControl->setIndiDriverList(_availableIndiDrivers);
    pMainControl->sendDump();
    pMainControl->setLng(_lng);

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
        pMainControl->sendMainError("Module " + name + " already loaded - can't load twice");
        return false;
    }
    QLibrary library("libost" + lib);
    if (!library.load())
    {
        pMainControl->sendMainError(name + " " + library.errorString());
        return false;
    }
    //pMainControl->sendMainMessage(name + " library loaded");

    typedef Basemodule *(*CreateModule)(QString, QString, QString, QVariantMap);
    CreateModule createmodule = (CreateModule)library.resolve("initialize");
    if (!createmodule)
    {
        pMainControl->sendMainError("Could not initialize module from library : " + lib);
        return false;
    }
    Basemodule *mod = createmodule(name, label, profile, _availableModuleLibs);
    //QPointer<Basemodule> mod = createmodule(name,label,profile,_availableModuleLibs);
    if (!mod)
    {
        pMainControl->sendMainError("Could not instanciate module from library : " + lib);
        return false;
    }
    mod->setParent(this);
    mod->setWebroot(_webroot);
    mod->setObjectName(name);
    mod->dbInit(_dbpath, name);
    mod->setProfile(profile);
    mod->setProfiles();
    mod->setLng(_lng);
    QVariantMap profs;
    dbmanager->getDbProfiles(mod->metaObject()->className(), profs);
    connect(mod, &Basemodule::moduleEvent, this, &Controller::OnModuleEvent);
    connect(mod, &Basemodule::moduleEvent, wshandler, &WShandler::processModuleEvent);
    connect(mod, &Basemodule::loadOtherModule, this, &Controller::loadModule);
    connect(this, &Controller::controllerEvent, mod, &Basemodule::OnExternalEvent);
    //connect(wshandler, &WShandler::externalEvent, mod, &Basemodule::OnExternalEvent);
    mod->OnExternalEvent("afterinit", name, QString(), QVariantMap());
    mod->sendDump();

    QList<Basemodule *> othermodules = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
    for (Basemodule *othermodule : othermodules)
    {
        if (othermodule->getModuleName() != mod->getModuleName())
        {
            connect(othermodule, &Basemodule::moduleStatusRequest, mod, &Basemodule::OnModuleStatusRequest);
            connect(othermodule, &Basemodule::moduleStatusAnswer, mod, &Basemodule::OnModuleStatusAnswer);
            connect(mod, &Basemodule::moduleStatusRequest, othermodule, &Basemodule::OnModuleStatusRequest);
            connect(mod, &Basemodule::moduleStatusAnswer, othermodule, &Basemodule::OnModuleStatusAnswer);
            connect(othermodule, &Basemodule::moduleEvent, mod, &Basemodule::OnExternalEvent);
            connect(mod, &Basemodule::moduleEvent, othermodule, &Basemodule::OnExternalEvent);
        }
    }
    QMap<QString, QString> l;
    l["label"] = label;
    l["type"] = lib;
    l["profile"] = profile;
    mModulesMap[name] = l;

    pMainControl->addModuleData(name, label, lib, profile);
    pMainControl->sendMainMessage("Module  " + label + " successfully loaded");

    updateGlobalModulesLov();

    return true;

}
void Controller::loadConf(const QString &pConf)
{
    QVariantMap result;
    if (!dbmanager->getDbConfiguration(pConf, result))
    {
        pMainControl->sendMainError("loadConf " + pConf + " failed");
        return;
    }
    for(QVariantMap::const_iterator iter = result.begin(); iter != result.end(); ++iter)
    {
        QVariantMap line = iter.value().toMap();
        QString namewithoutblanks = iter.key();
        namewithoutblanks.replace(" ", "");
        loadModule(line["moduletype"].toString(), namewithoutblanks, iter.key(), line["profilename"].toString());
    }
    pMainControl->sendMainMessage("loadConf " + pConf + " successful");
}
void Controller::saveConf(const QString &pConf)
{
    QVariantMap result;
    if (!dbmanager->saveDbConfiguration(pConf, mModulesMap))
    {
        pMainControl->sendMainError("saveDbConfiguration " + pConf + " failed");
        return;
    }
    pMainControl->sendMainMessage("saveDbConfiguration " + pConf + " sucessfull");

}


void Controller::OnModuleEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                               const QVariantMap &pEventData)
{
    Q_UNUSED(pEventKey);
    Q_UNUSED(pEventData);
    if (pEventType == "mm" || pEventType == "me" || pEventType == "mw")
    {
        //qDebug() << pEventModule << "-" << pEventData["message"].toString();
    }

    if (pEventType == "moduledelete")
    {
        if (!mModulesMap.contains(pEventModule))
        {
            pMainControl->sendMainWarning("moduledelete Module " + pEventModule + " not in module map");
        }
        mModulesMap.remove(pEventModule);
        pMainControl->deldModuleData(pEventModule);
        updateGlobalModulesLov();
    }
    if (pEventType == "modulesavedprofile")
    {
        mModulesMap[pEventModule]["profile"] = pEventKey;
        pMainControl->setModuleData(pEventModule, "", "", pEventKey);

    }
    if (pEventType == "moduleloadedprofile")
    {
        mModulesMap[pEventModule]["profile"] = pEventKey;
        pMainControl->setModuleData(pEventModule, "", "", pEventKey);

    }

}
void Controller::OnExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                 const QVariantMap &pEventData)
{
    QJsonObject obj = QJsonObject::fromVariantMap(pEventData);
    QJsonDocument doc(obj);
    QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    if (pEventModule == "mainctl")
    {
        pMainControl->sendMainMessage("Mainctl event : " + pEventType + " : " + pEventModule + " : " +  pEventKey + " : " +
                                      strJson);

    }
    if (pEventType == "Freadall")
    {
        wshandler->processFileEvent("foldersdump", mFoldersList);
        wshandler->processFileEvent("filesdump", mFilesList);
    }
    if (pEventType == "Ffolderselect")
    {
        for ( const auto &d : mFileWatcher.directories() )
        {
            mFileWatcher.removePath(d);
        }
        mSelectedFolder = _webroot + pEventKey;
        mFileWatcher.addPath(mSelectedFolder);
        OnFileWatcherEvent(QString());

    }

    /* we should check here if incoming message is valid*/
    emit controllerEvent(pEventType, pEventModule, pEventKey, pEventData);
}
void Controller::OnMainCtlEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                const QVariantMap &pEventData)
{
    Q_UNUSED(pEventModule);
    Q_UNUSED(pEventData);

    if (pEventType == "loadconf")
    {
        loadConf(pEventKey);
    }
    if (pEventType == "saveconf")
    {
        saveConf(pEventKey);
    }
    if (pEventType == "startindidriver")
    {
        startIndiDriver(pEventKey);
    }
    if (pEventType == "stopindidriver")
    {
        stopIndiDriver(pEventKey);
    }
    if (pEventType == "indiserver")
    {
        if (pEventKey == "start")
        {
            startIndi();
        }
        if (pEventKey == "stop")
        {
            stopIndi();
        }
    }
    if (pEventType == "killall")
    {
        QList<Basemodule *> othermodules = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
        for (Basemodule *othermodule : othermodules)
        {
            if (othermodule->getModuleName() != "mainctl")
            {
                othermodule->killMe();
            }
        }

    }

}
void Controller::checkModules(void)
{
    foreach (const QString &path, QCoreApplication::libraryPaths())
    {
        sendMessage("Check available modules in " + path);
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
                    sendMessage(lib + " " + library.errorString());
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
                            sendMessage("found library " + path + "/" + lib) ;
                            delete mod;
                        }
                    }
                    else
                    {
                        sendMessage("Could not initialize module from the loaded library : " + lib);
                    }
                }

            }


        }
    }


}
void Controller::checkIndiDrivers(void)
{
    const QString &path = "/usr/bin";

    sendMessage("Check available Indi drivers in " + path);
    QDir directory(path);
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList() << "indi_*");
    _availableIndiDrivers = directory.entryList();
    foreach(QString dr, _availableIndiDrivers)
    {
        //sendMessage("indi driver found " + dr);
    }
}
void Controller::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    pMainControl->sendMainMessage("PROCESS FINISHED (" + QString::number(exitCode) + ")" + exitStatus);
}
void Controller::processOutput()
{
    QString output = _process->readAllStandardOutput();
    pMainControl->sendMainMessage("PROCESS LOG   : " + output);
}
void Controller::processError()
{
    QString output = _process->readAllStandardError();
    pMainControl->sendMainError("PROCESS ERROR   : " + output);
}
void Controller::processIndiOutput()
{
    QString output = _indiProcess->readAllStandardOutput();
    pMainControl->sendMainMessage("INDI LOG   : " + output);
}
void Controller::processIndiError()
{
    QString output = _indiProcess->readAllStandardError();
    pMainControl->sendMainError("INDI ERROR   : " + output);
}
void Controller::sendMessage(const QString &pMessage)
{
    QString messageWithDateTime = "[" + QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + "]-" + pMessage;
    QDebug debug = qDebug();
    debug.noquote();
    debug << messageWithDateTime;
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
    pMainControl->sendMainMessage("Start Embedded indi server");
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
    pMainControl->sendMainMessage("Stop Embedded indi server");
    if (_indiProcess->isOpen()) _indiProcess->kill();

}
void Controller::startIndiDriver(const QString &pDriver)
{
    pMainControl->sendMainMessage("Start indidriver " + pDriver);
    QFile fifo("/tmp/ostserverIndiFIFO");
    fifo.open(QIODevice::WriteOnly);
    QTextStream txtStream(&fifo);
    txtStream << "start " + pDriver;
    fifo.close();

}
void Controller::stopIndiDriver(const QString &pDriver)
{
    pMainControl->sendMainMessage("Stop indidriver " + pDriver);
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
    emit controllerEvent("globallovupdate", "*", "loadedModules", lovData);
}
