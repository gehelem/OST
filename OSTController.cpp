#include "OSTController.h"
//#include <baseclientqt.h>
#include <vector>
#include <string>
#include "OSTParser.h"


OSTController::OSTController(QObject *parent)
{
    //OSTClientGEN *MyOSTClientGEN = new OSTClientGEN(parent);

    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("Echo Server"),QWebSocketServer::NonSecureMode, this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, 9624)) {
       IDLog("Echoserver listening on port %i\n",9624);
       connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &OSTController::onNewConnection);
       connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &OSTController::closed);
    }


    MyOSTClientFOC.reset(new OSTClientFOC(parent));
    MyOSTClientPAN.reset(new OSTClientPAN(parent));
    //MyOSTClientFOC.reset(new OSTClientFOC());
    MyOSTClientFOC->setClientname("focuser");
    MyOSTClientPAN->setClientname("indipanel");
    MyOSTClientFOC->LogLevel=OSTLOG_NONE;
    MyOSTClientPAN->LogLevel=OSTLOG_ALL;
    MyOSTClientFOC->setOSTDevices("CCD Simulator","Focuser Simulator","","","");
    MyOSTClientPAN->setOSTDevices("CCD Simulator","Focuser Simulator","Telescope Simulator","Filter Simulator","Guide Simulator");
    MyOSTClientFOC->setServer("localhost", 7624);
    MyOSTClientPAN->setServer("localhost", 7624);
    MyOSTClientFOC->connectServer();
    MyOSTClientPAN->connectServer();
    MyOSTClientFOC->getDevices();
    MyOSTClientPAN->getDevices();
    //server = new EchoServer(1234, true);
    //connect(server        , &EchoServer::startf     , this, &OSTController::startf   );
    QObject::connect(MyOSTClientFOC.get(), &OSTClientFOC::focusdone, this, &OSTController::focusdone);
    //connect(img.get(),&OSTImage::success,this,&OSTClientFOC::sssuccess);
    /*QObject::connect(MyOSTClientPAN.get(), &OSTClientPAN::emitnewprop, this, &OSTController::emitnewprop);
    QObject::connect(MyOSTClientPAN.get(), &OSTClientPAN::emitnewtext, this, &OSTController::emitnewtext);
    QObject::connect(MyOSTClientPAN.get(), &OSTClientPAN::emitnewnumber, this, &OSTController::emitnewnumber);
    QObject::connect(MyOSTClientPAN.get(), &OSTClientPAN::emitnewswitch, this, &OSTController::emitnewswitch);
    QObject::connect(MyOSTClientPAN.get(), &OSTClientPAN::emitnewlight, this, &OSTController::emitnewlight);
    QObject::connect(MyOSTClientPAN.get(), &OSTClientPAN::emitnewdevice, this, &OSTController::emitnewdevice);*/

}

OSTController::~OSTController()
{
    IDLog("OSTController delete\n");
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
    //delete MyOSTClientFOC;
    //delete MyOSTClientGEN;
}
void OSTController::focusdone(void)
{

    IDLog("OSTController saying focus is done\n");
    QWebSocket *pClient = m_clients[0];
    if (pClient) {
        pClient->sendTextMessage("focus done");
    }

}

void OSTController::sendmessage(QString message)
{
    for (int i=0;i<m_clients.size();i++) {
        QWebSocket *pClient = m_clients[i];
        if (pClient) pClient->sendTextMessage(message);
    }

}
void OSTController::sendjson(QJsonDocument json)
{
    //QString strJson(json.toJson(QJsonDocument::Compact)); // version compactée
    QString strJson(json.toJson(QJsonDocument::Indented));
    sendmessage(strJson);
}


void OSTController::startf(void)
{

    IDLog("OSTController asking to start focus\n");
    MyOSTClientFOC->startFocusing(35000,100,100,30);

}

void OSTController::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    IDLog("new client connection\n");

    connect(pSocket, &QWebSocket::textMessageReceived, this, &OSTController::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &OSTController::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &OSTController::socketDisconnected);

    m_clients << pSocket;
    emitall();
}


