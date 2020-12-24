#include "OSTClient.h"
#include "OSTClientGEN.h"
#include "OSTClientCTL.h"
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
    std::unique_ptr<OSTClientGEN> MyOSTClientGEN;
    std::unique_ptr<OSTClientPAN> MyOSTClientPAN;
    std::unique_ptr<OSTClientCTL> MyOSTClientCTL;
    std::unique_ptr<OSTClientFOC> MyOSTClientFOC;
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    QJsonObject properties;
    QString PropertiesFolder="~/OST";

public slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void sendmessage(QString message);
    void sendjson(QJsonObject json);
    void socketDisconnected();
    void emitnewprop(INDI::Property *property);
    void emitnewtext(ITextVectorProperty *prop);
    void emitnewnumber(INumberVectorProperty *prop);
    void emitnewswitch(ISwitchVectorProperty *prop);
    void emitnewlight(ILightVectorProperty *prop);
    void emitnewdevice(INDI::BaseDevice *d);
    //void emitall(void);
//    QJsonObject updateproperty(QString module,QString category,QString group,QString property);



signals:
    void closed();
private:
    //std::unique_ptr<OSTClientFOC> MyOSTClientFOC;
    //std::unique_ptr<OSTClientFOC> MyOSTClientFOC(new OSTClientFOC());
    //static std::unique_ptr<MyClient> camera_client(new MyClient());
};
