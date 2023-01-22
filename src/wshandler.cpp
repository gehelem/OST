#include <QCoreApplication>
#include <QtCore>
#include "wshandler.h"
#include <boost/log/trivial.hpp>

/*!

 * ... ...
 */
WShandler::WShandler(QObject *parent)
{

    //this->setParent(parent);
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("OST server"), QWebSocketServer::NonSecureMode, this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, 9624))
    {
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WShandler::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WShandler::closed);
    }
    BOOST_LOG_TRIVIAL(debug) << "OST WS server listening";
}


WShandler::~WShandler()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void WShandler::sendmessage(QString message)
{
    for (int i = 0; i < m_clients.size(); i++)
    {
        QWebSocket *pClient = m_clients[i];
        if (pClient) pClient->sendTextMessage(message);
    }

}
void WShandler::sendbinary(QByteArray *data)
{
    for (int i = 0; i < m_clients.size(); i++)
    {
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
    QString _mess = message.replace("\\\"", "\"");
    _mess = _mess.replace("}\"", "}");
    _mess = _mess.replace("\"{", "{");
    QJsonDocument jsonResponse = QJsonDocument::fromJson(_mess.toUtf8()); // garder
    emit textRcv(message);
    QJsonObject  obj = jsonResponse.object(); // garder
    //BOOST_LOG_TRIVIAL(debug) << "OST server received json" << message.toStdString();
    if (obj["evt"].toString() == "readall")
    {
        //sendAll();
        //emit changeValue(Prop());
        emit externalEvent("readall", "*", "*", QVariantMap());

    }
    if (obj["evt"].toString() == "setproperty")
    {
        emit externalEvent("setproperty", obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }


}



void WShandler::sendJsonMessage(QJsonObject json)
{
    QJsonDocument jsondoc;
    jsondoc.setObject(json);
    //QString strJson(jsondoc.toJson(QJsonDocument::Indented)); // version lisible
    QString strJson(jsondoc.toJson(QJsonDocument::Compact)); // version compactée
    sendmessage(strJson);
    //BOOST_LOG_TRIVIAL(debug) << "WS handler sends : " << strJson.toStdString();
}

void WShandler::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug("OST server received binary message");

    if (pClient)
    {
        pClient->sendBinaryMessage(message);
    }
}

void WShandler::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug("OST client disconnected");
    if (pClient)
    {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void WShandler::processModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                                   const QVariantMap &eventData)
{
    QJsonObject  obj;
    obj["evt"] = eventType;
    //obj["mod"]=eventModule;

    if (eventType == "moduledump")
    {
        QJsonObject mods;
        QJsonObject  mod;
        mods[eventModule] = QJsonObject::fromVariantMap(eventData);
        obj["modules"] = mods;
        sendJsonMessage(obj);
    }
    if (eventType == "addprop" || eventType == "addelt" || eventType == "setattributes")
    {
        QJsonObject mods;
        QJsonObject  mod;
        QJsonObject  prop;
        prop[eventKey] = QJsonObject::fromVariantMap(eventData);
        mod["properties"] = prop;
        mods[eventModule] = mod;
        obj["modules"] = mods;
        sendJsonMessage(obj);
    }
    if (eventType == "delprop")
    {
        QJsonObject mods;
        QJsonObject  mod;
        QJsonObject  prop;
        prop[eventKey] = QJsonObject();
        mod["properties"] = prop;

        mods[eventModule] = mod;
        obj["modules"] = mods;
        sendJsonMessage(obj);
    }
    if (eventType == "pushvalues")
    {
        QVariantMap values;
        QVariantMap prop;
        QJsonObject props;
        QJsonObject mod;
        QJsonObject mods;
        props[eventKey] = QJsonObject::fromVariantMap(eventData);
        mod["properties"] = props;
        mods[eventModule] = mod;
        obj["modules"] = mods;
        sendJsonMessage(obj);
    }
    if (eventType == "resetvalues")
    {
        QVariantMap values;
        QVariantMap prop;
        QJsonObject props;
        QJsonObject mod;
        QJsonObject mods;
        props[eventKey] = QJsonObject::fromVariantMap(prop);
        mod["properties"] = props;
        mods[eventModule] = mod;
        obj["modules"] = mods;
        sendJsonMessage(obj);
    }
    if (eventType == "setpropvalue")
    {
        QVariantMap prop = eventData;
        QVariantMap values;
        if (prop.contains("value"))
        {
            values["value"] = prop["value"];
        }
        if (prop.contains("status"))
        {
            values["status"] = prop["status"];
        }
        if (prop.contains("elements"))
        {
            QVariantMap elements;
            foreach(const QString &key, prop["elements"].toMap().keys())
            {
                QVariantMap element;
                element["value"] = prop["elements"].toMap()[key].toMap()["value"];
                if (prop["elements"].toMap()[key].toMap().contains("min"))
                {
                    element["min"] = prop["elements"].toMap()[key].toMap()["min"];
                }
                if (prop["elements"].toMap()[key].toMap().contains("max"))
                {
                    element["max"] = prop["elements"].toMap()[key].toMap()["max"];
                }
                if (prop["elements"].toMap()[key].toMap().contains("step"))
                {
                    element["step"] = prop["elements"].toMap()[key].toMap()["step"];
                }
                elements[key] = element;
            }
            values["elements"] = elements;
        }
        QJsonObject props;
        props[eventKey] = QJsonObject::fromVariantMap(values);
        QJsonObject mods;
        QJsonObject  mod;
        mod["properties"] = props;
        mods[eventModule] = mod;
        obj["modules"] = mods;
        sendJsonMessage(obj);
    }

}
