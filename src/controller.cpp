#include "controller.h"


/*!
 * ... ...
 */
Controller::Controller(QObject *parent, bool saveAllBlobs, const QString& host, int port,const QString& webroot)
    :_indihost(host),
      _indiport(port),
      _webroot(webroot)
{

    //this->setParent(parent);
    Q_UNUSED(saveAllBlobs);
    const QString DRIVER("QSQLITE");
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);
        db.setDatabaseName("/home/gilles/projets/OST/db/ost.db" );
        if(!db.open())
                    //qDebug() << "dbOpen - ERROR: " << db.lastError().text();
                    qDebug() << "dbOpen - ERROR: " << db.databaseName();// << db.lastError().text();
    }
    else
        qDebug() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";

    wshandler = new WShandler(this);

    BOOST_LOG_TRIVIAL(debug) << "Controller warmup";
    BOOST_LOG_TRIVIAL(debug) <<  "ApplicationDirPath :" << QCoreApplication::applicationDirPath().toStdString();

    MainControl *mainctl = new MainControl("maincontrol","Main control");
    mainctl->setHostport(_indihost,_indiport);
    mainctl->connectIndi();
    mainctl->setWebroot(_webroot);
    connect(mainctl,&Basemodule::propertyCreated,this,&Controller::OnPropertyCreated);
    connect(mainctl,&Basemodule::propertyUpdated,this,&Controller::OnPropertyUpdated);
    connect(mainctl,&Basemodule::propertyRemoved,this,&Controller::OnPropertyRemoved);
    connect(mainctl,&Basemodule::newMessageSent, this,&Controller::OnNewMessageSent);
    connect(mainctl,&Basemodule::moduleDumped, this,&Controller::OnModuleDumped);
    connect(mainctl,&MainControl::loadModule, this,&Controller::OnLoadModule);

    connect(mainctl,&Basemodule::propertyCreated,wshandler,&WShandler::OnPropertyCreated);
    connect(mainctl,&Basemodule::propertyUpdated,wshandler,&WShandler::OnPropertyUpdated);
    connect(mainctl,&Basemodule::propertyRemoved,wshandler,&WShandler::OnPropertyRemoved);
    connect(mainctl,&Basemodule::newMessageSent,wshandler,&WShandler::OnNewMessageSent);
    connect(mainctl,&Basemodule::moduleDumped, wshandler,&WShandler::OnModuleDumped);


    connect(wshandler,&WShandler::dumpAsked,mainctl,&Basemodule::OnDumpAsked);
    connect(wshandler,&WShandler::setPropertyText,mainctl,&Basemodule::OnSetPropertyText);
    connect(wshandler,&WShandler::setPropertyNumber,mainctl,&Basemodule::OnSetPropertyNumber);
    connect(wshandler,&WShandler::setPropertySwitch,mainctl,&Basemodule::OnSetPropertySwitch);


    //LoadModule(QCoreApplication::applicationDirPath()+"/libostguider.so","guider1","Guider");
    //LoadModule(QCoreApplication::applicationDirPath()+"/libostinspector.so","inspector1","Frame inspector");
    //LoadModule(QCoreApplication::applicationDirPath()+"/libostpolar.so","polar1","Polar assistant");
    //LoadModule(QCoreApplication::applicationDirPath()+"/libostfocuser.so","focus1","Focus assistant");
    LoadModule(QCoreApplication::applicationDirPath()+"/libostdummy.so","dummy1","Demo module");

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
            if (mod)
                mod->setHostport(_indihost,_indiport);
                mod->connectIndi();
                mod->setWebroot(_webroot);
                connect(mod,&Basemodule::propertyCreated,this,&Controller::OnPropertyCreated);
                connect(mod,&Basemodule::propertyUpdated,this,&Controller::OnPropertyUpdated);
                connect(mod,&Basemodule::propertyRemoved,this,&Controller::OnPropertyRemoved);
                connect(mod,&Basemodule::newMessageSent, this,&Controller::OnNewMessageSent);
                connect(mod,&Basemodule::moduleDumped, this,&Controller::OnModuleDumped);

                connect(mod,&Basemodule::propertyCreated,wshandler,&WShandler::OnPropertyCreated);
                connect(mod,&Basemodule::propertyUpdated,wshandler,&WShandler::OnPropertyUpdated);
                connect(mod,&Basemodule::propertyAppended,wshandler,&WShandler::OnPropertyAppended);
                connect(mod,&Basemodule::propertyRemoved,wshandler,&WShandler::OnPropertyRemoved);
                connect(mod,&Basemodule::newMessageSent,wshandler,&WShandler::OnNewMessageSent);
                connect(mod,&Basemodule::moduleDumped, wshandler,&WShandler::OnModuleDumped);
                mod->OnDumpAsked();

                connect(wshandler,&WShandler::dumpAsked,mod,&Basemodule::OnDumpAsked);
                connect(wshandler,&WShandler::setPropertyText,mod,&Basemodule::OnSetPropertyText);
                connect(wshandler,&WShandler::setPropertyNumber,mod,&Basemodule::OnSetPropertyNumber);
                connect(wshandler,&WShandler::setPropertySwitch,mod,&Basemodule::OnSetPropertySwitch);

                QVariantMap map = mod->property("ostproperties").toMap();
                if (map.size()==0) BOOST_LOG_TRIVIAL(debug) << "ostproperties reading problem";

                for (QVariant m : map)
                {
                    QVariantMap attribute = m.toMap();
                    QJsonObject obj =QJsonObject::fromVariantMap(attribute);
                    QJsonDocument doc(obj);
                    QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
                    QString strJson = QLatin1String(docByteArray);
                    //QJsonObject jsonobj = mod->property(obj["name"].toString().toStdString().c_str()).toJsonObject();
                    //QVariant val = jsonobj["value"].toVariant();
                    QVariant val = mod->property(attribute["name"].toString().toStdString().c_str());
                    BOOST_LOG_TRIVIAL(debug) << "ostproperties - " << attribute["name"].toString().toStdString() << " - " <<strJson.toStdString() << " = " << val.toString().toStdString();



                }
                connect(mod,&Basemodule::propertyChanged,this,Controller::OnPropertyChanged);

        } else {
            BOOST_LOG_TRIVIAL(debug)  << "Could not initialize module from the loaded library";
        }
    }
}

