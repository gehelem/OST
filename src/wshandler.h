#ifndef WSHANDLER_h_
#define WSHANDLER_h_
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <utils/jsondumper.h>
#include "model/property.h"

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
    WShandler(QObject *parent);
    ~WShandler();
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
public slots:
    void OnPropertyCreated(Property *pProperty, QString *pModulename);
    void OnPropertyUpdated(Property *pProperty, QString *pModulename);
    void OnPropertyRemoved(Property *pProperty, QString *pModulename);
    void OnNewMessageSent(QString message,QString *pModulename, QString Device);
    void OnModuleDumped(QMap<QString, QMap<QString, QMap<QString, Property*>>> treeList, QString* pModulename, QString* pModulelabel);


    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void sendmessage(QString message);
    void sendJsonMessage(QJsonObject json);
    void sendbinary(QByteArray *data);
    void socketDisconnected();
signals:
    void closed();
    void textRcv(QString txt);
    void dumpAsked(void);
private:
};
#endif
