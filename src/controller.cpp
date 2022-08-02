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

    //this->setParent(parent);
    Q_UNUSED(saveAllBlobs);


    wshandler = new WShandler(this);
    dbmanager = new DBManager(this,_dbpath);

    BOOST_LOG_TRIVIAL(debug) <<  "ApplicationDirPath :" << QCoreApplication::applicationDirPath().toStdString();

    /*MainControl *mainctl = new MainControl("maincontrol","Main control");
    mainctl->setHostport(_indihost,_indiport);
    mainctl->connectIndi();
    mainctl->setWebroot(_webroot);
    connect(mainctl,&Basemodule::newMessageSent, this,&Controller::OnNewMessageSent);
    connect(mainctl,&MainControl::loadModule, this,&Controller::OnLoadModule);

    connect(mainctl,&Basemodule::newMessageSent,wshandler,&WShandler::OnNewMessageSent);
    connect(mainctl,&Basemodule::moduleDumped2, wshandler,&WShandler::OnModuleDumped2);


    connect(wshandler,&WShandler::dumpAsked,mainctl,&Basemodule::OnDumpAsked);*/
    //connect(wshandler,&WShandler::setPropertyText,mainctl,&Basemodule::OnSetPropertyText);
    //connect(wshandler,&WShandler::setPropertyNumber,mainctl,&Basemodule::OnSetPropertyNumber);
    //connect(wshandler,&WShandler::setPropertySwitch,mainctl,&Basemodule::OnSetPropertySwitch);


    //LoadModule(QCoreApplication::applicationDirPath()+"/libostguider.so","guider1","Guider");
    //LoadModule(QCoreApplication::applicationDirPath()+"/libostinspector.so","inspector1","Frame inspector");
    //LoadModule(QCoreApplication::applicationDirPath()+"/libostpolar.so","polar1","Polar assistant");
    //LoadModule(QCoreApplication::applicationDirPath()+"/libostfocuser.so","focus1","Focus assistant");
    LoadModule(QCoreApplication::applicationDirPath()+"/libostdummy.so","dummy1","Demo module","default");
    LoadModule(QCoreApplication::applicationDirPath()+"/libostindipanel.so","indipanel","indi control panel","default");



    QVariantMap test;
    test["tata"]="totovalue";
    test["turlututu"]="chapeau pointu";
    test["titi"]="titivalue";
    test["pi"]=3.14159;
    test["vrai"]=true;

    QVariantMap elt;
    elt["elt1"]=1;
    elt["elt2"]="element 2";
    elt["elt3"]=false;
    test["elts"]=elt;


    if (!dbmanager->setProfile("dummy1","profil numéro 1",test)) {
        BOOST_LOG_TRIVIAL(debug) <<  "setProfile error";
    }

    test["tata"]="P2";
    if (!dbmanager->setProfile("dummy1","Profil numéro 2",test)) {
        BOOST_LOG_TRIVIAL(debug) <<  "setProfile error";
    }

    test["tata"]="P3";
    if (!dbmanager->setProfile("dummy1","Profil numéro 3",test)) {
        BOOST_LOG_TRIVIAL(debug) <<  "setProfile error";
    }
    test["tata"]="P4";
    if (!dbmanager->setProfile("dummy1","Profil numéro 4",test)) {
        BOOST_LOG_TRIVIAL(debug) <<  "setProfile error";
    }


    //QVariantMap test2;
    //if (!dbmanager->getProfile("titi","toto",test2)) {
    //    BOOST_LOG_TRIVIAL(debug) <<  "getProfile error";
    //} else {
    //    QJsonObject  obj2=QJsonObject::fromVariantMap(test2);
    //    QJsonDocument doc2(obj2);
    //    QByteArray docByteArray2 = doc2.toJson(QJsonDocument::Compact);
    //    QString strJson2 = QLatin1String(docByteArray2);
    //    BOOST_LOG_TRIVIAL(debug) <<  "getProfile result =" << strJson2.toStdString();
    //}





}


Controller::~Controller()
{
    /*m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());*/
}


