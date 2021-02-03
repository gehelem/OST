#include <QApplication>
#include <QtCore>
#include "wshandler.h"
#include "jsonparser.h"
/*!
 * ... ...
 */
WShandler::WShandler(QObject *parent,Properties *properties)
{

    this->setParent(parent);
    props = properties;
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("OST server"),QWebSocketServer::NonSecureMode, this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, 9624)) {
       connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &WShandler::onNewConnection);
       connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WShandler::closed);
    }
    qDebug("OST server listening");

}


WShandler::~WShandler()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}


void WShandler::sendmessage(QString message)
{
    for (int i=0;i<m_clients.size();i++) {
        QWebSocket *pClient = m_clients[i];
        if (pClient) pClient->sendTextMessage(message);
    }

}
void WShandler::sendbinary(QByteArray *data)
{
    for (int i=0;i<m_clients.size();i++) {
        QWebSocket *pClient = m_clients[i];
        if (pClient) pClient->sendBinaryMessage(*data);
    }
}


void WShandler::onNewConnection()
{
    qDebug("New client connection");
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    connect(pSocket, &QWebSocket::textMessageReceived, this, &WShandler::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WShandler::socketDisconnected);
    m_clients << pSocket;
}


void WShandler::processTextMessage(QString message)
{
    qDebug() << "OST server received text message" << message;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8()); // garder
    QJsonObject  obj = jsonResponse.object(); // garder
    if (obj["message"].toString()=="readall")
    {
        sendAll();
    }
    if (obj["message"].toString()=="readproperty")
    {
        //qDebug() << obj["modulename"].toString() <<obj["propertyname"].toString();
        sendProperty(props->getProp(obj["modulename"].toString(),obj["propertyname"].toString()));
    }
    if (obj["message"].toString()=="updateproperty")
    {
        QJsonObject prop = obj["property"].toObject();
        //sendProperty(props->getProp(obj["modulename"].toString(),obj["propertyname"].toString()));
        emit changeValue(JpropToO(prop));
    }


}
void WShandler::sendProperty(Prop prop)
{
    QJsonObject mess,obj,det;
    QJsonArray dets;
    obj=OpropToJ(prop);
    mess["message"]="updateproperty";
    mess["property"]=obj;
    sendJsonMessage(mess);
}

void WShandler::sendAll(void)
{
    QJsonObject mess;
    QJsonArray dets;
    mess["message"]="updateall";
    for(auto m : props->getModules()){
        dets.append(OmodToJ(props->getModule(m.modulename)));
    }
    mess["modules"]=dets;
    sendJsonMessage(mess);
}
void WShandler::sendGraphValue(Prop prop,OGraph gra,OGraphValue val)
{
    QJsonObject mess;
    QJsonArray dets;
    mess["message"]="appendgraph";
    mess["module"]=prop.modulename;
    mess["property"]=prop.propname;
    mess["graph"]=gra.name;
    mess["v0"]=val.v0;
    mess["v1"]=val.v1;
    mess["v2"]=val.v2;
    mess["v3"]=val.v3;
    mess["v4"]=val.v4;

    sendJsonMessage(mess);
}


void WShandler::sendElement(Prop prop)
{
    QJsonObject mess,obj,det;
    QJsonArray dets;
    /*obj["propertyname"]=elt.elemname;
    obj["label"]=elt.elemlabel;

    obj["permission"]="IP_RW";
    obj["state"]="IPS_IDLE";
    obj["parenttype"]="M"; // M=module/ C=Category / G=group
    obj["parentname"]="";
    obj["modulename"]=elt.modulename;
    obj["categname"]="";
    obj["groupname"]="";


    if (elt.type==ET_TEXT) {
        obj["type"]="INDI_TEXT";
        det["label"]=elt.elemlabel;
        det["textname"]=elt.elemname;
        det["text"]=elt.text;
        dets.append(det);
        obj["texts"]=dets;
    }
    if (elt.type==ET_NUM) {
        obj["type"]="INDI_NUMBER";
        det["label"]=elt.elemlabel;
        det["numbername"]=elt.elemname;
        det["value"]=elt.num;
        det["format"]="";
        det["min"]=0;
        det["max"]=0;
        det["step"]=0;
        dets.append(det);
        obj["numbers"]=dets;
    }
    if (elt.type==ET_BOOL) {
        obj["type"]="INDI_SWITCH";
        det["label"]=elt.elemlabel;
        det["switchname"]=elt.elemname;
        if (elt.sw) det["switch"]="ISS_ON";
        if (!elt.sw) det["switch"]="ISS_OFF";
        dets.append(det);
        obj["switchs"]=dets;
    }*/
    mess["message"]="updateproperty";
    mess["property"]=obj;
    sendJsonMessage(mess);
}
void WShandler::sendJsonMessage(QJsonObject json)
{
    QJsonDocument jsondoc;
    jsondoc.setObject(json);
    //QString strJson(jsondoc.toJson(QJsonDocument::Indented)); // version lisible
    QString strJson(jsondoc.toJson(QJsonDocument::Compact)); // version compactée
    sendmessage(strJson);
}

void WShandler::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug("OST server received binary message");

    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void WShandler::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug("OST client disconnected");
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}



