#include <QCoreApplication>
#include <QtCore>
#include <QFile>
#include <QSslConfiguration>
#include "wshandler.h"

/*!

 * ... ...
 */
WShandler::WShandler(QObject *parent, const QString &ssl, const QString &sslCert, const QString &sslKey,
                     const QString &grant): mServerGrant(grant)
{

    //this->setParent(parent);
    Q_UNUSED(parent);

    QWebSocketServer::SslMode sslMode = (ssl != "N") ? QWebSocketServer::SecureMode : QWebSocketServer::NonSecureMode;
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("OST server"), sslMode, this);

    // Configure SSL if enabled
    if (sslMode == QWebSocketServer::SecureMode)
    {
        QSslConfiguration sslConfiguration;

        // Load certificate
        QFile certFile(sslCert);
        if (!certFile.open(QIODevice::ReadOnly))
        {
            sendMessage("ERROR: Cannot open SSL certificate file: " + sslCert);
        }
        else
        {
            QSslCertificate certificate(&certFile, QSsl::Pem);
            certFile.close();

            if (certificate.isNull())
            {
                sendMessage("ERROR: Invalid SSL certificate in: " + sslCert);
            }
            else
            {
                sslConfiguration.setLocalCertificate(certificate);
                sendMessage("SSL certificate loaded: " + sslCert);
            }
        }

        // Load private key
        QFile keyFile(sslKey);
        if (!keyFile.open(QIODevice::ReadOnly))
        {
            sendMessage("ERROR: Cannot open SSL private key file: " + sslKey);
        }
        else
        {
            QSslKey privateKey(&keyFile, QSsl::Rsa, QSsl::Pem);
            keyFile.close();

            if (privateKey.isNull())
            {
                sendMessage("ERROR: Invalid SSL private key in: " + sslKey);
            }
            else
            {
                sslConfiguration.setPrivateKey(privateKey);
                sendMessage("SSL private key loaded: " + sslKey);
            }
        }

        // Set SSL protocol version
        sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
        sslConfiguration.setProtocol(QSsl::TlsV1_2OrLater);

        m_pWebSocketServer->setSslConfiguration(sslConfiguration);
    }

    if (m_pWebSocketServer->listen(QHostAddress::Any, 9624))
    {
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WShandler::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WShandler::closed);

        if (sslMode == QWebSocketServer::SecureMode)
        {
            sendMessage("OST WS server listening on port 9624 with SSL/TLS enabled");
        }
        else
        {
            sendMessage("OST WS server listening on port 9624 (non-secure mode)");
        }
    }
    else
    {
        sendMessage("ERROR: Failed to start OST WS server on port 9624");
    }
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
    sendMessage("New WS client connection");
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    connect(pSocket, &QWebSocket::textMessageReceived, this, &WShandler::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WShandler::socketDisconnected);
    qDebug() << pSocket << "-" << pSocket->peerAddress().toString();
    m_clients << pSocket;
    mClientGrants[pSocket->peerAddress().toString()] = "0";

}


