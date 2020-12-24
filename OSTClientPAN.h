#ifndef OSTClientPAN_h_
#define OSTClientPAN_h_
#pragma once

#include "OSTClient.h"
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

class OSTClientPAN : public OSTClient
{
    Q_OBJECT
  public:

    OSTClientPAN(QObject *parent = Q_NULLPTR);
    virtual ~OSTClientPAN();
    virtual QJsonObject getmodule  (QString module) override;
    virtual QJsonObject getcategory(QString module,QString category) override;
    virtual QJsonObject getgroup   (QString module,QString category,QString group) override;
    virtual QJsonObject getproperty(QString module,QString category,QString group,QString property) override;


public slots:
//    void sssuccess(void);

  protected:
    virtual void serverConnected() override;
    virtual void serverDisconnected(int exit_code) override;
    virtual void newDevice(INDI::BaseDevice *dp) override;
    virtual void removeDevice(INDI::BaseDevice *dp) override;
    virtual void newProperty(INDI::Property *property) override;
    virtual void removeProperty(INDI::Property *property) override;
    virtual void newNumber(INumberVectorProperty *nvp) override;
    virtual void newText(ITextVectorProperty *tvp) override;
    virtual void newSwitch(ISwitchVectorProperty *svp) override;
    virtual void newLight(ILightVectorProperty *lvp) override;
    virtual void newBLOB(IBLOB *bp) override;
    virtual void newMessage(INDI::BaseDevice *dp, int messageID) override;

private:



};

#endif
