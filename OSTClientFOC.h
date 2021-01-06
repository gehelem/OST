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
    bool startFinefocus(void);
    bool startFraming(void);
    virtual void executespecificcurrenttask(INumberVectorProperty *nvp,ITextVectorProperty *tvp,ISwitchVectorProperty *svp,ILightVectorProperty *lvp,IBLOB *bp) override;
  public slots:
    void sssuccess(void);
    virtual void processTextMessage(QString message) override;
  protected:
    virtual void newDevice(INDI::BaseDevice *dp) override;
    virtual void removeDevice(INDI::BaseDevice */*dp*/) override {}
    virtual void newProperty(INDI::Property *property) override;
    virtual void removeProperty(INDI::Property */*property*/) override {}
  private:
    // simple focus parameters
    double Fexp;
    double Fpos,Fbl,Fincr,Fnb,Factpos;
    double Fs; // simple focus iterator
    double FBestpos;
    float FBestHFR;
  signals:
    void focusdone(void);
};

#endif
