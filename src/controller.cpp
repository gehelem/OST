#include "controller.h"


/*!
 * ... ...
 */
Controller::Controller(bool saveAllBlobs, const QString& host, int port, const QString& webroot, const QString &dbpath)
    :_indihost(host),
      _indiport(port),
      _webroot(webroot),
      _dbpath(dbpath)
{

    Q_UNUSED(saveAllBlobs);

    checkModules();

    wshandler = new WShandler(this);
    connect(wshandler,&WShandler::externalEvent,this,&Controller::OnExternalEvent);
    dbmanager = new DBManager(this,_dbpath);



    LoadModule("libostinspector","inspector1","CCD inspector","default");
    //LoadModule(QCoreApplication::applicationDirPath()+"/libostpolar.so","polar1","Polar assistant");
    LoadModule("libostmaincontrol","mainctl","Maincontrol","default");
    //LoadModule("libostdummy","dummy1","Dummy 1","default");
    LoadModule("libostfocuser","focus1","My favorite focuser","default");
    //LoadModule("libostindipanel","indipanel","indi control panel","default");
    LoadModule("libostallsky","allsky","Allsky Camera","default");

}


Controller::~Controller()
{
}


void Controller::LoadModule(QString lib,QString name,QString label,QString profile)
{
    QString fulllib = QCoreApplication::applicationDirPath()+"/"+lib+".so";
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
    BOOST_LOG_TRIVIAL(debug) << "Check available modules";
    QDir directory(QCoreApplication::applicationDirPath());
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList() << "*ost*.so");
    QStringList libs = directory.entryList();
    foreach(QString lib, libs)
    {
        QString tt = lib.replace(".so","");
        if (!((tt=="libostmaincontrol" )||(tt=="libostbasemodule" )||(tt=="libostindimodule" ))) {
            QLibrary library(QCoreApplication::applicationDirPath()+"/"+lib);
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
