#ifndef WSHANDLER_h_
#define WSHANDLER_h_
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"

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
    void OnNewMessageSent(QString message,QString *pModulename, QString Device);
    void OnModuleDumped2(QVariant props, QString* pModulename, QString* pModulelabel, QString* pModuledescription);
    void OnPropertyChanged(QString *moduleName, QString *propName, QVariant *propValue, QVariant *prop);
    void OnModuleEvent(QString *pModulename, const QString &eventType, QVariant *pEventData, QString *pFree);


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
    //void setPropertyText(TextProperty *pProperty);
private:
};
#endif
