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
                  const QString &grant = "0");
        ~WShandler();
    public :
        void processModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                                const QVariantMap &eventData);
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
        void onModuleEvent(Basemodule* module, OST::Event event);

    signals:
        void closed();
        void externalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                           const QVariantMap &eventData);
    private:
        void onNewConnection();
        void processTextMessage(QString message);
        void processBinaryMessage(QByteArray message);
        void sendmessage(QString message);
        void sendJsonMessage(QJsonObject json);
        void sendbinary(QByteArray *data);
        void socketDisconnected();
        QWebSocketServer *m_pWebSocketServer;
        QList<QWebSocket *> m_clients;
        QMap<QString, QString> mClientGrants;
        QString mServerGrant = "0";
        void sendMessage(const QString &pMessage);

        OST::TranslateManager* mTranslater = nullptr;
        QMap<QWebSocket*, QString> mClientLanguages;  // Client → language

        QString logLevelToEventType(OST::LogLevel level);


};
#endif
