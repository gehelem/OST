#ifndef OSTClientPAN_h_
#define OSTClientPAN_h_

#pragma once
#include "OSTClientGEN.h"
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
#include <OSTCommon.h>

class OSTClientPAN : public OSTClientGEN
{
    Q_OBJECT
  public:

    OSTClientPAN(QObject *parent = Q_NULLPTR);
    virtual ~OSTClientPAN();
    void givemeall(void);

  public slots:
//    void sssuccess(void);
  protected:
    virtual void newDevice(INDI::BaseDevice *dp);
    virtual void removeDevice(INDI::BaseDevice */*dp*/) override {}
    virtual void newProperty(INDI::Property *property) override;
    virtual void removeProperty(INDI::Property */*property*/) override {}
    virtual void newBLOB(IBLOB */*bp*/) override;
    virtual void newSwitch(ISwitchVectorProperty *svp) override;
    virtual void newNumber(INumberVectorProperty *nvp);
    virtual void newMessage(INDI::BaseDevice *dp, int messageID) override;
    virtual void newText(ITextVectorProperty *tvp) override;
    virtual void newLight(ILightVectorProperty *lvp) override;
  private:

  signals:
    void emitnewdevice(INDI::BaseDevice *dp);
    void emitnewprop(INDI::Property *property);
    void emitnewtext(ITextVectorProperty *prop);
    void emitnewlight(ILightVectorProperty *prop);
    void emitnewnumber(INumberVectorProperty *prop);
    void emitnewswitch(ISwitchVectorProperty *prop);

};

#endif
