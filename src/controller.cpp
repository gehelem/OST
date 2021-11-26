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

    properties=Properties::getInstance();
    wshandler = new WShandler(this,properties);
    connect(properties,&Properties::signalPropCreated,this,&Controller::propCreated);
    connect(properties,&Properties::signalPropDeleted,this,&Controller::propDeleted);
    connect(properties,&Properties::signalvalueChanged,this,&Controller::valueChanged);

    BOOST_LOG_TRIVIAL(debug) << "Controller warmup";
    BOOST_LOG_TRIVIAL(debug) <<  "ApplicationDirPath :" << QCoreApplication::applicationDirPath().toStdString();
    LoadModule(QCoreApplication::applicationDirPath()+"/libmaincontrol.so","maincontrol","maincontrol1");
    LoadModule(QCoreApplication::applicationDirPath()+"/libfocuser.so","focuser1","focuser 1");
    LoadModule(QCoreApplication::applicationDirPath()+"/libindipanel.so","indipanel1","indipanel 1");



}


Controller::~Controller()
{
    /*m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());*/
}

void Controller::valueChanged(Prop prop)
{
    wshandler->updateElements(prop);
}
void Controller::AppendGraph (Prop prop,OGraph gra,OGraphValue val)
{
    wshandler->sendGraphValue(prop,gra,val);
}

void Controller::propCreated(Prop prop)
{
    wshandler->sendmessage("New prop " +  prop.propname);
}
void Controller::propDeleted(Prop prop)
{
    wshandler->sendmessage("Del prop " +  prop.propname);
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
                mod->echoNameAndLabel();
                //connect(mod,&Basemodule::NewModuleMessage,this,&Controller::OnNewModuleMessage);
                //connect(mod,&Basemodule::newMessage )
                mod->setHostport(_indihost,_indiport);
                mod->connectIndi();

        } else {
            BOOST_LOG_TRIVIAL(debug)  << "Could not initialize module from the loaded library";
        }
    }
}


