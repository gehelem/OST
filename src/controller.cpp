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



    //LoadModule(QCoreApplication::applicationDirPath()+"/libostguider.so","guider1","Guider");
    //LoadModule(QCoreApplication::applicationDirPath()+"/libostinspector.so","inspector1","Frame inspector");
    //LoadModule(QCoreApplication::applicationDirPath()+"/libostpolar.so","polar1","Polar assistant");
    LoadModule("libostmaincontrol","mainctl","Maincontrol","default");
    LoadModule("libostdummy","dummy1","Dummy 1","default");
    //LoadModule(QCoreApplication::applicationDirPath()+"/libostindipanel.so","indipanel","indi control panel","default");

}


Controller::~Controller()
{
    /*m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());*/
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
                mod->setHostport(_indihost,_indiport);
                mod->connectIndi();
                mod->setWebroot(_webroot);
                mod->setObjectName(name);
                QVariantMap prof;
                dbmanager->getProfile(mod->_moduletype,profile,prof);
                mod->setProfile(prof);
                connect(mod,&Basemodule::moduleEvent, this,&Controller::OnModuleEvent);
                connect(mod,&Basemodule::moduleEvent, wshandler,&WShandler::OnModuleEvent);
                connect(mod,&Basemodule::loadOtherModule, this,&Controller::LoadModule);
                connect(this,&Controller::controllerEvent,mod,&Basemodule::OnExternalEvent);
                emit controllerEvent("dump",name,"*",QVariantMap());

                QList<Basemodule *> othermodules = findChildren<Basemodule *>(QString(),Qt::FindChildrenRecursively);
                for (Basemodule *othermodule : othermodules) {
                    //BOOST_LOG_TRIVIAL(debug) << "child= " << othermodule->objectName().toStdString();
                    if (othermodule->getName()!=mod->getName()) {
                        connect(othermodule,&Basemodule::moduleEvent, mod,&Basemodule::OnExternalEvent);
                        connect(mod,&Basemodule::moduleEvent, othermodule,&Basemodule::OnExternalEvent);

                    }
                }


            }
        } else {
            BOOST_LOG_TRIVIAL(debug)  << "Could not initialize module from the loaded library";
        }
    }
}


void Controller::OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{
    QJsonObject obj;
    obj["evt"]=eventType;
    obj["mod"]=eventModule;
    obj["key"]=eventKey;
    if (eventType=="moduledump") {
        obj["dta"]=QJsonObject::fromVariantMap(eventData);
    }
    if (eventType=="addprop"||eventType=="setpropvalue") {
        obj["dta"]=QJsonObject::fromVariantMap(eventData);
    }
    if (eventType=="delprop") {
        obj["key"]=eventKey;
    }


    QJsonDocument doc(obj);
    QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    //BOOST_LOG_TRIVIAL(debug) << "OnModuleEvent - " << mod->getName().toStdString() << " - " << eventType.toStdString() << " - " << strJson.toStdString();


}
void Controller::OnExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{
    BOOST_LOG_TRIVIAL(debug) << "Controller OnExternalEvent : " << eventType.toStdString() << "-" << eventKey.toStdString();
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
        if (!(tt=="libostmaincontrol" )) {
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
                    Basemodule *mod = createmodule(tt,"temp",QString(),QVariantMap());
                    if (mod) {
                        mod->setParent(this);
                        mod->setObjectName(lib);
                        QVariantMap info = mod->getModuleInfo();
                        _availableModuleLibs[tt]=info;
                        QString message;
                        foreach (QString key,info.keys()) {
                            message = message + "--" + key+ "=" + info[key].toString();
                        }
                        delete mod;
                    }
                } else {
                    BOOST_LOG_TRIVIAL(debug)  << "Could not initialize module from the loaded library";
                }
            }

        }


    }

}
