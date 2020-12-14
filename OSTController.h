#include "OSTClientGEN.h"
//#include "OSTClientSEQ.h"
#include "OSTClientFOC.h"
#include "OSTClientPAN.h"
#include "OSTParser.h"
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
    std::unique_ptr<OSTClientPAN> MyOSTClientPAN;
    void startf(void);
    void focusdone(void);
    void sendmessage(QString message);
    void sendjson(QJsonDocument json);
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    QJsonObject properties;
    QString PropertiesFolder="/home/gilles/OST";

private Q_SLOTS:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
    void emitnewprop(INDI::Property *property);
    void emitnewtext(ITextVectorProperty *prop);
    void emitnewnumber(INumberVectorProperty *prop);
    void emitnewswitch(ISwitchVectorProperty *prop);
    void emitnewlight(ILightVectorProperty *prop);
    void emitnewdevice(INDI::BaseDevice *d);
    void emitall(void);



Q_SIGNALS:
    void closed();
private:
    //std::unique_ptr<OSTClientFOC> MyOSTClientFOC;
    //std::unique_ptr<OSTClientFOC> MyOSTClientFOC(new OSTClientFOC());
    //static std::unique_ptr<MyClient> camera_client(new MyClient());
};
