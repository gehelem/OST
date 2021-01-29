#ifndef WSHANDLER_h_
#define WSHANDLER_h_
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)
#include "properties.h"
/*!
 * Websocket management
 * open/close/send/receives sockets
 * it should translate JSON messages
 */
class WShandler : public QObject
{
    Q_OBJECT
public:
    WShandler(QObject *parent,Properties *properties);
    ~WShandler();
    Properties *props;
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
public slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void sendmessage(QString message);
    void sendJsonMessage(QJsonObject json);
    void sendbinary(QByteArray *data);
    void socketDisconnected();
    void sendProperty(Prop prop);
    void sendElement(Prop prop);
    void sendAll(void);
signals:
    void closed();
    void changeValue(Prop prop);
private:
};
#endif
