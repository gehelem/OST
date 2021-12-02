#include <QCoreApplication>
#include <QtCore>
#include "wshandler.h"
#include "jsonparser.h"
#include <boost/log/trivial.hpp>/*!
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
    BOOST_LOG_TRIVIAL(debug) <<"OST WS server listening";
}


WShandler::~WShandler()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void WShandler::OnPropertyCreated(Property *pProperty, QString *pModulename)
{
    JSonDumper jsonDumper;
    pProperty->accept(&jsonDumper);
    //BOOST_LOG_TRIVIAL(debug) << "JSON : " << jsonDumper.getResult().toStdString();
    QJsonObject  obj;
    obj["event"]="createproperty";
    obj["module"]=*pModulename;
    obj["property"]=jsonDumper.getJsonResult();
    sendJsonMessage(obj);

}
void WShandler::OnPropertyUpdated(Property *pProperty, QString *pModulename)
{
    JSonDumper jsonDumper;
    pProperty->accept(&jsonDumper);
    //BOOST_LOG_TRIVIAL(debug) << "JSON : " << jsonDumper.getResult().toStdString();
    QJsonObject  obj;
    obj["event"]="updateproperty";
    obj["module"]=*pModulename;
    obj["property"]=jsonDumper.getJsonResult();
    sendJsonMessage(obj);

}void WShandler::OnPropertyRemoved(Property *pProperty, QString *pModulename)
{
    JSonDumper jsonDumper;
    pProperty->accept(&jsonDumper);
    //BOOST_LOG_TRIVIAL(debug) << "JSON : " << jsonDumper.getResult().toStdString();
    QJsonObject  obj;
    obj["event"]="removeproperty";
    obj["module"]=*pModulename;
    obj["property"]=jsonDumper.getJsonResult();
    sendJsonMessage(obj);

}
void WShandler::OnNewMessageSent(QString message, QString *pModulename, QString *pDevice)
{
    QJsonObject  obj;
    obj["event"]="newmessage";
    obj["module"]=*pModulename;
    obj["device"]=*pDevice;
    obj["message"]=message;
    sendJsonMessage(obj);
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
    BOOST_LOG_TRIVIAL(debug) << "New WS client connection";
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    connect(pSocket, &QWebSocket::textMessageReceived, this, &WShandler::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WShandler::socketDisconnected);
    m_clients << pSocket;
}


void WShandler::processTextMessage(QString message)
{
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8()); // garder
    emit textRcv(message);
    QJsonObject  obj = jsonResponse.object(); // garder
    BOOST_LOG_TRIVIAL(debug) << "OST server received json" << message.toStdString();
    if (obj["message"].toString()=="readall")
    {
        //sendAll();
        //emit changeValue(Prop());

    }
    if (obj["message"].toString()=="readproperty")
    {
        //qDebug() << obj["modulename"].toString() <<obj["propertyname"].toString();
        //sendProperty(props->getProp(obj["modulename"].toString(),obj["propertyname"].toString()));
    }
    if (obj["message"].toString()=="updateproperty")
    {
        //QJsonObject prop = obj["property"].toObject();
        //sendProperty(props->getProp(obj["modulename"].toString(),obj["propertyname"].toString()));
        //emit changeValue(JpropToO(prop));
    }


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



