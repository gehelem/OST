#ifndef CLIENT_h_
#define CLIENT_h_
#pragma once
#include <baseclientqt.h>

class MyClient : public INDI::BaseClientQt
{
    Q_OBJECT
  public:
    MyClient(QObject *parent = Q_NULLPTR);
    ~MyClient() = default;
  signals:
    virtual void gotserverConnected();
    virtual void gotserverDisconnected(int exit_code);
    virtual void gotnewDevice(INDI::BaseDevice *dp);
    virtual void gotremoveDevice(INDI::BaseDevice *dp);
    virtual void gotnewProperty(INDI::Property *property);
    virtual void gotremoveProperty(INDI::Property *property);
    virtual void gotnewText(ITextVectorProperty *tvp);
    virtual void gotnewSwitch(ISwitchVectorProperty *svp);
    virtual void gotnewLight(ILightVectorProperty *lvp);
    virtual void gotnewBLOB(IBLOB *bp);
    virtual void gotnewNumber(INumberVectorProperty *nvp);
    virtual void gotnewMessage(INDI::BaseDevice *dp, int messageID);

  protected:
    virtual void serverConnected();
    virtual void serverDisconnected(int exit_code);
    virtual void newDevice(INDI::BaseDevice *dp);
    virtual void removeDevice(INDI::BaseDevice *dp);
    virtual void newProperty(INDI::Property *property);
    virtual void removeProperty(INDI::Property *property);
    virtual void newText(ITextVectorProperty *tvp);
    virtual void newSwitch(ISwitchVectorProperty *svp);
    virtual void newLight(ILightVectorProperty *lvp);
    virtual void newBLOB(IBLOB *bp);
    virtual void newNumber(INumberVectorProperty *nvp);
    virtual void newMessage(INDI::BaseDevice *dp, int messageID);


}
;
#endif
