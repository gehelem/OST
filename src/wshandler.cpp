#include <QCoreApplication>
#include <QtCore>
#include "wshandler.h"
#include <boost/log/trivial.hpp>
#include "basemodule.h"
/*!

 * ... ...
 */
WShandler::WShandler(QObject *parent)
{

    //this->setParent(parent);
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

//void WShandler::OnPropertyCreated(Property *pProperty, QString *pModulename)
//{
//}
//void WShandler::OnPropertyUpdated(Property *pProperty, QString *pModulename)
//{
//}
//void WShandler::OnPropertyAppended(Property *pProperty, QString *pModulename, double s, double x,double y,double z,double k)
//{
//}
//void WShandler::OnPropertyRemoved(Property *pProperty, QString *pModulename)
//{
//}
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
        emit dumpAsked();

    }
    if (obj["message"].toString()=="readproperty")
    {
        //qDebug() << obj["modulename"].toString() <<obj["propertyname"].toString();
        //sendProperty(props->getProp(obj["modulename"].toString(),obj["propertyname"].toString()));
    }
    if (obj["message"].toString()=="setproperty")
    {
        //QJsonObject prop = obj["property"].toObject();
        //if (prop["texts"].isArray()) emit setPropertyText(&jsonDumper.setProTextFromJson(prop));
        //if (prop["numbers"].isArray()) emit setPropertyNumber(&jsonDumper.setProNumberFromJson(prop));
        //if (prop["switches"].isArray()) emit setPropertySwitch(&jsonDumper.setProSwitchFromJson(prop));
    }


}



void WShandler::sendJsonMessage(QJsonObject json)
{
    QJsonDocument jsondoc;
    jsondoc.setObject(json);
    //QString strJson(jsondoc.toJson(QJsonDocument::Indented)); // version lisible
    QString strJson(jsondoc.toJson(QJsonDocument::Compact)); // version compactée
    sendmessage(strJson);
    BOOST_LOG_TRIVIAL(debug) << "WS handler sends : " << strJson.toStdString();
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

void WShandler::OnModuleEvent(const QString &eventType, const QString &eventData)
{
        Basemodule *mod = qobject_cast<Basemodule *>(sender());

        QJsonObject  obj;
        obj["evt"]=eventType;
        obj["mod"]=mod->getName();
        if (eventType=="moduledump") {
            obj["dta"]=QJsonObject::fromVariantMap(mod->getOstProperties());
        }
        if (eventType=="addprop"||eventType=="addelt") {
            QJsonObject  property;
            property[eventData]=QJsonObject::fromVariantMap(mod->getOstProperty(eventData.toStdString().c_str()));
            obj["dta"]=property;
        }
        if (eventType=="delprop") {
            obj["dta"]=eventData;
        }
        if (eventType=="setpropvalue") {
           QVariantMap prop = mod->getOstProperty(eventData.toStdString().c_str());
           QVariantMap values;
           if (prop.contains("value")) {
               values["value"]=prop["value"];
           }
           if (prop.contains("elements")) {
               QVariantMap elements;
               foreach(const QString& key, prop["elements"].toMap().keys()) {
                   QVariantMap element;
                   element["value"]=prop["elements"].toMap()[key].toMap()["value"];
                   elements[key]=element;
               }
               values["elements"]=elements;
            }
            QJsonObject  property;
            property[eventData]=QJsonObject::fromVariantMap(values);
            obj["dta"]=property;

        }

        sendJsonMessage(obj);
}
