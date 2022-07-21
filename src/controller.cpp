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
    const QString DRIVER("QSQLITE");
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);
        db.setDatabaseName(_dbpath+"ost.db" );
        if(!db.open())
                    //qDebug() << "dbOpen - ERROR: " << db.lastError().text();
                    qDebug() << "dbOpen - ERROR: " << db.databaseName();// << db.lastError().text();
    }
    else
        qDebug() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";

    wshandler = new WShandler(this);

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
    LoadModule(QCoreApplication::applicationDirPath()+"/libostdummy.so","dummy1","Demo module");
    LoadModule(QCoreApplication::applicationDirPath()+"/libostindipanel.so","indipanel","indi control panel");

}


Controller::~Controller()
{
    /*m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());*/
}


void Controller::LoadModule(QString lib,QString name,QString label)
{
    QLibrary library(lib);
    if (!library.load())
    {
        BOOST_LOG_TRIVIAL(debug) << name.toStdString() << " " << library.errorString().toStdString();
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << name.toStdString() << " " << "library loaded";

        typedef Basemodule *(*CreateModule)(QString,QString);
        CreateModule createmodule = (CreateModule)library.resolve("initialize");
        if (createmodule) {
            Basemodule *mod = createmodule(name,label);
            if (mod) {
                mod->setHostport(_indihost,_indiport);
                mod->connectIndi();
                mod->setWebroot(_webroot);
                connect(mod,&Basemodule::newMessageSent, this,&Controller::OnNewMessageSent);
                connect(mod,&Basemodule::moduleEvent, this,&Controller::OnModuleEvent);

                connect(mod,&Basemodule::newMessageSent,wshandler,&WShandler::OnNewMessageSent);
                connect(mod,&Basemodule::moduleDumped2, wshandler,&WShandler::OnModuleDumped2);

                mod->OnDumpAsked();

                connect(wshandler,&WShandler::dumpAsked,mod,&Basemodule::OnDumpAsked);
                //connect(wshandler,&WShandler::setPropertyText,mod,&Basemodule::OnSetPropertyText);
                //connect(wshandler,&WShandler::setPropertyNumber,mod,&Basemodule::OnSetPropertyNumber);
                //connect(wshandler,&WShandler::setPropertySwitch,mod,&Basemodule::OnSetPropertySwitch);

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
void Controller::OnLoadModule(QString lib, QString label)
{
    QString name=label;
    name.replace(" ","");
    LoadModule(QCoreApplication::applicationDirPath()+"/"+lib,name,label);
}

void Controller::OnPropertyChanged(QString *moduleName, QString *propName, QVariant *propValue)
{
    //BOOST_LOG_TRIVIAL(debug) << propValue->typeName();
    if (strcmp(propValue->typeName(),"QVariantMap")==0 )
    {
        QVariantMap map=propValue->toMap();
        QJsonObject obj =QJsonObject::fromVariantMap(map);
        QJsonDocument doc(obj);
        QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
        QString strJson = QLatin1String(docByteArray);
        BOOST_LOG_TRIVIAL(debug) << "OnDynamicPropertyChangeEvent - "  <<  moduleName->toStdString() << " - " << propName->toStdString() << " - " <<  strJson.toStdString();
    }


}
void Controller::OnModuleEvent(QString *pModulename, QString *eventType, QVariantMap *pEventData, QString *pFree)
{
    if (pEventData->isValid()) {
        //BOOST_LOG_TRIVIAL(debug) << "OnModuleEvent - " << pEventData->typeName() << "-" << pFree->toStdString() << "-" << pModulename->toStdString() << "-" << eventType->toStdString();
        //QVariantMap map=pEventData->toMap();
        QJsonObject obj =QJsonObject::fromVariantMap(pEventData);
        QJsonDocument doc(obj);
        QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
        QString strJson = QLatin1String(docByteArray);
        BOOST_LOG_TRIVIAL(debug) << "OnModuleEvent - " << pModulename->toStdString() << " - " << eventType->toStdString() << " - " << strJson.toStdString();
        //switch eventType {
        //    case:"propertyCreated" {
        //       BOOST_LOG_TRIVIAL(debug) << "OnDynamicPropertyChangeEvent - " ;
        //    }
        //
        //}

    } else {
        BOOST_LOG_TRIVIAL(debug) << "OnModuleEvent - INVALID DATA - " << eventType->toStdString() << " - " << pModulename->toStdString() << " - " << pFree->toStdString() ;
    }
}

