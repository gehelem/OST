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
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

class OSTClientFOC : public OSTClient
{
    Q_OBJECT
  public:

    OSTClientFOC(QObject *parent = Q_NULLPTR);
    virtual ~OSTClientFOC();
    INDI::BaseDevice *cameradevice;
    INDI::BaseDevice *focuserdevice;

    std::unique_ptr<OSTImage> img =nullptr;
    bool startFocusing(void);
    //void setOSTDevices(std::string wcamera,std::string wfocuser,std::string wmount,std::string wwheel,std::string wguider);
  public slots:
    void sssuccess(void);
    virtual void processTextMessage(QString message) override;
  protected:
    virtual void newDevice(INDI::BaseDevice *dp) override;
    virtual void removeDevice(INDI::BaseDevice */*dp*/) override {}
    virtual void newProperty(INDI::Property *property) override;
    virtual void removeProperty(INDI::Property */*property*/) override {}
    virtual void newBLOB(IBLOB */*bp*/) override;
    virtual void newSwitch(ISwitchVectorProperty *svp);
    virtual void newNumber(INumberVectorProperty *nvp) override ;
    virtual void newMessage(INDI::BaseDevice *dp, int messageID) override;
    virtual void newText(ITextVectorProperty *tvp);
    virtual void newLight(ILightVectorProperty */*lvp*/) override {}
  private:
    // simple focus parameters
    double Fpos,Fbl,Fincr,Fnb,Factpos;
    double Fs; // simple focus iterator
    double FBestpos;
    float FBestHFR;
  signals:
    void focusdone(void);
};

#endif
