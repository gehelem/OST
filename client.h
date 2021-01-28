#ifndef CLIENT_h_
#define CLIENT_h_
#include <baseclientqt.h>
/*!
 * indiclient class, overloaded with some basic functions to help us
 * it should be instanciated only once, to keep only one active connection to indiserver
*/
class MyClient : public INDI::BaseClientQt
{
    Q_OBJECT
  public:
    MyClient(QObject *parent = Q_NULLPTR);
    ~MyClient() = default;
    bool connectIndi(void);
    void connectAllDevices(void);
    void disconnectAllDevices(void);
    void loadDevicesConfs(void);
  signals:
    void gotserverConnected();
    void gotserverDisconnected(int exit_code);
    void gotnewDevice(INDI::BaseDevice *dp);
    void gotremoveDevice(INDI::BaseDevice *dp);
    void gotnewProperty(INDI::Property *property);
    void gotremoveProperty(INDI::Property *property);
    void gotnewText(ITextVectorProperty *tvp);
    void gotnewSwitch(ISwitchVectorProperty *svp);
    void gotnewLight(ILightVectorProperty *lvp);
    void gotnewBLOB(IBLOB *bp);
    void gotnewNumber(INumberVectorProperty *nvp);
    void gotnewMessage(INDI::BaseDevice *dp, int messageID);

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
