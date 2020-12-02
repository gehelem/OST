#include "OSTClientGEN.h"
//#include "OSTClientSEQ.h"
#include "OSTClientFOC.h"
#include <baseclientqt.h>
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class OSTController : public QObject
{
    Q_OBJECT
public:
    OSTController(QObject *parent);
    ~OSTController();
    std::unique_ptr<OSTClientFOC> MyOSTClientFOC;
    void startf(void);
    void focusdone(void);
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
private Q_SLOTS:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
Q_SIGNALS:
    void closed();
private:
    //std::unique_ptr<OSTClientFOC> MyOSTClientFOC;
    //std::unique_ptr<OSTClientFOC> MyOSTClientFOC(new OSTClientFOC());
    //static std::unique_ptr<MyClient> camera_client(new MyClient());
};
