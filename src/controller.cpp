#include <QApplication>
#include "controller.h"

/*!
 * ... ...
 */
Controller::Controller(QObject *parent)
{

    this->setParent(parent);

//    indiclient = MyClient::getInstance();

    wshandler = new WShandler(this,properties);

    focuser = new FocusModule();
    connect(wshandler,&WShandler::textRcv,focuser,&FocusModule::test0);


    //properties->dumproperties();

}


Controller::~Controller()
{
    /*m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());*/
}

void Controller::valueChanged(Prop prop)
{
    wshandler->sendProperty(prop);
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




