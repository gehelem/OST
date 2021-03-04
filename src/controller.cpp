#include <QApplication>
#include "controller.h"

#include "boost/log/trivial.hpp"

/*!
 * ... ...
 */
Controller::Controller(QObject *parent)
: _setup(Setup()){

    this->setParent(parent);

    indiclient=IndiCLient::getInstance();

    wshandler = new WShandler(this,properties);

    focuser = new FocusModule();
    QMap<QString,QString> dev;
    dev["camera"]="CCD Simulator";
    dev["focuser"]="Focuser Simulator";
    focuser->setDevices(dev);
    //focuser->setProperty("modulename","focuser of the death");
    connect(wshandler,&WShandler::textRcv,focuser,&FocusModule::test0);
    /*focuser2 = new FocusModule();
    connect(wshandler,&WShandler::textRcv,focuser2,&FocusModule::test0);*/
    const QMetaObject *metaobject = focuser->metaObject();
    int count = metaobject->propertyCount();
    for (int i=0; i<count; ++i) {
        QMetaProperty metaproperty = metaobject->property(i);
        qDebug() << "focus props " <<  metaproperty.name() <<  metaproperty.isReadable() <<  metaproperty.isWritable() << metaproperty.type();
    }

    bool clientConnected = indiclient->connectServer();
    if ( ! clientConnected ) {
        BOOST_LOG_TRIVIAL(warning) << "Could not connect to INDI server. Won't do much for now... Please start INDI server and restart OST";
    }
    connect(indiclient, &IndiCLient::newDeviceSeen, this, &Controller::onNewDeviceSeen);
    connect(indiclient, &IndiCLient::deviceRemoved, this, &Controller::onDeviceRemoved);
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

void Controller::onNewDeviceSeen(const std::string &deviceName) {
    _setup.addDevice(new Device(deviceName));
}

void Controller::onDeviceRemoved(const std::string &deviceName) {
    _setup.removeDeviceByName(deviceName);
}




