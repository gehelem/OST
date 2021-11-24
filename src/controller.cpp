#include "controller.h"


/*!
 * ... ...
 */
Controller::Controller(QObject *parent)
{

    this->setParent(parent);

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

    //indiclient=IndiCLient::getInstance();
    properties=Properties::getInstance();

    wshandler = new WShandler(this,properties);
    QMap<QString,QString> dev;
    dev["camera"]="CCD Simulator";
    dev["focuser"]="Focuser Simulator";
    //focuser->setProperty("modulename","focuser of the death");
    BOOST_LOG_TRIVIAL(debug) << "Controller warmup";
    BOOST_LOG_TRIVIAL(debug) <<  "ApplicationDirPath :" << QCoreApplication::applicationDirPath().toStdString();
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

        typedef Basemodule *(*CreateModule)();
        CreateModule createmodule = (CreateModule)library.resolve("initialize");
        if (createmodule) {
            Basemodule *mod = createmodule();
            if (mod)
                mod->setNameAndLabel(name,label);
                mod->echoNameAndLabel();
                //connect(mod,&Basemodule::NewModuleMessage,this,&Controller::OnNewModuleMessage);
                //connect(mod,&Basemodule::newMessage )

        } else {
            BOOST_LOG_TRIVIAL(debug)  << "Could not initialize module from the loaded library";
        }
    }
}