void Controller::LoadModule(QString lib,QString name,QString label,QString profile)
{
    QLibrary library(lib);
    if (!library.load())
    {
        BOOST_LOG_TRIVIAL(debug) << name.toStdString() << " " << library.errorString().toStdString();
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << name.toStdString() << " " << "library loaded";

        typedef Basemodule *(*CreateModule)(QString,QString,QString);
        CreateModule createmodule = (CreateModule)library.resolve("initialize");
        if (createmodule) {
            Basemodule *mod = createmodule(name,label,profile);
            if (mod) {
                mod->setParent(this);
                mod->setHostport(_indihost,_indiport);
                mod->connectIndi();
                mod->setWebroot(_webroot);
                mod->setObjectName(name);
                connect(mod,&Basemodule::moduleEvent, this,&Controller::OnModuleEvent);
                connect(mod,&Basemodule::moduleEvent, wshandler,&WShandler::OnModuleEvent);
                connect(wshandler,&WShandler::dumpAsked,mod,&Basemodule::OnDumpAsked);

                QList<Basemodule *> othermodules = findChildren<Basemodule *>(QString(),Qt::FindChildrenRecursively);
                for (Basemodule *othermodule : othermodules) {
                    //BOOST_LOG_TRIVIAL(debug) << "child= " << othermodule->objectName().toStdString();
                    if (othermodule->getName()!=mod->getName()) {
                        connect(othermodule,&Basemodule::moduleEvent, mod,&Basemodule::OnExternalEvent);
                        connect(mod,&Basemodule::moduleEvent, othermodule,&Basemodule::OnExternalEvent);

                    }
                }
                QVariantMap prof;
                dbmanager->getProfile(mod->_moduletype,profile,prof);
                mod->setProfile(prof);

            }
        } else {
            BOOST_LOG_TRIVIAL(debug)  << "Could not initialize module from the loaded library";
        }
    }
}

//void Controller::OnPropertyCreated(Property *pProperty, QString *pModulename)
//{
//}
//
//void Controller::OnPropertyUpdated(Property *pProperty, QString *pModulename)
//{
//}
//void Controller::OnPropertyAppended(Property *pProperty, QString *pModulename)
//{
//}
//void Controller::OnPropertyRemoved(Property *pProperty, QString *pModulename)
//{
//}
void Controller::OnNewMessageSent(QString message, QString *pModulename, QString Device)
{
    BOOST_LOG_TRIVIAL(debug) << "MODULE " << pModulename->toStdString() << " DEVICE  "<< Device.toStdString() << " MESSAGE " << message.toStdString();
}
void Controller::OnLoadModule(QString lib, QString label,QString profile)
{
    QString name=label;
    name.replace(" ","");
    LoadModule(QCoreApplication::applicationDirPath()+"/"+lib,name,label,profile);
}

void Controller::OnModuleEvent(const QString &eventType, const QString &eventData)
{
    Basemodule *mod = qobject_cast<Basemodule *>(sender());
    //BOOST_LOG_TRIVIAL(debug) << "OnModuleEvent - SENDER = " << mod->getName().toStdString();
    //BOOST_LOG_TRIVIAL(debug) << "OnModuleEvent - EVENT TYPE = " <<  eventType.toStdString();

    //if (eventType=="profilerequest") {
    //    QVariantMap pro;
    //    if (!dbmanager->getProfile(pComplement.toMap()["moduletype"].toString(),pComplement.toMap()["profilename"].toString(),pro)) {
    //        BOOST_LOG_TRIVIAL(debug) <<  "getProfile error";
    //    } else {
    //        QJsonObject  obj2=QJsonObject::fromVariantMap(pro);
    //        QJsonDocument doc2(obj2);
    //        QByteArray docByteArray2 = doc2.toJson(QJsonDocument::Compact);
    //        QString strJson2 = QLatin1String(docByteArray2);
    //        BOOST_LOG_TRIVIAL(debug) <<  "getProfile result =" << strJson2.toStdString();
    //    }
    //
    //}


    QJsonObject obj;
    obj["evt"]=eventType;
    obj["mod"]=mod->getName();
    if (eventType=="moduledump") {
        obj["dta"]=QJsonObject::fromVariantMap(mod->getOstProperties());
    }
    if (eventType=="addprop"||eventType=="setpropvalue") {
        obj["dta"]=QJsonObject::fromVariantMap(mod->getOstProperty(eventData.toStdString().c_str()));
    }
    if (eventType=="delprop") {
        obj["dta"]=eventData;
    }


    QJsonDocument doc(obj);
    QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    //BOOST_LOG_TRIVIAL(debug) << "OnModuleEvent - " << mod->getName().toStdString() << " - " << eventType.toStdString() << " - " << strJson.toStdString();


}
