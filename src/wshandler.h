#ifndef WSHANDLER_h_
#define WSHANDLER_h_
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QSslCertificate>
#include <QSslKey>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)
/*!
 * Websocket management
 * open/close/send/receives sockets
 * it should translate JSON messages
 */
class WShandler : public QObject
{
        Q_OBJECT
    public:
        WShandler(QObject *parent, const QString &ssl = "N", const QString &sslCert = "", const QString &sslKey = "");
        ~WShandler();
    public :
        void processModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                                const QVariantMap &eventData);
        void processFileEvent(const QString &eventType, const QStringList &eventData);

    signals:
        void closed();
        void textRcv(QString txt);
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
        void sendMessage(const QString &pMessage);


};
#endif
