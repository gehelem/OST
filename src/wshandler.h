#ifndef WSHANDLER_h_
#define WSHANDLER_h_
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QSslCertificate>
#include <QSslKey>
#include <QVariantList>
#include "basemodule.h"
#include "dbmanager.h"
#include "logger.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

namespace OST
{
class TranslateManager;
}
/*!
 * Websocket management
 * open/close/send/receives sockets
 * it should translate JSON messages
 */
class WShandler : public QObject
{
        Q_OBJECT
    public:
        WShandler(QObject *parent, const QString &ssl = "N", const QString &sslCert = "", const QString &sslKey = "",
                  const QString &grant = "0", const QString &lng = "en");
        ~WShandler();
    public :
        void processFileEvent(const QString &eventType, const QStringList &eventData);
        DBManager   *dbmanager;

        /**
         * @brief Configure TranslateManager to translate per client
         * @param translator Pointer to TranslateManager
         */
        void setTranslateManager(OST::TranslateManager* translator);

        /**
         * @brief Sets the language for a WebSocket client
         * @param client Pointer to the client
         * @param language Language code (fr, en, etc.)
         */
        void setClientLanguage(QWebSocket* client, const QString &language);

    public slots:
        /**
         * @brief Slot that listens to the universal logSignal() signal
         * @param level Log level
         * @param message Translation key
         * @param args Arguments for parametric translation
         * @param context Message context/source
         */
        void onLog(OST::LogLevel level, const QString &message,
                   const QVariantList &args, const QString &context);
        void onModuleEvent(OST::EvType evt, QVariant data, OST::ElementBase* elt, OST::PropertyBase* prp, OST::LovBase* lov,
                           Basemodule* mod);
        void sendJsonMessage(QJsonObject json);
        void sendJsonMessage(QJsonObject json, QWebSocket* client);

    signals:
        void closed();
        void externalEvent(QVariantMap);
        void clientEvent(QVariantMap, QWebSocket*, QString);

    private:
        void onNewConnection();
        void processTextMessage(QString message);
        void processBinaryMessage(QByteArray message);
        void sendmessage(QString message);
        void sendmessage(QString message, QWebSocket* client);
        void sendbinary(QByteArray *data);
        void socketDisconnected();
        QWebSocketServer *m_pWebSocketServer;
        QList<QWebSocket *> m_clients;
        QMap<QString, QString> mClientGrants;
        QMap<QWebSocket*, QString> mClientLanguage;  // Client → language
        QString mServerGrant = "0";
        QString mLng = "en";
        void sendMessage(const QString &pMessage);
        QJsonObject translateJson(const QJsonObject pJsonObject, const QString &pLng);

        OST::TranslateManager* mTranslater = nullptr;

        QString logLevelToEventType(OST::LogLevel level);
};
#endif
