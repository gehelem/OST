#include "OSTController.h"
//#include <baseclientqt.h>
#include <vector>
#include <string>



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
    IDLog("Echoserver new connection\n");

    connect(pSocket, &QWebSocket::textMessageReceived, this, &OSTController::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &OSTController::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &OSTController::socketDisconnected);

    m_clients << pSocket;
    //MyOSTClientPAN->givemeall();
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
    sendjson(tINDItoJSON(prop));
}
void OSTController::emitnewnumber(INumberVectorProperty *prop) {
    sendjson(nINDItoJSON(prop));
}
void OSTController::emitnewswitch(ISwitchVectorProperty *prop) {
    sendjson(sINDItoJSON(prop));
}
void OSTController::emitnewlight(ILightVectorProperty *prop) {
    sendjson(lINDItoJSON(prop));
}
void OSTController::emitnewdevice(INDI::BaseDevice *d) {
    sendjson(dINDItoJSON(d));
}
void OSTController::emitall(void){
    QJsonArray pros,devs,numbers,texts,swits,lights,groups;
    QJsonObject pro,dev,rt,number,text,swit,light,group;
    rt=QJsonObject();
    devs=QJsonArray();

    /*for (int i=0;i<MyOSTClientPAN->getDevices().size();i++) {
        INDI::BaseDevice *d = MyOSTClientPAN->getDevices()[i];
        dev=QJsonObject();
        groups=QJsonArray();
        dev["DeviceName"]=d->getDeviceName();
        dev["Groups"]=groups;
        devs.append(dev);
    }
    rt["devices"]=devs;

    for (int i=0;i<MyOSTClientPAN->getDevices().size();i++) {
        INDI::BaseDevice *d = MyOSTClientPAN->getDevices()[i];
        std::vector<INDI::Property *> allprops = *d->getProperties();
        for (int j=0;j<allprops.size();j++) {

        }
    }
    rt["devices"]=devs;*/


    for (int i=0;i<MyOSTClientPAN->getDevices().size();i++) {
        INDI::BaseDevice *d = MyOSTClientPAN->getDevices()[i];
        dev=QJsonObject();
        dev["DeviceName"]=d->getDeviceName();
        std::vector<INDI::Property *> allprops = *d->getProperties();
        pros=QJsonArray();
        for (int j=0;j<allprops.size();j++) {
            pro=QJsonObject();
            pro["Name"]=allprops[j]->getName();
            pro["Label"]=allprops[j]->getLabel();
            pro["GroupName"]=allprops[j]->getGroupName();
            switch(allprops[j]->getState()){
            case IPS_IDLE:
                pro["State"]="IPS_IDLE";
                break;
            case IPS_OK:
                pro["State"]="IPS_OK";
                break;
            case IPS_BUSY:
                pro["State"]="IPS_BUSY";
                break;
            case IPS_ALERT:
                pro["State"]="IPS_ALERT";
                break;
            }

            switch(allprops[j]->getPermission()){
            case IP_RO:
                pro["Permission"]="IP_RO";
                break;
            case IP_WO:
                pro["Permission"]="IP_WO";
                break;
            case IP_RW:
                pro["Permission"]="IP_RW";
                break;
            }

            switch (allprops[j]->getType()) {

            case INDI_NUMBER:
                pro["Type"]="INDI_NUMBER";
                numbers=QJsonArray();
                for (int k=0;k<allprops[j]->getNumber()->nnp;k++) {
                    number=QJsonObject();
                    number["value"]=allprops[j]->getNumber()->np[k].value;
                    number["name"]=allprops[j]->getNumber()->np[k].name;
                    number["label"]=allprops[j]->getNumber()->np[k].label;
                    number["format"]=allprops[j]->getNumber()->np[k].format;
                    number["min"]=allprops[j]->getNumber()->np[k].min;
                    number["max"]=allprops[j]->getNumber()->np[k].max;
                    number["step"]=allprops[j]->getNumber()->np[k].step;
                    numbers.append(number);
                }
                pro["Numbers"]=numbers;
                break;

            case INDI_TEXT:
                pro["Type"]="INDI_TEXT";
                texts=QJsonArray();
                for (int k=0;k<allprops[j]->getText()->ntp;k++) {
                    text=QJsonObject();
                    text["name"]=allprops[j]->getText()->tp[k].name;
                    text["label"]=allprops[j]->getText()->tp[k].label;
                    text["text"]=allprops[j]->getText()->tp[k].text;
                    texts.append(text);
                }
                pro["Texts"]=texts;
                break;
            case INDI_LIGHT:
                pro["Type"]="INDI_LIGHT";
                lights=QJsonArray();
                for (int k=0;k<allprops[j]->getLight()->nlp;k++) {
                    light=QJsonObject();
                    light["name"]=allprops[j]->getLight()->lp[k].name;
                    light["label"]=allprops[j]->getLight()->lp[k].label;
                    switch (allprops[j]->getLight()->lp[k].s) {
                    case IPS_IDLE:
                        light["light"]="IPS_IDLE";
                        break;
                    case IPS_OK:
                        light["light"]="IPS_OK";
                        break;
                    case IPS_BUSY:
                        light["light"]="IPS_BUSY";
                        break;
                    case IPS_ALERT:
                        light["light"]="IPS_ALERT";
                        break;
                    }
                    lights.append(light);
                }
                pro["Lights"]=lights;
                break;
            case INDI_SWITCH:
                pro["Type"]="INDI_SWITCH";
                switch (allprops[j]->getSwitch()->r){
                case ISR_1OFMANY:
                    pro["Rule"] ="ISR_1OFMANY";
                    break;
                case ISR_ATMOST1:
                    pro["Rule"] ="ISR_ATMOST1";
                    break;
                case ISR_NOFMANY:
                    pro["Rule"] ="ISR_NOFMANY";
                    break;
                }
                swits=QJsonArray();
                for (int k=0;k<allprops[j]->getSwitch()->nsp;k++) {
                    swit=QJsonObject();
                    swit["name"]=allprops[j]->getSwitch()->sp[k].name;
                    swit["label"]=allprops[j]->getSwitch()->sp[k].label;
                    switch (allprops[j]->getSwitch()->sp[k].s) {
                    case ISS_ON:
                        swit["switch"]="ISS_ON";
                        break;
                    case ISS_OFF:
                        swit["switch"]="ISS_OFF";
                        break;
                    }
                    swits.append(swit);
                }
                pro["Switchs"]=swits;
                break;
            }
            pros.append(pro);

            dev["Properties"]=pros;
        }
        devs.append(dev);
    }
    rt["Devices"]=devs;

    QJsonDocument jsondoc(rt);
    sendjson(jsondoc);
}

