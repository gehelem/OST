#include <QApplication>
#include "controller.h"
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
//#include <QtSql/QSqlError>
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
    indipanel = new IndiPanel();
    focuser = new FocusModule();
    QMap<QString,QString> dev;
    dev["camera"]="CCD Simulator";
    dev["focuser"]="Focuser Simulator";
    focuser->setDevices(dev);
    //focuser->setProperty("modulename","focuser of the death");
    connect(wshandler,&WShandler::textRcv,focuser,&FocusModule::test0);
    connect(focuser,&FocusModule::valueChanged,this,&Controller::OnValueChanged);
    connect(indipanel,&IndiPanel::valueChanged,this,&Controller::OnValueChanged);
    connect(properties,&Properties::signalvalueChanged,this,&Controller::valueChanged);
    connect(properties,&Properties::signalPropCreated,wshandler,&WShandler::sendProperty);

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



