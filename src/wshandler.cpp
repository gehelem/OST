#include <QCoreApplication>
#include <QtCore>
#include <QFile>
#include <QSslConfiguration>
#include <QJsonDocument>
#include "wshandler.h"
#include "translatemanager.h"

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
        //qDebug() << message;
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
    //qDebug() << pSocket << "-" << pSocket->peerAddress().toString();
    m_clients << pSocket;
    if (mServerGrant == "0") mClientGrants[pSocket->peerAddress().toString()] = "1"; // full access for anyone
    if (mServerGrant == "1") mClientGrants[pSocket->peerAddress().toString()] =
            "0"; // read only for anyone - need grants to write
    if (mServerGrant == "2") mClientGrants[pSocket->peerAddress().toString()] = "-1"; // access grant required for anyone
}


void WShandler::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    QString clientGrant = mClientGrants[pClient->peerAddress().toString()];
    OST::Event event;

    if (clientGrant == "-1")
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
            if ((g == "0") || (g == "1"))
            {
                mClientGrants[pClient->peerAddress().toString()] = g;
                event.type = "Freadall";
                emit externalEvent(event);
            }
        }
        return;
    }

    if (obj["evt"].toString() == "Freadall")
    {
        event.type = "Freadall";
        emit externalEvent(event);
    }

    // Handle language setting from client
    if (obj["evt"].toString() == "setlanguage")
    {
        QString language = obj["language"].toString();
        if (!language.isEmpty())
        {
            setClientLanguage(pClient, language);
            sendMessage("Client language set to: " + language);
        }
        return;
    }

    /* ignore update messages from readonly users */
    if (clientGrant == "0") return;

    event.type = obj["evt"].toString();
    event.module = obj["mod"].toString();
    event.property = obj["key"].toString();
    event.data = obj["dta"].toVariant().toMap();

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
        mClientLanguages.remove(pClient);
        pClient->deleteLater();
        mClientGrants.remove(pClient->peerAddress().toString());

    }
}
void WShandler::onModuleEvent(Basemodule *module, OST::Event event)
{
    QJsonObject  obj;
    QJsonObject mods;
    QJsonObject  mod;
    mod["p"] = module->getPropertiesDump(event);
    //if (event.type == "moduledump")
    //{
    //    QJsonObject  infos;
    //    infos["label"] = module->getModuleLabel();
    //    mod["infos"] = QJsonObject::fromVariantMap(module->getAllMetadata());
    //    mod["globallovs"] = module->getGlobalLovsDump();

    //}
    mods[event.module] = mod;
    //obj["m"] = mods;

    obj[event.type] = mods;
    sendJsonMessage(obj);


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
        mClientLanguages[client] = language;
        qDebug() << "Client" << client->peerAddress().toString() << "language set to:" << language;
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

void WShandler::onLog(OST::LogLevel level, const QString &message,
                      const QVariantList &args, const QString &context)
{
    if (!mTranslater)
    {
        qWarning() << "WShandler: TranslateManager not set, cannot translate messages";
        return;
    }

    QDateTime dt = QDateTime::currentDateTime();

    // Broadcast to each client in their language
    for (QWebSocket* client : m_clients)
    {
        // Get client language (default: "en")
        QString clientLang = mClientLanguages.value(client, "en");

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
}
