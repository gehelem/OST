#include "controller.h"


/*!
 * ... ...
 */
Controller::Controller(QObject *parent, bool saveAllBlobs, const QString& host, int port)
    :_indihost(host),
      _indiport(port)
{

    this->setParent(parent);
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
    mainctl->echoNameAndLabel();
    mainctl->setHostport(_indihost,_indiport);
    mainctl->connectIndi();
    connect(mainctl,&Basemodule::propertyCreated,this,&Controller::OnPropertyCreated);
    connect(mainctl,&Basemodule::propertyUpdated,this,&Controller::OnPropertyUpdated);
    connect(mainctl,&Basemodule::propertyRemoved,this,&Controller::OnPropertyRemoved);
    connect(mainctl,&Basemodule::newMessageSent, this,&Controller::OnNewMessageSent);
    connect(mainctl,&Basemodule::propertyCreated,wshandler,&WShandler::OnPropertyCreated);
    connect(mainctl,&Basemodule::propertyUpdated,wshandler,&WShandler::OnPropertyUpdated);
    connect(mainctl,&Basemodule::propertyRemoved,wshandler,&WShandler::OnPropertyRemoved);
    connect(mainctl,&Basemodule::newMessageSent,wshandler,&WShandler::OnNewMessageSent);



    QDir directory(QCoreApplication::applicationDirPath());
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList() << "*ost*.so");
    _availableModuleLibs = directory.entryList();
    foreach(QString lib, _availableModuleLibs)
    {
        BOOST_LOG_TRIVIAL(debug) << "Module lib found " << lib.toStdString();
    }

    //LoadModule(QCoreApplication::applicationDirPath()+"/libostfocuser.so","focuser1","focuser 1");
    LoadModule(QCoreApplication::applicationDirPath()+"/libostindipanel.so","indipanel1","indipanel 1");

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
                connect(mod,&Basemodule::propertyCreated,this,&Controller::OnPropertyCreated);
                connect(mod,&Basemodule::propertyUpdated,this,&Controller::OnPropertyUpdated);
                connect(mod,&Basemodule::propertyRemoved,this,&Controller::OnPropertyRemoved);
                connect(mod,&Basemodule::newMessageSent, this,&Controller::OnNewMessageSent);
                connect(mod,&Basemodule::propertyCreated,wshandler,&WShandler::OnPropertyCreated);
                connect(mod,&Basemodule::propertyUpdated,wshandler,&WShandler::OnPropertyUpdated);
                connect(mod,&Basemodule::propertyRemoved,wshandler,&WShandler::OnPropertyRemoved);
                connect(mod,&Basemodule::newMessageSent,wshandler,&WShandler::OnNewMessageSent);

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
void Controller::OnPropertyRemoved(Property *pProperty, QString *pModulename)
{
    PropertyTextDumper textDumper;
    pProperty->accept(&textDumper);
    //BOOST_LOG_TRIVIAL(debug) << "MODULE " << pModulename->toStdString() <<" REMOVED" << textDumper.getResult();
}
void Controller::OnNewMessageSent(QString message, QString *pModulename, QString *pDevice)
{
    BOOST_LOG_TRIVIAL(debug) << "MODULE " << pModulename->toStdString() << " DEVICE  "<< pDevice->toStdString() << " MESSAGE " << message.toStdString();
}
