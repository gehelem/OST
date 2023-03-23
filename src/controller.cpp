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

    pMainControl = new Maincontrol(QString("mainctl2"), QString("Main control 2"), QString(), QVariantMap());
    connect(pMainControl, &Maincontrol::moduleEvent, this, &Controller::OnModuleEvent);
    connect(pMainControl, &Maincontrol::moduleEvent, wshandler, &WShandler::processModuleEvent);
    connect(pMainControl, &Maincontrol::loadOtherModule, this, &Controller::loadModule);
    connect(pMainControl, &Maincontrol::loadConf, this, &Controller::loadConf);
    connect(pMainControl, &Maincontrol::saveConf, this, &Controller::saveConf);
    connect(this, &Controller::controllerEvent, pMainControl, &Maincontrol::OnExternalEvent);
    pMainControl->setParent(this);
    pMainControl->setWebroot(_webroot);
    pMainControl->setObjectName("mainctl2");
    pMainControl->dbInit(_dbpath, "mainctl2");


    pMainControl->sendDump();

    loadModule("maincontrol", "mainctl", "Maincontrol", "default");


    if (_installfront != "N")
    {
        this->installFront();
    }

    checkModules();


    loadConf(_conf);

}


Controller::~Controller()
{
}


bool Controller::loadModule(QString lib, QString name, QString label, QString profile)
{
    if (mModulesMap.contains(name ))
    {
        sendMessage("Module " + name + " already loaded - can't load twice");
        return false;
    }
    QLibrary library("libost" + lib);
    if (!library.load())
    {
        sendMessage(name + " " + library.errorString());
        return false;
    }
    sendMessage(name + " library loaded");

    typedef Basemodule *(*CreateModule)(QString, QString, QString, QVariantMap);
    CreateModule createmodule = (CreateModule)library.resolve("initialize");
    if (!createmodule)
    {
        sendMessage("Could not initialize module from library : " + lib);
        return false;
    }
    Basemodule *mod = createmodule(name, label, profile, _availableModuleLibs);
    //QPointer<Basemodule> mod = createmodule(name,label,profile,_availableModuleLibs);
    if (!mod)
    {
        sendMessage("Could not instanciate module from library : " + lib);
        return false;
    }
    mod->setParent(this);
    mod->setWebroot(_webroot);
    mod->setObjectName(name);
    mod->dbInit(_dbpath, name);
    mod->setProfile(profile);
    QVariantMap profs;
    dbmanager->getDbProfiles(mod->metaObject()->className(), profs);
    if (name != "mainctl") mod->setProfiles(profs);
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
    if (name == "mainctl")
    {
        mod->OnExternalEvent("refreshConfigurations", name, QString(), QVariantMap());
    }
    else
    {
        QMap<QString, QString> l;
        l["label"] = label;
        l["type"] = lib;
        l["profile"] = profile;
        mModulesMap[name] = l;
    }
    return true;

}
void Controller::loadConf(const QString &pConf)
{
    QVariantMap result;
    if (!dbmanager->getDbConfiguration(pConf, result))
    {
        return;
    }
    for(QVariantMap::const_iterator iter = result.begin(); iter != result.end(); ++iter)
    {
        QVariantMap line = iter.value().toMap();
        QString namewithoutblanks = iter.key();
        namewithoutblanks.replace(" ", "");

        loadModule(line["moduletype"].toString(), namewithoutblanks, iter.key(), line["profilename"].toString());
    }

}
void Controller::saveConf(const QString &pConf)
{
    QVariantMap result;
    if (!dbmanager->saveDbConfiguration(pConf, mModulesMap))
    {
        sendMessage("saveDbConfiguration " + pConf + " failed");
        return;
    }
    sendMessage("saveDbConfiguration " + pConf + " sucessfull");

}


void Controller::OnModuleEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                               const QVariantMap &pEventData)
{
    Q_UNUSED(pEventKey);
    Q_UNUSED(pEventData);
    //if (pEventType == "mm" || pEventType == "me" || pEventType == "mw")
    //{
    //    sendMessage(pEventModule + "-" + pEventData["message"].toString());
    //}
    if (pEventType == "moduledelete")
    {
        if (!mModulesMap.contains(pEventModule))
        {
            sendMessage("moduledelete Module " + pEventModule + " not in module map");
        }
        mModulesMap.remove(pEventModule);
    }
}
void Controller::OnExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                 const QVariantMap &pEventData)
{
    QJsonObject obj = QJsonObject::fromVariantMap(pEventData);
    QJsonDocument doc(obj);
    QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    if (pEventModule == "mainctl2")
    {
        sendMessage("Mainctl event : " + pEventType + " : " + pEventModule + " : " +  pEventKey + " : " + strJson);

    }



    /* we should check here if incoming message is valid*/
    emit controllerEvent(pEventType, pEventModule, pEventKey, pEventData);
}
void Controller::checkModules(void)
{
    foreach (const QString &path, QCoreApplication::libraryPaths())
    {
        sendMessage(" ************ Check available modules in " + path);
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
void Controller::installFront(void)
{
    QString arch = _installfront;
    if (arch == "Y")
    {
        arch = "https://github.com/gehelem/ost-front/releases/download/WorkInProgress/html.tar.gz";
        sendMessage("download default archive " + arch);
    }
    else
    {
        sendMessage("download specific archive " + arch);
    }

    _process = new QProcess(this);
    connect(_process, &QProcess::readyReadStandardOutput, this, &Controller::processOutput);
    connect(_process, &QProcess::readyReadStandardError, this, &Controller::processError);
    connect(_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            &Controller::processFinished);
    sendMessage("****************************");
    sendMessage("Install default web frontend");
    sendMessage("****************************");
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
    sendMessage("PROCESS FINISHED (" + QString::number(exitCode) + ")" + exitStatus);
}
void Controller::processOutput()
{
    QString output = _process->readAllStandardOutput();
    sendMessage("PROCESS LOG   : " + output);
}
void Controller::processError()
{
    QString output = _process->readAllStandardError();
    sendMessage("PROCESS ERROR   : " + output);

}
void Controller::sendMessage(const QString &pMessage)
{
    pMainControl->sendMessage(pMessage);
}