QJsonDocument  OSTController::nINDItoJSON(INumberVectorProperty *prop) {
    QJsonObject json;
    QJsonArray numbers;
    json["dev"]="indi";
    json["basedevice"]="fakebasedevice";
    json["name"]      =prop->name;
    json["label"]     =prop->label;
    json["group"] =prop->group;
    json["device"]=prop->device;
    json["timestamp"] =prop->timestamp;
    json["type"]="INDI_NUMBER";
    switch(prop->p){
        case IP_RO:
            json["perm"]="IP_RO";
            break;
        case IP_WO:
            json["perm"]="IP_WO";
            break;
        case IP_RW:
            json["perm"]="IP_RW";
            break;
    }

    for (int i=0;i<prop->nnp;i++) {
        QJsonObject number;
        number["value"]=prop->np[i].value;
        number["name"]=prop->np[i].name;
        number["label"]=prop->np[i].label;
        number["format"]=prop->np[i].format;
        number["min"]=prop->np[i].min;
        number["max"]=prop->np[i].max;
        number["step"]=prop->np[i].step;
        numbers.append(number);
    };
    json["number"]=numbers;

    QJsonDocument jsondoc(json);
    return jsondoc;
}


QJsonDocument  OSTController::tINDItoJSON(ITextVectorProperty *prop) {
    QJsonObject json;
    QJsonArray texts;
    json["dev"]="indi";
    json["basedevice"]="fakebasedevice";
    json["name"]      =prop->name;
    json["label"]     =prop->label;
    json["group"] =prop->group;
    json["device"]=prop->device;
    json["timestamp"] =prop->timestamp;
    json["type"]="INDI_TEXT";
    switch(prop->p){
        case IP_RO:
            json["perm"]="IP_RO";
            break;
        case IP_WO:
            json["perm"]="IP_WO";
            break;
        case IP_RW:
            json["perm"]="IP_RW";
            break;
    }


    for (int i=0;i<prop->ntp;i++) {
        QJsonObject text;
        text["name"]=prop->tp[i].name;
        text["label"]=prop->tp[i].label;
        text["text"]=prop->tp[i].text;
        texts.append(text);
    };
    json["text"]=texts;

    QJsonDocument jsondoc(json);
    return jsondoc;
}

