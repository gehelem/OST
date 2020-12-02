#pragma once

#include "OSTClientGEN.h"
#include <baseclientqt.h>



class OSTClientSEQ : public OSTClientGEN
{
    Q_OBJECT
  public:
    OSTClientSEQ(QObject *parent = Q_NULLPTR);
    virtual ~OSTClientSEQ();

  protected:
    virtual void newDevice(INDI::BaseDevice *dp) override;
    virtual void removeDevice(INDI::BaseDevice */*dp*/) override {}
    virtual void newProperty(INDI::Property *property) override;
    virtual void removeProperty(INDI::Property */*property*/) override {}
    virtual void newBLOB(IBLOB */*bp*/) override;
    virtual void newSwitch(ISwitchVectorProperty */*svp*/) override {}
    //virtual void newNumber(INumberVectorProperty *nvp) override ;
    virtual void newMessage(INDI::BaseDevice *dp, int messageID) override;
    virtual void newText(ITextVectorProperty */*tvp*/) override {}
    virtual void newLight(ILightVectorProperty */*lvp*/) override {}

//  private:

};
