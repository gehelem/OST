#include <QApplication>
#include <QtCore>
#include "wshandler.h"

/*!
 * ... ...
 */
WShandler::WShandler(QObject *parent)
{

    this->setParent(parent);
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
    if (obj["message"].toString()=="clic")
    {
        elem el;
        el.module = obj["module"].toString();
        el.name = obj["name"].toString();
        el.type = ET_BTN;
        emit changeValue(el);
    }

}
void WShandler::sendProperty(pro prop)
{
    QJsonObject mess,obj,det;
    QJsonArray dets;
}
void WShandler::sendElement(elem elt)
{
    QJsonObject mess,obj,det;
    QJsonArray dets;
    obj["propertyname"]=elt.name;
    obj["label"]=elt.label;

    obj["permission"]="IP_RW";
    obj["state"]="IPS_IDLE";
    obj["parenttype"]="M"; // M=module/ C=Category / G=group
    obj["parentname"]="";
    obj["modulename"]=elt.module;
    obj["categoryname"]="";
    obj["groupname"]="";


    if (elt.type==ET_TEXT) {
        obj["type"]="INDI_TEXT";
        det["label"]=elt.label;
        det["textname"]=elt.name;
        det["text"]=elt.text;
        dets.append(det);
        obj["texts"]=dets;
    }
    if (elt.type==ET_NUM) {
        obj["type"]="INDI_NUMBER";
        det["label"]=elt.label;
        det["numbername"]=elt.name;
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
        det["label"]=elt.label;
        det["switchname"]=elt.name;
        if (elt.sw) det["switch"]="ISS_ON";
        if (!elt.sw) det["switch"]="ISS_OFF";
        dets.append(det);
        obj["switchs"]=dets;
    }
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