void WShandler::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    QString clientGrant = mClientGrants[pClient->peerAddress().toString()];
    if ((clientGrant != "1") && (clientGrant != "2") && (clientGrant != "pending") && ((mServerGrant == "1")
            || (mServerGrant == "2")))
    {
        sendMessage("request identification " + pClient->peerAddress().toString());
        QFile jsonFile(":loginpage.json");
        if (jsonFile.open(QFile::ReadOnly))
        {
            QJsonDocument d = QJsonDocument().fromJson(jsonFile.readAll());
            jsonFile.close();
            mClientGrants[pClient->peerAddress().toString()] = "pending";
            pClient->sendTextMessage(d.toJson());
        };
        return;
    }

    QString _mess = message.replace("\\\"", "\"");
    _mess = _mess.replace("}\"", "}");
    _mess = _mess.replace("\"{", "{");
    QJsonDocument jsonResponse = QJsonDocument::fromJson(_mess.toUtf8()); // garder
    QJsonObject  obj = jsonResponse.object(); // garder
    sendMessage("OST server received json" + message);

    if (clientGrant == "pending")
    {
        if (
            (obj["evt"].toString() == "Fsetproperty")
            &&    (obj["mod"].toString() == "mainctl")
            && obj["dta"].toVariant().toMap().contains("login")
        )
        {
            QVariant v = obj["dta"].toVariant().toMap()["login"];
            QVariant Elts = v.toMap()["elements"];
            QString user = Elts.toMap()["user"].toString();
            QString pw = Elts.toMap()["pw"].toString();
            QString g = dbmanager->getGrants(user, pw);
            if ((g == "1") || (g == "2"))
            {
                mClientGrants[pClient->peerAddress().toString()] = g;
                emit externalEvent("Freadall", "*", "*", QVariantMap());
            }

        }
        return;
    }

    if (obj["evt"].toString() == "Freadall")
    {
        emit externalEvent("Freadall", "*", "*", QVariantMap());
    }
    /* ignore update messages from readonly users */
    if (clientGrant == 1) return;

    if (obj["evt"].toString() == "Fsetproperty")
    {
        emit externalEvent("Fsetproperty", obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }
    if (obj["evt"].toString() == "Flcreate")
    {
        emit externalEvent("Flcreate", obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }
    if (obj["evt"].toString() == "Flupdate")
    {
        emit externalEvent("Flupdate", obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }
    if (obj["evt"].toString() == "Fldelete")
    {
        emit externalEvent("Fldelete", obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }
    if (obj["evt"].toString() == "Flselect")
    {
        emit externalEvent("Flselect", obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }
    if (obj["evt"].toString() == "Flup")
    {
        emit externalEvent("Flup", obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }
    if (obj["evt"].toString() == "Fldown")
    {
        emit externalEvent("Fldown", obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }
    if (obj["evt"].toString() == "Fclearmessages")
    {
        emit externalEvent("Fclearmessages", obj["mod"].toString(), QString(), QVariantMap());
    }
    if ((obj["evt"].toString() == "Fpreicon") || (obj["evt"].toString() == "Fposticon") )
    {
        emit externalEvent(obj["evt"].toString(), obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }
    if ((obj["evt"].toString() == "Fbadge"))
    {
        emit externalEvent(obj["evt"].toString(), obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }
    if ((obj["evt"].toString() == "Fproppreicon1") || (obj["evt"].toString() == "Fproppreicon2")
            || (obj["evt"].toString() == "Fpropposticon1") || (obj["evt"].toString() == "Fpropposticon2"))
    {
        emit externalEvent(obj["evt"].toString(), obj["mod"].toString(), obj["key"].toString(), obj["dta"].toVariant().toMap());
    }
    if ((obj["evt"].toString() == "Ffolderselect"))
    {
        emit externalEvent(obj["evt"].toString(), QString(), obj["folder"].toString(), QVariantMap());
    }


}



void WShandler::sendJsonMessage(QJsonObject json)
{
    QJsonDocument jsondoc;
    jsondoc.setObject(json);
    //QString strJson(jsondoc.toJson(QJsonDocument::Indented)); // version lisible
    QString strJson(jsondoc.toJson(QJsonDocument::Compact)); // version compactée
    sendmessage(strJson);
    //sendMessage("WS handler sends : " + strJson);
}

void WShandler::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    sendMessage("OST server received binary message");

    if (pClient)
    {
        pClient->sendBinaryMessage(message);
    }
}

void WShandler::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    sendMessage("OST client disconnected");
    qDebug() << pClient << "-" << pClient->peerAddress().toString();

    if (pClient)
    {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
        mClientGrants.remove(pClient->peerAddress().toString());

    }
}

void WShandler::processModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                                   const QVariantMap &eventData)
{
    QJsonObject  obj;
    obj["evt"] = eventType;

    if (eventType == "moduledump")
    {
        QJsonObject mods;
        QJsonObject  mod;
        mods[eventModule] = QJsonObject::fromVariantMap(eventData);
        obj["modules"] = mods;
        sendJsonMessage(obj);
    }
    if (eventType == "cp" || eventType == "ce" || eventType == "ap" || eventType == "ae")
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
    if (eventType == "lc")
    {
        QJsonObject mods;
        QJsonObject  mod;
        QJsonObject  lov;
        lov[eventKey] = QJsonObject::fromVariantMap(eventData);
        mod["globallovs"] = lov;
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
    if (eventType == "sp" || eventType == "se")
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
        if (prop.contains("enabled"))
        {
            values["enabled"] = prop["enabled"];
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
                if (prop["elements"].toMap()[key].toMap().contains("dynlabel"))
                {
                    element["dynlabel"] = prop["elements"].toMap()[key].toMap()["dynlabel"];
                }
                if (prop["elements"].toMap()[key].toMap().contains("type"))
                {
                    if (prop["elements"].toMap()[key].toMap()["type"] == "img")
                    {
                        element = prop["elements"].toMap()[key].toMap();
                    }
                    if (prop["elements"].toMap()[key].toMap()["type"] == "video")
                    {
                        element = prop["elements"].toMap()[key].toMap();
                    }
                    if (prop["elements"].toMap()[key].toMap()["type"] == "date")
                    {
                        element = prop["elements"].toMap()[key].toMap();
                    }
                    if (prop["elements"].toMap()[key].toMap()["type"] == "time")
                    {
                        element = prop["elements"].toMap()[key].toMap();
                    }
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
    if (eventType == "mm")
    {
        QJsonObject mod;
        QJsonObject mods;
        mod["message"] = QJsonObject::fromVariantMap(eventData);;
        mods[eventModule] = mod;
        obj["modules"] = mods;
        sendJsonMessage(obj);
    }
    if (eventType == "me")
    {
        QJsonObject mod;
        QJsonObject mods;
        mod["error"] = QJsonObject::fromVariantMap(eventData);;
        mods[eventModule] = mod;
        obj["modules"] = mods;
        sendJsonMessage(obj);
    }
    if (eventType == "mw")
    {
        QJsonObject mod;
        QJsonObject mods;
        mod["warning"] = QJsonObject::fromVariantMap(eventData);;
        mods[eventModule] = mod;
        obj["modules"] = mods;
        sendJsonMessage(obj);
    }


}
void WShandler::sendMessage(const QString &pMessage)
{
    QString messageWithDateTime = "[" + QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + "]-" + pMessage;
    QDebug debug = qDebug();
    debug.noquote();
    debug << messageWithDateTime;
}
void WShandler::processFileEvent(const QString &eventType, const QStringList &eventData)
{
    QJsonObject  obj;
    obj["evt"] = eventType;
    obj["fileevent"] = QJsonArray::fromStringList(eventData);
    sendJsonMessage(obj);
}
