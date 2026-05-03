#include <QCoreApplication>
#include <QtCore>
#include <QFile>
#include <QSslConfiguration>
#include <QJsonDocument>
#include "wshandler.h"
#include "translatemanager.h"
#include "model/element/common.h"
#include <lovjsondumper.h>
#include "libs/utils/modulejsondumper.h"
/*!

 * ... ...
 */
WShandler::WShandler(QObject *parent, const QString &ssl, const QString &sslCert, const QString &sslKey,
                     const QString &grant, const QString &lng): mServerGrant(grant), mLng(lng)
{
    // Register meta types for queued signal/slot connections
    qRegisterMetaType<OST::LogLevel>("OST::LogLevel");
    qRegisterMetaType<OST::EvType>("OST::EvType");

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
        sendmessage(message, pClient);
        //qDebug() << message;
    }
}

void WShandler::sendmessage(QString message, QWebSocket *client)
{
    if (client) client->sendTextMessage(message);
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
    //qDebug() << pSocket << "-" << pSocket->peerAddress().toString();
    m_clients << pSocket;
    if (mServerGrant == "0") mClientGrants[pSocket->peerAddress().toString()] = "1"; // full access for anyone
    if (mServerGrant == "1") mClientGrants[pSocket->peerAddress().toString()] =
            "0"; // read only for anyone - need grants to write
    if (mServerGrant == "2") mClientGrants[pSocket->peerAddress().toString()] = "-1"; // access grant required for anyone
    mClientLanguage[pSocket] = mLng;
}


