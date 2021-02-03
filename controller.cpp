#include <QApplication>
#include "controller.h"

/*!
 * ... ...
 */
Controller::Controller(QObject *parent)
{

    this->setParent(parent);

    indiclient = new MyClient(this);
    properties = new Properties(this);
    wshandler = new WShandler(this,properties);

    focuser = new MFocuser(indiclient,properties);
    connect(focuser,&MFocuser::signalvalueChanged,this,&Controller::valueChanged);
    connect(focuser,&MFocuser::signalpropCreated,this,&Controller::propCreated);
    connect(focuser,&MFocuser::signalpropDeleted,this,&Controller::propDeleted);
    connect(focuser,&MFocuser::signalAppendGraph,this,&Controller::AppendGraph);
    connect(wshandler,&WShandler::changeValue,focuser,&MFocuser::slotvalueChangedFromCtl);
    focuser->initProperties();

    mainctl = new MMainctl(indiclient,properties);
    connect(mainctl,&MMainctl::signalvalueChanged,this,&Controller::valueChanged);
    connect(mainctl,&MMainctl::signalpropCreated,this,&Controller::propCreated);
    connect(mainctl,&MMainctl::signalpropDeleted,this,&Controller::propDeleted);
    connect(mainctl,&MMainctl::signalAppendGraph,this,&Controller::AppendGraph);
    connect(wshandler,&WShandler::changeValue,mainctl,&MMainctl::slotvalueChangedFromCtl);
    mainctl->initProperties();

    navigator = new MNavigator(indiclient,properties);
    connect(navigator,&MNavigator::signalvalueChanged,this,&Controller::valueChanged);
    connect(navigator,&MNavigator::signalpropCreated,this,&Controller::propCreated);
    connect(navigator,&MNavigator::signalpropDeleted,this,&Controller::propDeleted);
    connect(navigator,&MNavigator::signalAppendGraph,this,&Controller::AppendGraph);
    connect(wshandler,&WShandler::changeValue,navigator,&MNavigator::slotvalueChangedFromCtl);
    navigator->initProperties();

    guider= new MGuider(indiclient,properties);
    connect(guider,&MGuider::signalvalueChanged,this,&Controller::valueChanged);
    connect(guider,&MGuider::signalpropCreated,this,&Controller::propCreated);
    connect(guider,&MGuider::signalpropDeleted,this,&Controller::propDeleted);
    connect(guider,&MGuider::signalAppendGraph,this,&Controller::AppendGraph);
    connect(wshandler,&WShandler::changeValue,guider,&MGuider::slotvalueChangedFromCtl);
    guider->initProperties();

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




