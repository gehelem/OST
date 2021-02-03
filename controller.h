#ifndef CONTROLLER_h_
#define CONTROLLER_h_
#include "mFocuser.h"
#include "mMainctl.h"
#include "mSequence.h"
#include "mGuider.h"
#include "mNavigator.h"
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
    MFocuser    *focuser;
    MGuider     *guider;
    MSequence   *sequence;
    MNavigator  *navigator;
    MMainctl    *mainctl;
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
