#include <QApplication>
#include <stdio.h>
#include "client.h"
#include "job.h"
#include "controller.h"
#include "jFocuser.h"


Controller::Controller(QObject *parent)
{

    this->setParent(parent);
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("OST server"),QWebSocketServer::NonSecureMode, this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, 9624)) {
       connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &Controller::onNewConnection);
       connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &Controller::closed);
    }
    IDLog("OST server listening\n");

    indiclient = new MyClient(this);
    connectIndi();
}

Controller::~Controller()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}


void Controller::sendmessage(QString message)
{
    for (int i=0;i<m_clients.size();i++) {
        QWebSocket *pClient = m_clients[i];
        if (pClient) pClient->sendTextMessage(message);
    }

}
void Controller::sendbinary(QByteArray *data)
{
    for (int i=0;i<m_clients.size();i++) {
        QWebSocket *pClient = m_clients[i];
        if (pClient) pClient->sendBinaryMessage(*data);
    }
}


void Controller::onNewConnection()
{
    IDLog("New client connection\n");
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    connect(pSocket, &QWebSocket::textMessageReceived, this, &Controller::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &Controller::socketDisconnected);
    m_clients << pSocket;
}


void Controller::processTextMessage(QString message)
{
    IDLog("OST server received text message %s\n",message.toStdString().c_str() );
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8()); // garder
    QJsonObject  obj = jsonResponse.object(); // garder

    if ( (obj["message"].toString()=="shoot")
      || (message=="shoot") )
    {
        if (indiclient->isServerConnected())
        {
            processShoot();
        } else {
            IDLog("Indi server disconnected - trying to connect\n");
            connectIndi();
        }
    }

}

void Controller::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("OST server received binary message\n");

    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void Controller::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("OST client disconnected\n");
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
void Controller::connectIndi()
{
    indiclient->setServer("localhost", 7624);
    indiclient->connectServer();
    if (indiclient->isServerConnected())
    {
        IDLog("Indi server connected\n");
        connectAllDevices();
    } else {
        IDLog("Could not connect to indi server\n");
    }
}

void Controller::connectAllDevices()
{
    std::vector<INDI::BaseDevice *> devs = indiclient->getDevices();
    IDLog("trying to connect %i devices\n",devs.size());
    for(std::size_t i = 0; i < devs.size(); i++) {
        IDLog("trying to connect %s \n",devs[i]->getDeviceName());
        if (!devs[i]->isConnected()) {
            indiclient->connectDevice(devs[i]->getDeviceName());

        }
        if (!devs[i]->isConnected())
            IDLog("Can't connect %s\n",devs[i]->getDeviceName());

        if (devs[i]->getDriverInterface() & INDI::BaseDevice::CCD_INTERFACE)
        {
            IDLog("Setting blob mode %s \n",devs[i]->getDeviceName());
            indiclient->setBLOBMode(B_ALSO,devs[i]->getDeviceName(),nullptr);
        }
    }
}


void Controller::processShoot()
{
    IDLog("process shoot request\n");
    connectAllDevices();
    JFocuser *JobShoot = new JFocuser(indiclient);
    JobShoot->startFraming();
}

