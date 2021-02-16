#ifndef CONTROLLER_h_
#define CONTROLLER_h_
#include "focus.h"
#include "wshandler.h"
#include "properties.h"


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
    MyClient    *indiclient;
    Properties  *properties;
    FocusModule    *focuser;
    WShandler   *wshandler;
public slots:
    void valueChanged(Prop prop);
    void AppendGraph (Prop prop,OGraph gra,OGraphValue val);
    void propCreated (Prop prop);
    void propDeleted (Prop prop);

signals:
    void closed();
private:
};
#endif
