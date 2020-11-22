#ifndef OSTClientFOC_h_
#define OSTClientFOC_h_

#pragma once

#include "OSTClientGEN.h"
#include "OSTImage.h"
//QT Includes
//QT Includes
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>

class OSTClientFOC : public OSTClientGEN
{
    Q_OBJECT
  public:
    OSTClientFOC();
    std::unique_ptr<OSTImage> img =nullptr;
    bool startFocusing(int start, int backlash,int incr, int nb);
    //void setOSTDevices(std::string wcamera,std::string wfocuser,std::string wmount,std::string wwheel,std::string wguider);
  public slots:
    void sssuccess(void);
  protected:
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
    // simple focus parameters
    int Fpos,Fbl,Fincr,Fnb;
    int Fs; // simple focus iterator

};

#endif
