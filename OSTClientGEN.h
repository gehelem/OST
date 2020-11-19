#ifndef OSTClientGEN_h_
#define OSTClientGEN_h_
#pragma once

#include "baseclient.h"

class OSTClientGEN : public  INDI::BaseClient
{
  public:
    OSTClientGEN();
    ~OSTClientGEN() = default;

    void setClientname(std::string n);
    void connectallOSTDevices(void);
    void disconnectallOSTDevices(void);
    void setOSTDevices(std::string wcamera,std::string wfocuser,std::string wmount,std::string wwheel,std::string wguider);
    INDI::BaseDevice *camera;
    INDI::BaseDevice *focuser;
    INDI::BaseDevice *mount;
    INDI::BaseDevice *wheel;
    INDI::BaseDevice *guider;
    //std::vector<INDI::Property *> pAll;

  protected:
    virtual void newDevice(INDI::BaseDevice *dp);
    virtual void removeDevice(INDI::BaseDevice */*dp*/) {}
    virtual void newProperty(INDI::Property *property);
    virtual void removeProperty(INDI::Property *property);
    virtual void newBLOB(IBLOB *bp);
    virtual void newSwitch(ISwitchVectorProperty */*svp*/) {}
    virtual void newNumber(INumberVectorProperty *nvp);
    virtual void newMessage(INDI::BaseDevice *dp, int messageID);
    virtual void newText(ITextVectorProperty *tvp);
    virtual void newLight(ILightVectorProperty */*lvp*/) {}
    virtual void serverConnected();
    virtual void serverDisconnected(int exit_code);

private:
    std::string client_name="defaultname";

};

#endif
