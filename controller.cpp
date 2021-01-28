#include <QApplication>
#include "controller.h"

/*!
 * ... ...
 */
Controller::Controller(QObject *parent)
{

    this->setParent(parent);

    wshandler = new WShandler(this);
    indiclient = new MyClient(this);
    properties = new Properties(this);

    focuser = new MFocuser(indiclient,properties);
    connect(focuser,&MFocuser::signalvalueChanged,this,&Controller::valueChanged);
    connect(focuser,&MFocuser::signalpropCreated,this,&Controller::propCreated);
    connect(focuser,&MFocuser::signalpropDeleted,this,&Controller::propDeleted);
    connect(wshandler,&WShandler::changeValue,focuser,&MFocuser::slotvalueChangedFromCtl);
    focuser->initProperties();

    mainctl = new MMainctl(indiclient,properties);
    connect(mainctl,&MMainctl::signalvalueChanged,this,&Controller::valueChanged);
    connect(mainctl,&MMainctl::signalpropCreated,this,&Controller::propCreated);
    connect(mainctl,&MMainctl::signalpropDeleted,this,&Controller::propDeleted);
    connect(wshandler,&WShandler::changeValue,mainctl,&MMainctl::slotvalueChangedFromCtl);
    mainctl->initProperties();

    navigator = new MNavigator(indiclient,properties);
    connect(navigator,&MNavigator::signalvalueChanged,this,&Controller::valueChanged);
    connect(navigator,&MNavigator::signalpropCreated,this,&Controller::propCreated);
    connect(navigator,&MNavigator::signalpropDeleted,this,&Controller::propDeleted);
    connect(wshandler,&WShandler::changeValue,navigator,&MNavigator::slotvalueChangedFromCtl);
    navigator->initProperties();

    properties->dumproperties();
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

void Controller::propCreated(Prop prop)
{
    wshandler->sendmessage("New prop " +  prop.propname);
}
void Controller::propDeleted(Prop prop)
{
    wshandler->sendmessage("Del prop " +  prop.propname);
}




