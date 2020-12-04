#ifndef OSTClientGEN_h_
#define OSTClientGEN_h_
#pragma once

//QT Includes
//QT Includes
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>
#include <QtNetwork>
#include <baseclientqt.h>
#include <basedevice.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <QJsonObject>
#include <QJsonDocument>

enum OSTLogLevel
{
    OSTLOG_ALL,
    OSTLOG_NONE,
    OSTLOG_INFO
};

class OSTClientGEN : public INDI::BaseClientQt
{
  public:
    OSTClientGEN(QObject *parent = Q_NULLPTR);
    virtual ~OSTClientGEN();

    void setClientname(std::string n);
    void askNewJob(std::string job);
    void connectallOSTDevices(void);
    void disconnectallOSTDevices(void);
    void setOSTDevices(std::string wcamera,std::string wfocuser,std::string wmount,std::string wwheel,std::string wguider);
    void switchstate(std::string newstate);

    INDI::BaseDevice *camera;
    INDI::BaseDevice *focuser;
    INDI::BaseDevice *mount;
    INDI::BaseDevice *wheel;
    INDI::BaseDevice *guider;
    std::string camera_name;
    std::string focuser_name;
    std::string mount_name;
    std::string wheel_name;
    std::string guider_name;
    std::string client_name="generic";
    std::string state="idle";
    std::string job="";
    OSTLogLevel LogLevel;
    QList<INDI::Property> OSTproperties;


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

signals:
    void s_newstate(void);

};

#endif