QJsonDocument  OSTController::sINDItoJSON(ISwitchVectorProperty *prop) {
    QJsonObject json;
    QJsonArray switchs;
    json["dev"]="indi";
    json["basedevice"]="fakebasedevice";
    json["name"]      =prop->name;
    json["label"]     =prop->label;
    json["group"] =prop->group;
    json["device"]=prop->device;
    json["timestamp"] =prop->timestamp;
    json["type"]="INDI_SWITCH";
    switch(prop->p){
        case IP_RO:
            json["perm"]="IP_RO";
            break;
        case IP_WO:
            json["perm"]="IP_WO";
            break;
        case IP_RW:
            json["perm"]="IP_RW";
            break;
    }


    for (int i=0;i<prop->nsp;i++) {
        QJsonObject swit;
        swit["name"]=prop->sp[i].name;
        swit["label"]=prop->sp[i].label;
        switch (prop->sp[i].s) {
            case ISS_ON:
                swit["s"]="ISS_ON";
                break;
            case ISS_OFF:
                swit["s"]="ISS_OFF";
                break;
        }
        switchs.append(swit);
    };
    json["switch"]=switchs;

    QJsonDocument jsondoc(json);
    return jsondoc;
}


QJsonDocument  OSTController::lINDItoJSON(ILightVectorProperty *prop) {
    QJsonObject json;
    QJsonArray lights;
    json["dev"]="indi";
    json["basedevice"]="fakebasedevice";
    json["name"]      =prop->name;
    json["label"]     =prop->label;
    json["group"]     =prop->group;
    json["device"]    =prop->device;
    json["timestamp"] =prop->timestamp;
    json["type"]="INDI_LIGHT";


    for (int i=0;i<prop->nlp;i++) {
        QJsonObject light;
        light["name"]=prop->lp[i].name;
        light["label"]=prop->lp[i].label;
        switch (prop->lp[i].s) {
            case IPS_IDLE:
                light["s"]="IPS_IDLE";
                break;
            case IPS_OK:
                light["s"]="IPS_OK";
                break;
            case IPS_BUSY:
                light["s"]="IPS_BUSY";
                break;
            case IPS_ALERT:
                light["s"]="IPS_ALERT";
                break;
        }
        lights.append(light);
    };
    json["lights"]=lights;

    QJsonDocument jsondoc(json);
    return jsondoc;
}
QJsonDocument  OSTController::dINDItoJSON(INDI::BaseDevice *d) {
    QJsonObject json;
    json["basedevice"]="fakebasedevice";
    json["DeviceName"]=d->getDeviceName();
    QJsonDocument jsondoc(json);
    return jsondoc;
}
