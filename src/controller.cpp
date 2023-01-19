#include "controller.h"


/*!
 * ... ...
 */
Controller::Controller(bool saveAllBlobs, const QString& host, int port, const QString& webroot, const QString &dbpath,const QString& libpath,const QString& installfront,const QString& conf)
    :_indihost(host),
      _indiport(port),
      _webroot(webroot),
      _dbpath(dbpath),
      _libpath(libpath),
      _installfront(installfront),
      _conf(conf)
{

    Q_UNUSED(saveAllBlobs);
    if (_installfront=="Y") {
        this->installFront();
    }
    if (_libpath=="") {
        _libpath=QCoreApplication::applicationDirPath();
    }

    checkModules();

    wshandler = new WShandler(this);
    connect(wshandler,&WShandler::externalEvent,this,&Controller::OnExternalEvent);
    dbmanager = new DBManager(this,_dbpath);

    LoadModule("libostmaincontrol","mainctl","Maincontrol","default");

    QVariantMap _result;
    dbmanager->getConfiguration(_conf,_result);
     for(QVariantMap::const_iterator iter = _result.begin(); iter != _result.end(); ++iter) {
       QVariantMap _line=iter.value().toMap();
       QString _namewithoutblanks = iter.key();
       _namewithoutblanks.replace(" ","");

       LoadModule("libost"+_line["moduletype"].toString(),_namewithoutblanks,iter.key(),_line["profilename"].toString());
     }

}


Controller::~Controller()
{
}


void Controller::LoadModule(QString lib,QString name,QString label,QString profile)
{

    QString fulllib = _libpath+"/"+lib+".so";
    qDebug() << "Try to load " << fulllib;
    QLibrary library(fulllib);
    if (!library.load())
    {
        BOOST_LOG_TRIVIAL(debug) << name.toStdString() << " " << library.errorString().toStdString();
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << name.toStdString() << " " << "library loaded";

        typedef Basemodule *(*CreateModule)(QString,QString,QString,QVariantMap);
        CreateModule createmodule = (CreateModule)library.resolve("initialize");
        if (createmodule) {
            Basemodule *mod = createmodule(name,label,profile,_availableModuleLibs);
            //QPointer<Basemodule> mod = createmodule(name,label,profile,_availableModuleLibs);
            if (mod) {
                mod->setParent(this);
                //mod->setHostport(_indihost,_indiport);
                //mod->connectIndi();
                mod->setWebroot(_webroot);
                mod->setObjectName(name);
                QVariantMap prof;
                dbmanager->getProfile(mod->_moduletype,profile,prof);
                mod->setProfile(prof);
                QVariantMap profs;
                dbmanager->getProfiles(mod->_moduletype,profs);
                mod->setProfiles(profs);
                connect(mod,&Basemodule::moduleEvent, this,&Controller::OnModuleEvent);
                connect(mod,&Basemodule::loadOtherModule, this,&Controller::LoadModule);
                connect(this,&Controller::controllerEvent,mod,&Basemodule::OnExternalEvent);
                mod->sendDump();

                QList<Basemodule *> othermodules = findChildren<Basemodule *>(QString(),Qt::FindChildrenRecursively);
                for (Basemodule *othermodule : othermodules) {
                    //BOOST_LOG_TRIVIAL(debug) << "child= " << othermodule->objectName().toStdString();
                    if (othermodule->getName()!=mod->getName()) {
                        //connect(othermodule,&Basemodule::moduleEvent, mod,&Basemodule::OnExternalEvent);
                        //connect(mod,&Basemodule::moduleEvent, othermodule,&Basemodule::OnExternalEvent);

                    }
                }


            }
        } else {
            BOOST_LOG_TRIVIAL(debug)  << "Could not initialize module from the loaded library : " << fulllib.toStdString();
        }
    }
}


