#include "controller.h"

/*!
 * ... ...
 */
Controller::Controller(bool saveAllBlobs, const QString &webroot, const QString &dbpath,
                       const QString &libpath, const QString &installfront, const QString &conf)
    : _webroot(webroot),
      _dbpath(dbpath),
      _libpath(libpath),
      _installfront(installfront),
      _conf(conf)
{

    Q_UNUSED(saveAllBlobs);
    if (_installfront != "N")
    {
        this->installFront();
    }
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

    wshandler = new WShandler(this);
    //connect(wshandler, &WShandler::externalEvent, this, &Controller::OnExternalEvent);
    dbmanager = new DBManager();
    dbmanager->dbInit(_dbpath, "controller");
    //connect(dbmanager, SIGNAL(dbEvent), &Controller::OnModuleEvent);


    loadModule("libostmaincontrol", "mainctl", "Maincontrol", "default");
    loadConf(_conf);
}


Controller::~Controller()
{
}


bool Controller::loadModule(QString lib, QString name, QString label, QString profile)
{

    QLibrary library(lib);
    if (!library.load())
    {
        sendMessage(name + " " + library.errorString());
        return false;
    }
    else
    {
        sendMessage(name + " library loaded");

        typedef Basemodule *(*CreateModule)(QString, QString, QString, QVariantMap);
        CreateModule createmodule = (CreateModule)library.resolve("initialize");
        if (createmodule)
        {
            Basemodule *mod = createmodule(name, label, profile, _availableModuleLibs);
            //QPointer<Basemodule> mod = createmodule(name,label,profile,_availableModuleLibs);
            if (mod)
            {
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
                if (name == "mainctl")
                {
                    connect(mod, &Basemodule::loadConf, this, &Controller::loadConf);
                }
                //connect(this, &Controller::controllerEvent, mod, &Basemodule::OnExternalEvent);
                connect(wshandler, &WShandler::externalEvent, mod, &Basemodule::OnExternalEvent);
                mod->sendDump();

                QList<Basemodule *> othermodules = findChildren<Basemodule *>(QString(), Qt::FindChildrenRecursively);
                for (Basemodule *othermodule : othermodules)
                {
                    //sendMessage("child= " + othermodule->objectName());
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

                return true;
            }
            return false;
        }
        else
        {
            sendMessage("Could not initialize module from library : " + lib);
            return false;
        }
    }
    return false;
}
void Controller::loadConf(const QString &pConf)
{
    QVariantMap result;
    dbmanager->getDbConfiguration(pConf, result);
    for(QVariantMap::const_iterator iter = result.begin(); iter != result.end(); ++iter)
    {
        QVariantMap line = iter.value().toMap();
        QString namewithoutblanks = iter.key();
        namewithoutblanks.replace(" ", "");

        loadModule("libost" + line["moduletype"].toString(), namewithoutblanks, iter.key(), line["profilename"].toString());
    }

}


void Controller::OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                               const QVariantMap &eventData)
{
    Q_UNUSED(eventKey);
    if (eventType == "mm" || eventType == "me" || eventType == "mw")
    {
        sendMessage(eventModule + "-" + eventData["message"].toString());
    }
}
void Controller::OnExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                                 const QVariantMap &eventData)
{
    //QJsonObject obj =QJsonObject::fromVariantMap(eventData);
    //QJsonDocument doc(obj);
    //QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    //QString strJson = QLatin1String(docByteArray);

    //sendMessage("Controller OnExternalEvent : " + eventType + " : " + eventModule+ " : "+  eventKey + " : " + strJson);


    /* we should check here if incoming message is valid*/
    emit controllerEvent(eventType, eventModule, eventKey, eventData);
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
    QString messageWithDateTime = "[" + QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + "]-" + pMessage;
    QDebug debug = qDebug();
    debug.noquote();
    debug << messageWithDateTime;
}
