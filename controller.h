#ifndef CONTROLLER_h_
#define CONTROLLER_h_
#pragma once
#include "mFocuser.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"




QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

/*!
 * This class is the heart of OST
 * It dispatches events/orders/datas  from one layer to each other
 * - indiclient
 * - websocket traffic
 * - functional Modules
 */
class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(QObject *parent);
    ~Controller();
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    MyClient *indiclient;
public slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void sendmessage(QString message);
    void sendbinary(QByteArray *data);
    void socketDisconnected();
signals:
    void closed();
private:
};
#endif
