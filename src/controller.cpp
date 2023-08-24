#include "controller.h"

/*!
 * ... ...
 */
Controller::Controller(const QString &webroot, const QString &dbpath,
                       const QString &libpath, const QString &installfront, const QString &conf)
    :       _webroot(webroot),
            _dbpath(dbpath),
            _libpath(libpath),
            _installfront(installfront),
            _conf(conf)
{

    wshandler = new WShandler(this);
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
    checkModules();
    pMainControl->setAvailableModuleLibs(_availableModuleLibs);

    pMainControl->sendDump();

    //loadModule("maincontrol", "mainctl", "Maincontrol", "default");


    if (_installfront != "N")
    {
        this->installFront();
    }



    loadConf(_conf);

    dbmanager->populateCatalog(":messier.txt", "Messier");
    //dbmanager->populateCatalog(":ngc.txt", "NGC");
    //dbmanager->populateCatalog(":sh2.txt", "Sh2");
    //dbmanager->populateCatalog(":ldn.txt", "LDN");
    //dbmanager->populateCatalog(":ic.txt", "IC");
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
    QVariantMap profs;
    dbmanager->getDbProfiles(mod->metaObject()->className(), profs);
    connect(mod, &Basemodule::moduleEvent, this, &Controller::OnModuleEvent);
    connect(mod, &Basemodule::moduleEvent, wshandler, &WShandler::processModuleEvent);
    connect(mod, &Basemodule::loadOtherModule, this, &Controller::loadModule);
    connect(this, &Controller::controllerEvent, mod, &Basemodule::OnExternalEvent);
    //connect(wshandler, &WShandler::externalEvent, mod, &Basemodule::OnExternalEvent);
    mod->sendDump();

    QList<Basemodule *> othermodules = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
    for (Basemodule *othermodule : othermodules)
    {
        if (othermodule->getModuleName() != mod->getModuleName())
        {
            //connect(othermodule,&Basemodule::moduleEvent, mod,&Basemodule::OnExternalEvent);
            //connect(mod,&Basemodule::moduleEvent, othermodule,&Basemodule::OnExternalEvent);
        }
    }
    QMap<QString, QString> l;
    l["label"] = label;
    l["type"] = lib;
    l["profile"] = profile;
    mModulesMap[name] = l;

    pMainControl->addModuleData(name, label, lib, profile);
    pMainControl->sendMainMessage("Module  " + label + " successfully loaded");

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
        pMainControl->sendMainMessage("Check available modules in " + path);
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
                    pMainControl->sendMainWarning(lib + " " + library.errorString());
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
                            pMainControl->sendMainMessage("found library " + path + "/" + lib) ;
                            delete mod;
                        }
                    }
                    else
                    {
                        pMainControl->sendMainError("Could not initialize module from the loaded library : " + lib);
                    }
                }

            }


        }
    }


}
void Controller::installFront(void)
{
    QString arch = _installfront;
    if (arch == "Y")
    {
        arch = "https://github.com/gehelem/ost-front/releases/download/WorkInProgress/html.tar.gz";
        pMainControl->sendMainMessage("download default archive " + arch);
    }
    else
    {
        pMainControl->sendMainMessage("download specific archive " + arch);
    }

    _process = new QProcess(this);
    connect(_process, &QProcess::readyReadStandardOutput, this, &Controller::processOutput);
    connect(_process, &QProcess::readyReadStandardError, this, &Controller::processError);
    connect(_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            &Controller::processFinished);
    pMainControl->sendMainMessage("****************************");
    pMainControl->sendMainMessage("Install default web frontend");
    pMainControl->sendMainMessage("****************************");
    if (_process->state() != 0)
    {
        qDebug() << "can't start process";
    }
    else
    {
        QString program = "rm";
        QStringList arguments;
        arguments << "-rf" ;
        arguments << _webroot + "/html.tar.gz";
        _process->start(program, arguments);
        _process->waitForFinished();
        program = "wget";
        arguments.clear();
        arguments << arch ;
        arguments << "--directory";
        arguments << _webroot;
        arguments << " && ";
        arguments << "tar";
        _process->start(program, arguments);
        _process->waitForFinished();
        program = "tar";
        arguments.clear();
        arguments << "-xvf";
        arguments << _webroot + "/html.tar.gz";
        arguments << "-C";
        arguments << _webroot;
        _process->start(program, arguments);
        _process->waitForFinished();

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