void WShandler::processTextMessage(QString message)
{
    //qDebug() << "WShandler::processTextMessage" << message;
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    QString clientGrant = mClientGrants[pClient->peerAddress().toString()];
    // OST::Event event;

    QString _mess = message.replace("\\\"", "\"");
    _mess = _mess.replace("}\"", "}");
    _mess = _mess.replace("\"{", "{");
    QJsonDocument jsonResponse = QJsonDocument::fromJson(_mess.toUtf8()); // garder
    QJsonObject  obj = jsonResponse.object(); // garder
    sendMessage("OST server received json" + message);

    /* check message integrity */

    //message must contain only one root key
    if (obj.size() != 1)
    {
        logToClient(OST::LogLevel::Error, "Invalid message size : %1", {message}, "WS", pClient);
        return;
    }

    // message root key must be valid
    OST::ExtEvType eventType = OST::StrToExtEvent(obj.begin().key());
    if (eventType == OST::ExtEvType::ZZ)
    {
        logToClient(OST::LogLevel::Error, "Invalid message event: %1", {obj.begin().key()}, "WS", pClient);
        return;
    }

    // message data must be valid, ie keys/values content
    if (!obj[obj.begin().key()].isObject())
    {
        logToClient(OST::LogLevel::Error, "Invalid message data: %1", {message}, "WS", pClient);
        return;
    }

    /* format message */
    OST::ExtEvent event;
    event.ev = eventType;
    event.data = obj[obj.begin().key()].toObject();

    /* Check if client is granted to send update requests */
    if (mClientGrants[pClient->peerAddress().toString()] != "1" && event.ev != OST::ExtEvType::DU
            && event.ev != OST::ExtEvType::LO && event.ev != OST::ExtEvType::IL )
    {
        logToClient(OST::LogLevel::Error, "Client not granted for updates", {}, "WS", pClient);
        return;
    }

    /* client sends login request */
    if (event.ev == OST::ExtEvType::LO)
    {
        if (!event.data.contains("user") || !event.data.contains("pw") )
        {
            logToClient(OST::LogLevel::Error, "Invalid login data", {}, "WS", pClient);
            return;
        }

        logToClient(OST::LogLevel::Debug, "Login request: %1", {obj.begin().key()}, "WS", pClient);
        mClientGrants[pClient->peerAddress().toString()] = dbmanager->getGrants(event.data["user"].toString(),
                event.data["pw"].toString());

        if (event.data.contains("language")) setClientLanguage(pClient, event.data["language"].toString());

        emit clientEvent(event, pClient, mClientGrants[pClient->peerAddress().toString()]);
        return;
    }

    /* client changes its language */
    if (event.ev == OST::ExtEvType::IL)
    {
        if (!event.data.contains("language"))
        {
            logToClient(OST::LogLevel::Error, "Invalid language data", {}, "WS", pClient);
            return;
        }
        logToClient(OST::LogLevel::Debug, "Client set language: %1", {event.data["language"].toString()}, "WS", pClient);
        setClientLanguage(pClient, event.data["language"].toString());
        emit clientEvent(event, pClient, mClientGrants[pClient->peerAddress().toString()]);
        return;
    }

    if (event.ev == OST::ExtEvType::DU)
    {
        logToClient(OST::LogLevel::Debug, "Full dump request: %1", {obj.begin().key()}, "WS", pClient);
        if (event.data.contains("language"))
        {
            logToClient(OST::LogLevel::Debug, "Client set language: %1", {event.data["language"].toString()}, "WS", pClient);
            setClientLanguage(pClient, event.data["language"].toString());
        }

        emit clientEvent(event, pClient, mClientGrants[pClient->peerAddress().toString()]);
        return;
    }

    if (mClientGrants[pClient->peerAddress().toString()] != "1") return;

    emit externalEvent(event);

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
void WShandler::sendJsonMessage(QJsonObject json, QWebSocket* pClient)
{
    QJsonObject obj = translateJson(json, mClientLanguage.value(pClient, "en"));

    QJsonDocument jsondoc;
    jsondoc.setObject(obj);
    //QString strJson(jsondoc.toJson(QJsonDocument::Indented)); // version lisible
    QString strJson(jsondoc.toJson(QJsonDocument::Compact)); // version compactée
    sendmessage(strJson, pClient);
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
    //qDebug() << pClient << "-" << pClient->peerAddress().toString();

    if (pClient)
    {
        m_clients.removeAll(pClient);
        mClientLanguage.remove(pClient);
        pClient->deleteLater();
        mClientGrants.remove(pClient->peerAddress().toString());

    }
}
void WShandler::onModuleEvent(OST::EvType evt, QVariant data, OST::ElementBase *elt, OST::PropertyBase *prp,
                              OST::LovBase *lov, Basemodule *mod)
{
    //qDebug() << "WShandler::onModuleEvent" << static_cast<int>(evt) << data << mod->getModuleLabel();
    //    QJsonObject  obj;
    //    QJsonObject mods;
    //    QJsonObject  mod;
    //    mod["p"] = module->getPropertiesDump(event);
    //    //if (event.type == "moduledump")
    //    //{
    //    //    QJsonObject  infos;
    //    //    infos["label"] = module->getModuleLabel();
    //    //    mod["infos"] = QJsonObject::fromVariantMap(module->getAllMetadata());
    //    //    mod["globallovs"] = module->getGlobalLovsDump();
    //
    //    //}
    //    mods[event.module] = mod;
    //    //obj["m"] = mods;
    //
    //    obj[event.type] = mods;
    //    sendJsonMessage(obj);
    QJsonValue m = OST::ModuleJsonDumper(evt, data, elt, prp, lov, mod);
    QJsonObject o, s;
    if (evt != OST::EvType::dm)
    {
        o[mod->getModuleName()] = m;
    }
    else
    {
        o[data.toString()] = m;
    }
    s[EvToString(evt)] = o;
    sendJsonMessage(s);


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

void WShandler::setTranslateManager(OST::TranslateManager* translator)
{
    mTranslater = translator;
}

void WShandler::setClientLanguage(QWebSocket* client, const QString &language)
{
    if (client && !language.isEmpty())
    {
        mClientLanguage[client] = language;
        //qDebug() << "Client" << client->peerAddress().toString() << "language set to:" << language;
    }
}

QString WShandler::logLevelToEventType(OST::LogLevel level)
{
    switch (level)
    {
        case OST::LogLevel::Debug:
            return "md";
        case OST::LogLevel::Info:
            return "mi";
        case OST::LogLevel::Warning:
            return "mw";
        case OST::LogLevel::Error:
            return "me";
        case OST::LogLevel::Critical:
            return "mc";
        default:
            return "mc";
    }
}
void WShandler::logToClient(OST::LogLevel level, const QString &message,
                            const QVariantList &args, const QString &context, QWebSocket* client)
{
    if (!mTranslater)
    {
        qWarning() << "WShandler: TranslateManager not set, cannot translate messages";
        return;
    }

    QDateTime dt = QDateTime::currentDateTime();
    // Get client language (default: "en")
    QString clientLang = mClientLanguage.value(client, "en");

    // Translate message to client's language
    QString translatedMessage = mTranslater->translateWithArgs(message, args, clientLang);

    // Build JSON
    QJsonObject log;
    QJsonObject obj;

    log["d"] = dt.toString(Qt::ISODateWithMs);
    log["c"] = context;
    log["t"] = translatedMessage;
    log["l"] = static_cast<int>(level);
    obj["l"] = log;
    // Send to client
    client->sendTextMessage(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void WShandler::onLog(OST::LogLevel level, const QString &message,
                      const QVariantList &args, const QString &context)
{
    for (QWebSocket* client : m_clients)
    {
        logToClient(level, message, args, context, client);
    }
}
QJsonObject WShandler::translateJson(const QJsonObject pJsonObject, const QString &pLng)
{
    QJsonObject obj = pJsonObject;
    foreach(const QString &key, obj.keys())
    {
        QJsonValue value = obj[key];
        if ((key == "label" || key == "hint") && obj[key].isString())
        {
            obj[key] = mTranslater->translateWithArgs(obj[key].toString(), {}, pLng);
        }
        if (value.isObject())
        {
            obj[key] = translateJson(value.toObject(), pLng);
        }
    }
    return obj;
}
void WShandler::onControllerEvent(OST::EvType evt, QString key, QVariant data, OST::LovBase *lov)
{
    QJsonObject globlovs, result, o, s;
    if (evt == OST::EvType::uc)
    {
        if (strcmp(data.typeName(), "QStringList") == 0)
        {

            o[key] = data.toJsonArray();
        }
        else if (strcmp(data.typeName(), "QVariantMap") == 0)
        {

            o[key] = data.toJsonObject();
        }
        else
        {
            o[key] = data.toJsonValue();
        }
        s[EvToString(evt)] = o;
    }
    if (evt == OST::EvType::lc || evt == OST::EvType::lu)
    {
        OST::LovJsonDumper d;
        lov->accept(&d);
        globlovs[lov->getKey()] = d.getResult();
        o["l"] = globlovs;
        s[OST::EvToString(evt)] = o;
    }
    if (evt == OST::EvType::ld)
    {
        OST::LovJsonDumper d;
        lov->accept(&d);
        globlovs[key] = QJsonObject();
        o["l"] = globlovs;
        s[OST::EvToString(evt)] = o;
    }


    sendJsonMessage(s);
}
