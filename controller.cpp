#include <QApplication>
#include "controller.h"

/*!
 * ... ...
 */
Controller::Controller(QObject *parent)
{

    this->setParent(parent);

    indiclient = new MyClient(this);
    wshandler= new WShandler(this);
    focuser = new MFocuser(indiclient);
    connect(focuser,&MFocuser::signalvalueChanged,this,&Controller::valueChanged);
    connect(focuser,&MFocuser::signalpropCreated,this,&Controller::propCreated);
    connect(focuser,&MFocuser::signalpropDeleted,this,&Controller::propDeleted);
    connect(wshandler,&WShandler::changeValue,focuser,&MFocuser::slotvalueChangedFromCtl);
    focuser->initProperties();

    mainctl = new MMainctl(indiclient);
    connect(mainctl,&MMainctl::signalvalueChanged,this,&Controller::valueChanged);
    connect(mainctl,&MMainctl::signalpropCreated,this,&Controller::propCreated);
    connect(mainctl,&MMainctl::signalpropDeleted,this,&Controller::propDeleted);
    connect(wshandler,&WShandler::changeValue,mainctl,&MMainctl::slotvalueChangedFromCtl);
    mainctl->initProperties();

    navigator = new MNavigator(indiclient);
    connect(navigator,&MNavigator::signalvalueChanged,this,&Controller::valueChanged);
    connect(navigator,&MNavigator::signalpropCreated,this,&Controller::propCreated);
    connect(navigator,&MNavigator::signalpropDeleted,this,&Controller::propDeleted);
    connect(wshandler,&WShandler::changeValue,navigator,&MNavigator::slotvalueChangedFromCtl);
    navigator->initProperties();

}


Controller::~Controller()
{
    /*m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());*/
}

void Controller::valueChanged(elem elt)
{
    wshandler->sendElement(elt);
}

void Controller::propCreated(elem elt)
{
    wshandler->sendmessage("New prop " +  elt.name);
}
void Controller::propDeleted(elem elt)
{
    wshandler->sendmessage("Del prop " +  elt.name);
}




