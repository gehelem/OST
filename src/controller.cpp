#include <QApplication>
#include <utils/propertytextdumper.h>
#include <utils/jsondumper.h>
#include "controller.h"

#include "boost/log/trivial.hpp"
#include "model/property.h"

/*!
 * ... ...
 */
Controller::Controller(QObject *parent, bool saveAllBlobs, const QString& host, int port)
:  indiclient(nullptr),
  _setup(Setup()),
  _appSettingsSaveEveryBlob(saveAllBlobs),
  _appSettingsHostName(host),
  _appSettingsServerPort(port)
{

    this->setParent(parent);

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

    resetClient();
    connectClient();
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

void Controller::onNewDeviceSeen(const QString &devcatName) {
    _setup.addDevcat(new Devcat(devcatName));
}

void Controller::onDeviceRemoved(const QString &devcatName) {
    _setup.removeDevcatByName(devcatName);
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

    PropertyTextDumper textDumper;
    pProperty->accept(&textDumper);
//    BOOST_LOG_TRIVIAL(debug) << "CREATED " << textDumper.getResult();

    JSonDumper jsonDumper;
    pProperty->accept(&jsonDumper);
//    BOOST_LOG_TRIVIAL(debug) << "JSON : " << jsonDumper.getResult();

    _propertyStore.add(pProperty);
    wshandler->sendmessage(QString::fromStdString(jsonDumper.getResult()));

    //dumpStore();
}

void Controller::onPropertyUpdated(Property *pProperty) {

    PropertyTextDumper textDumper;
    pProperty->accept(&textDumper);
    BOOST_LOG_TRIVIAL(debug) << "UPDATED " << textDumper.getResult();

    JSonDumper jsonDumper;
    pProperty->accept(&jsonDumper);
//    BOOST_LOG_TRIVIAL(debug) << "JSON : " << jsonDumper.getResult();

    _propertyStore.update(pProperty);
    wshandler->sendmessage(QString::fromStdString(jsonDumper.getResult()));

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

void Controller::onPropertyRemoved(Property *pProperty) {

    BOOST_LOG_TRIVIAL(debug) << "Property removed : "
    << pProperty->getDevcatName().toStdString() << '|'
    << pProperty->getGroupName().toStdString() << '|'
    << pProperty->getName().toStdString();

    _propertyStore.remove(pProperty);

    dumpStore();
}

void Controller::onIndiConnected() {

    BOOST_LOG_TRIVIAL(info) << "Connected to INDI server";
}

void Controller::onIndiDisconnected() {

    BOOST_LOG_TRIVIAL(info) << "Lost connection to INDI server";
    _propertyStore.cleanup();
    _setup.cleanup();
    resetClient();
    connectClient();
}

void Controller::resetClient() {

    delete indiclient;
    indiclient = new IndiCLient();
    connect(indiclient, &IndiCLient::newDeviceSeen, this, &Controller::onNewDeviceSeen);
    connect(indiclient, &IndiCLient::deviceRemoved, this, &Controller::onDeviceRemoved);
    connect(indiclient, &IndiCLient::newBlobReceived, this, &Controller::onNewBlobReveived);
    connect(indiclient, &IndiCLient::newPropertyReceived, this, &Controller::onNewPropertyReceived);
    connect(indiclient, &IndiCLient::propertyUpdated, this, &Controller::onPropertyUpdated);
    connect(indiclient, &IndiCLient::propertyRemoved, this, &Controller::onPropertyRemoved);
    connect(indiclient, &IndiCLient::messageReceived, this, &Controller::onMessageReceived);
    connect(indiclient, &IndiCLient::indiConnected, this, &Controller::onIndiConnected);
    connect(indiclient, &IndiCLient::indiDisconnected, this, &Controller::onIndiDisconnected);
    indiclient->setServer(_appSettingsHostName.toStdString().c_str(), _appSettingsServerPort);
}

void Controller::connectClient() {

    static const int retry_period_in_seconds = 5;
    BOOST_LOG_TRIVIAL(debug) << "Tring to connect to INDI server...";
    bool connected = indiclient->connectServer();

    while (!connected) {
        BOOST_LOG_TRIVIAL(debug) << "Waiting " << retry_period_in_seconds << "s before retrying to connect to INDI server";
        sleep(retry_period_in_seconds);
        connected = indiclient->connectServer();
    }
}


void Controller::dumpStore() {

    BOOST_LOG_TRIVIAL(debug) << "*** STORE DUMP START ***";

    PropertyTextDumper dumper;

    QList<Property*> allProps = _propertyStore.toList();

    for (Property* pProperty : allProps) {
        pProperty->accept(&dumper);
        BOOST_LOG_TRIVIAL(debug) << dumper.getResult();
    }

    BOOST_LOG_TRIVIAL(debug) << "Store size: " << allProps.size();
    BOOST_LOG_TRIVIAL(debug) << "*** STORE DUMP END ***";
}
