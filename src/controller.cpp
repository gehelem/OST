#include <QApplication>
#include "controller.h"

#include "boost/log/trivial.hpp"
#include "model/property.h"

/*!
 * ... ...
 */
Controller::Controller(QObject *parent, bool saveAllBlobs, const QString& host, int port)
: _setup(Setup()),
  _appSettingsSaveEveryBlob(saveAllBlobs),
  _appSettingsHostName(host),
  _appSettingsServerPort(port)
{

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

    indiclient->setServer(_appSettingsHostName.toStdString().c_str(), _appSettingsServerPort);
    bool clientConnected = indiclient->connectServer();
    if ( ! clientConnected ) {
        BOOST_LOG_TRIVIAL(warning) << "Could not connect to INDI server. Won't do much for now... Please start INDI server and restart OST";
    }
    connect(indiclient, &IndiCLient::newDeviceSeen, this, &Controller::onNewDeviceSeen);
    connect(indiclient, &IndiCLient::deviceRemoved, this, &Controller::onDeviceRemoved);
    connect(indiclient, &IndiCLient::newBlobReceived, this, &Controller::onNewBlobReveived);
    connect(indiclient, &IndiCLient::newPropertyReceived, this, &Controller::onNewPropertyReceived);
    connect(indiclient, &IndiCLient::propertyUpdated, this, &Controller::onPropertyUpdated);
    connect(indiclient, &IndiCLient::messageReceived, this, &Controller::onMessageReceived);
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

void Controller::onNewDeviceSeen(const QString &deviceName) {
    _setup.addDevice(new Device(deviceName));
}

void Controller::onDeviceRemoved(const QString &deviceName) {
    _setup.removeDeviceByName(deviceName);
}

void Controller::onNewBlobReveived(const QByteArray& data, const QString& format) const {

    if ( _appSettingsSaveEveryBlob ) {
        QFile outputFile(QString("/tmp/ost_blob_test").append(format));
        bool saveSuccess = false;
        if (outputFile.open(QIODevice::WriteOnly)) {
            qint64 writtenLength = outputFile.write(data);
            if (-1 != writtenLength) {
                saveSuccess = true;
            }
            outputFile.close();
        }
        if (!saveSuccess) {
            BOOST_LOG_TRIVIAL(error) << "could not save BLOG to file: " << outputFile.fileName().toStdString();
        } else {
            BOOST_LOG_TRIVIAL(info) << "Saved BLOG to file: " << outputFile.fileName().toStdString();
        }
    }

}

void Controller::onNewPropertyReceived(Property *pProperty) {

    PropertyLogger logger(false);
    pProperty->accept(&logger);
    // TODO do something with this new property
    delete pProperty;
}

void Controller::onPropertyUpdated(Property *pProperty) {

    PropertyLogger logger;
    pProperty->accept(&logger);
    // TODO do something with this updated property
    delete pProperty;
}

void Controller::onMessageReceived(const QString& message) {

    QString fullMessage = "Message received: " + message;

    if ( message.contains("[ERROR]") ) {
        BOOST_LOG_TRIVIAL(error) << fullMessage.toStdString();
    } else if ( message.contains("[WARNING]") ) {
        BOOST_LOG_TRIVIAL(warning) << fullMessage.toStdString();
    } else if ( message.contains("[INFO]") ) {
        BOOST_LOG_TRIVIAL(info) << fullMessage.toStdString();
    }
}




