#ifndef CONTROLLER_h_
#define CONTROLLER_h_
#pragma once
#include "client.h"
#include "job.h"
#include <baseclientqt.h>
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>



QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(QObject *parent);
    ~Controller();
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    QJsonObject properties;
    QString PropertiesFolder="~/OST";
    void processShoot(void);
    void connectIndi(void);
    void connectAllDevices(void);
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
