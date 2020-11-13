#pragma once

#include "OSTClientGEN.h"
#include "OSTImage.h"


class OSTClientFOC : public QObject, public OSTClientGEN
{
    Q_OBJECT
  public:
    //OSTClientSEQ() ;
    //~OSTClientSEQ()  = default;
    void startFocusing(void);
    void setOSTDevices(std::string wcamera,std::string wfocuser,std::string wmount,std::string wwheel,std::string wguider);


  protected:
    virtual void newDevice(INDI::BaseDevice *dp) override;
    virtual void removeDevice(INDI::BaseDevice */*dp*/) override {}
    virtual void newProperty(INDI::Property *property) override;
    virtual void removeProperty(INDI::Property */*property*/) override {}
    virtual void newBLOB(IBLOB */*bp*/) override;
    virtual void newSwitch(ISwitchVectorProperty */*svp*/) override {}
    virtual void newNumber(INumberVectorProperty *nvp) override ;
    virtual void newMessage(INDI::BaseDevice *dp, int messageID) override;
    virtual void newText(ITextVectorProperty */*tvp*/) override {}
    virtual void newLight(ILightVectorProperty */*lvp*/) override {}
  private:
    OSTImage img;

};