void Controller::OnPropertyCreated(Property *pProperty, QString *pModulename)
{
    PropertyTextDumper textDumper;
    pProperty->accept(&textDumper);
    //BOOST_LOG_TRIVIAL(debug) << "MODULE " << pModulename->toStdString() <<" CREATED " << textDumper.getResult();
}

void Controller::OnPropertyUpdated(Property *pProperty, QString *pModulename)
{
    PropertyTextDumper textDumper;
    pProperty->accept(&textDumper);
    //BOOST_LOG_TRIVIAL(debug) << "MODULE " << pModulename->toStdString() <<" UPDATED " << textDumper.getResult();
}
void Controller::OnPropertyAppended(Property *pProperty, QString *pModulename)
{
    PropertyTextDumper textDumper;
    pProperty->accept(&textDumper);
    //BOOST_LOG_TRIVIAL(debug) << "MODULE " << pModulename->toStdString() <<" UPDATED " << textDumper.getResult();
}
void Controller::OnPropertyRemoved(Property *pProperty, QString *pModulename)
{
    PropertyTextDumper textDumper;
    pProperty->accept(&textDumper);
    //BOOST_LOG_TRIVIAL(debug) << "MODULE " << pModulename->toStdString() <<" REMOVED" << textDumper.getResult();
}
void Controller::OnNewMessageSent(QString message, QString *pModulename, QString Device)
{
    BOOST_LOG_TRIVIAL(debug) << "MODULE " << pModulename->toStdString() << " DEVICE  "<< Device.toStdString() << " MESSAGE " << message.toStdString();
}
void Controller::OnModuleDumped(QMap<QString, QMap<QString, QMap<QString, Property *> > > treeList, QString* pModulename, QString* pModulelabel)
{
    //BOOST_LOG_TRIVIAL(debug) << "MODULE DUMPED " << pModulename->toStdString() << " size " << treeList.size();

    for ( const QString& device : treeList.keys() ) {
        //BOOST_LOG_TRIVIAL(debug) << "MODULE DUMPED " << pModulename->toStdString() << " device " << device.toStdString();
        for ( const QString& group : treeList[device].keys() ) {
            //BOOST_LOG_TRIVIAL(debug) << "MODULE DUMPED " << pModulename->toStdString() << " device " << device.toStdString() << " group " << group.toStdString();
            for ( const QString& property : treeList[device][group].keys() ) {
                //BOOST_LOG_TRIVIAL(debug) << "MODULE DUMPED " << pModulename->toStdString() << " device " << device.toStdString() << " group " << group.toStdString()<< " property " << property.toStdString();
                PropertyTextDumper textDumper;
                treeList[device][group][property]->accept(&textDumper);
                //BOOST_LOG_TRIVIAL(debug) << "MODULE " << pModulename->toStdString() <<" DUMPED " << textDumper.getResult();
            }
        }
    }
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