void OSTController::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    /*QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject obj = doc.object();
    QVariantMap mainMap = obj.toVariantMap();
    QVariantMap dataMap = mainMap[1].toMap();*/

    IDLog("Message received %s\n",message.toUtf8());

    if (pClient) {
//        pClient->sendTextMessage(message);
    }

}

void OSTController::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("Echoserver received binary message\n");

    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void OSTController::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("Echoserver disconnect\n");
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void OSTController::emitnewprop(INDI::Property *property) {
    //IDLog("emit new prop %s\n",property->getName());
}
void OSTController::emitnewtext(ITextVectorProperty *prop) {
    //sendjson(tINDItoJSON(prop));
}
void OSTController::emitnewnumber(INumberVectorProperty *prop) {
    //sendjson(nINDItoJSON(prop));
}
void OSTController::emitnewswitch(ISwitchVectorProperty *prop) {
    //sendjson(sINDItoJSON(prop));
}
void OSTController::emitnewlight(ILightVectorProperty *prop) {
    //sendjson(lINDItoJSON(prop));
}
void OSTController::emitnewdevice(INDI::BaseDevice *d) {
    //sendjson(dINDItoJSON(d));
}
void OSTController::emitall(void){
    QJsonObject dev,client,grp,prop,num;
    QJsonArray alldev,allclient,allgrp,allprop,allnum;
    allclient = QJsonArray();
    client=QJsonObject();
    client["ClientName"]="OSTCLientPAN";
    client["ClientLabel"]="INDI control panel";

    for (int i=0;i<MyOSTClientPAN->getDevices().size();i++) {
        dev=QJsonObject();
        dev["DeviceName"]=MyOSTClientPAN->getDevices()[i]->getDeviceName();
        dev["groups"]=IDevToJson(MyOSTClientPAN->getDevices()[i]);
        alldev.append(dev);
    }
    client["devices"]=alldev;
    allclient.append(client);

    client=QJsonObject();
    client["ClientName"]="MAINCONTROL";
    client["ClientLabel"]="Main control";
    dev=QJsonObject();
    dev["DeviceName"]="Global parameters";
    grp=QJsonObject();
    allgrp=QJsonArray();
    grp["GroupName"]="Optics";

    prop=QJsonObject();
    allprop=QJsonArray();
    prop["Name"]="SCOPE";
    prop["Label"]="Main scope values";
    prop["Permission"]="IP_RW";
    prop["State"]="IPS_IDLE";
    prop["Type"]="INDI_NUMBER";

    allnum=QJsonArray();
    num=QJsonObject();
    num["name"]="MFL";
    num["label"]="Focal length (mm)";
    num["value"]=800;
    allnum.append(num);
    num=QJsonObject();
    num["name"]="MAP";
    num["label"]="Diameter (mm)";
    num["value"]=200;
    allnum.append(num);
    prop["Numbers"]=allnum;
    allprop.append(prop);

    prop=QJsonObject();
    prop["Name"]="GUIDE";
    prop["Label"]="Guide scope values";
    prop["Permission"]="IP_RW";
    prop["State"]="IPS_IDLE";
    prop["Type"]="INDI_NUMBER";

    allnum=QJsonArray();
    num=QJsonObject();
    num["name"]="GFL";
    num["label"]="Focal length (mm)";
    num["value"]=250;
    allnum.append(num);
    num=QJsonObject();
    num["name"]="GAP";
    num["label"]="Diameter (mm)";
    num["value"]=60;
    allnum.append(num);
    prop["Numbers"]=allnum;
    allprop.append(prop);


    grp["properties"]=allprop;
    allgrp.append(grp);
    client["groups"]=allgrp;
    allclient.append(client);



    client=QJsonObject();
    client["ClientName"]="OSTCLientFOC";
    client["ClientLabel"]="Focuser";
    //client["devices"]=alldev; ???
    allclient.append(client);

    client=QJsonObject();
    client["ClientName"]="OSTCLientSEQ";
    client["ClientLabel"]="Sequencer";
    //client["devices"]=alldev; ???
    allclient.append(client);

    QJsonObject rt;
    rt=QJsonObject();
    rt["Clients"]=allclient;
    QJsonDocument jsondoc(rt);
    sendjson(jsondoc);

    //sendjson(IProToJson(p));
}


