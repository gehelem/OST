#include "OSTController.h"
//#include <baseclientqt.h>




OSTController::OSTController(QObject *parent)
{
    //OSTClientGEN *MyOSTClientGEN = new OSTClientGEN(parent);

    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("Echo Server"),QWebSocketServer::NonSecureMode, this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, 9624)) {
       IDLog("Echoserver listening on port %i\n",9624);
       connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &OSTController::onNewConnection);
       connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &OSTController::closed);
    }


    MyOSTClientFOC.reset(new OSTClientFOC(parent));
    //MyOSTClientFOC.reset(new OSTClientFOC());
    MyOSTClientFOC->setClientname("focuser");
    MyOSTClientFOC->LogLevel=OSTLOG_ALL;
    MyOSTClientFOC->setOSTDevices("CCD Simulator","Focuser Simulator","","","");
    MyOSTClientFOC->setServer("localhost", 7624);
    MyOSTClientFOC->connectServer();
    MyOSTClientFOC->getDevices();
    //server = new EchoServer(1234, true);
    //connect(server        , &EchoServer::startf     , this, &OSTController::startf   );
    QObject::connect(MyOSTClientFOC.get(), &OSTClientFOC::focusdone, this, &OSTController::focusdone);
    //connect(img.get(),&OSTImage::success,this,&OSTClientFOC::sssuccess);
}

OSTController::~OSTController()
{
    IDLog("OSTController delete\n");
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
    //delete MyOSTClientFOC;
    //delete MyOSTClientGEN;
}
void OSTController::focusdone(void)
{

    IDLog("OSTController saying focus is done\n");
    QWebSocket *pClient = m_clients[0];
    if (pClient) {
        pClient->sendTextMessage("focus done");
    }

}


void OSTController::startf(void)
{

    IDLog("OSTController asking to start focus\n");
    MyOSTClientFOC->startFocusing(35000,100,100,30);

}

void OSTController::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    IDLog("Echoserver new connection\n");

    connect(pSocket, &QWebSocket::textMessageReceived, this, &OSTController::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &OSTController::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &OSTController::socketDisconnected);

    m_clients << pSocket;
}


void OSTController::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("Echoserver received message%s\n",message.toUtf8().data());

    if (pClient) {
        pClient->sendTextMessage(message);
    }
    if (message=="startfocus"){
        startf();
    }
}

void OSTController::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("Echoserver received binary message\n");

    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void OSTController::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("Echoserver disconnect\n");
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