void Controller::OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{


    if (eventType=="modsaveprofile") {
        Basemodule* mod= qobject_cast<Basemodule*>(sender());
        QVariantMap _vm = mod->getProfile();
        dbmanager->setProfile(eventModule,eventKey,_vm);
        return;
    }
    if (eventType=="modloadprofile") {
        QVariantMap _prof;
        dbmanager->getProfile(eventModule,eventKey,_prof);
        Basemodule* mod= qobject_cast<Basemodule*>(sender());
        mod->setProfile(_prof);
        return;

    }
    wshandler->processModuleEvent(eventType,eventModule,eventKey,eventData);
    //QJsonDocument doc(obj);
    //QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    //QString strJson = QLatin1String(docByteArray);
    //BOOST_LOG_TRIVIAL(debug) << "OnModuleEvent - " << mod->getName().toStdString() << " - " << eventType.toStdString() << " - " << strJson.toStdString();


}
void Controller::OnExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{
    //QJsonObject obj =QJsonObject::fromVariantMap(eventData);
    //QJsonDocument doc(obj);
    //QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    //QString strJson = QLatin1String(docByteArray);

    //BOOST_LOG_TRIVIAL(debug) << "Controller OnExternalEvent : " << eventType.toStdString() << " : " << eventModule.toStdString() << eventKey.toStdString() << " : "<< " : " << strJson.toStdString();


    /* we should check here if incoming message is valid*/
    emit controllerEvent(eventType,eventModule,eventKey,eventData);
}
void Controller::checkModules(void)
{
    foreach (const QString &path, QCoreApplication::libraryPaths()) {
        BOOST_LOG_TRIVIAL(debug) << "Lib paths : " << path.toStdString();
    }

    BOOST_LOG_TRIVIAL(debug) << "Check available modules in " << _libpath.toStdString();
    QDir directory(_libpath);
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList() << "libost*.so");
    QStringList libs = directory.entryList();
    foreach(QString lib, libs)
    {
        QString tt = lib.replace(".so","");
        if (!((tt=="libostmaincontrol" )||(tt=="libostbasemodule" )||(tt=="libostindimodule" ))) {
            QLibrary library(_libpath+"/"+lib);
            if (!library.load())
            {
                BOOST_LOG_TRIVIAL(debug) << lib.toStdString() << " " << library.errorString().toStdString();
            }
            else
            {
                typedef Basemodule *(*CreateModule)(QString,QString,QString,QVariantMap);
                CreateModule createmodule = (CreateModule)library.resolve("initialize");


                if (createmodule) {
                    Basemodule *mod = createmodule(tt,tt,QString(),QVariantMap());
                    if (mod) {
                        mod->setParent(this);
                        mod->setObjectName(lib);
                        QVariantMap info = mod->getModuleInfo();
                        _availableModuleLibs[tt]=info;
                        delete mod;
                    }
                } else {
                    BOOST_LOG_TRIVIAL(debug)  << "Could not initialize module from the loaded library : " << lib.toStdString();
                }
            }

        }


    }

}
void Controller::installFront(void)
{
    _process = new QProcess(this);
    connect(_process,&QProcess::readyReadStandardOutput,this,&Controller::processOutput);
    connect(_process,&QProcess::readyReadStandardError, this,&Controller::processError);
    connect(_process,static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,&Controller::processFinished);
    qDebug() << "****************************";
    qDebug() << "Install default web frontend";
    qDebug() << "****************************";
    if (_process->state()!=0) {
        qDebug() << "can't start process";
    } else {
        QString program = "wget";
        QStringList arguments;
        arguments << "https://github.com/gehelem/ost-front/releases/download/WorkInProgress/html.tar.gz";
        arguments << "&&";
        arguments << "tar -xf html.tar.gz -C";
        arguments << _webroot;
        qDebug() << "PROCESS ARGS " << arguments;
        _process->start(program,arguments);

    }


}
void Controller::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "PROCESS FINISHED (" + QString::number(exitCode) + ")";
}
void Controller::processOutput()
{
    QString output = _process->readAllStandardOutput();
    qDebug() << "PROCESS LOG   : " << output;
}
void Controller::processError()
{
    QString output = _process->readAllStandardError();
    qDebug() << "PROCESS ERROR : " + output;

}
void Controller::sendMessage(QString message)
{
    QString mess = QDateTime::currentDateTime().toString("[yyyyMMdd hh:mm:ss.zzz]") + " - ostserver - " + message;
    qDebug() << mess;
    // should we add a dispatch over WS ?
}
