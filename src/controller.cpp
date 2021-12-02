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

    MainControl *basemodule = new MainControl("maincontrol","Main control");
    basemodule->echoNameAndLabel();
    basemodule->setHostport(_indihost,_indiport);
    basemodule->connectIndi();
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


void Controller::OnValueChanged(double newValue)
{
    qDebug() << "*******************************" << newValue;

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
                //mod->echoNameAndLabel();
                mod->setHostport(_indihost,_indiport);
                mod->connectIndi();
                connect(mod,&Basemodule::propertyCreated,this,&Controller::OnPropertyCreated);
                connect(mod,&Basemodule::propertyCreated,wshandler,&WShandler::OnPropertyCreated);
                //connect(wshandler,&WShandler::changeValue,mod,&Basemodule::changeProp);
        } else {
            BOOST_LOG_TRIVIAL(debug)  << "Could not initialize module from the loaded library";
        }
    }
}

void Controller::OnPropertyCreated(Property *pProperty, QString *pModulename)
{
    PropertyTextDumper textDumper;
    pProperty->accept(&textDumper);
    BOOST_LOG_TRIVIAL(debug) << "MODULE " << pModulename->toStdString() <<"CREATED " << textDumper.getResult();
}

