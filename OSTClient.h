#ifndef OSTClient_h_
#define OSTClient_h_
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


class OSTClient : public INDI::BaseClientQt
{
    Q_OBJECT
  public:
    OSTClient(QObject *parent = Q_NULLPTR);
    virtual ~OSTClient();
    OSTLogLevel LogLevel=OSTLOG_ALL;
    void switchstate(std::string newstate);
    std::string state="idle";
    QJsonObject thismodule;
    QJsonArray properties;
    QJsonArray groups;
    QJsonArray categories;


    virtual QJsonObject getmodule  (QString module) ;
    virtual QJsonObject getcategory(QString module,QString category) ;
    virtual QJsonObject getgroup   (QString module,QString category,QString group) ;
    virtual QJsonObject getproperty(QString module,QString category,QString group,QString property) ;
    void updateproperty(QString module, QString category, QString group, QString property);
    void updategroup   (QString module, QString category, QString group);
    void updatecategory(QString module, QString category);
    void updatemodule  (QString module);
    void appendmessage(QString module, QString messagename, QString message);
    void messagelog(QString message);

  public slots:
    virtual void processTextMessage(QString message);

  signals:
    void sendjson(QJsonObject message);

  protected:
    virtual void serverConnected();
    virtual void serverDisconnected(int exit_code);
    virtual void newDevice(INDI::BaseDevice *dp);
    virtual void removeDevice(INDI::BaseDevice *dp);
    virtual void newProperty(INDI::Property *property);
    virtual void removeProperty(INDI::Property *property);
    virtual void newNumber(INumberVectorProperty *nvp);
    virtual void newText(ITextVectorProperty *tvp);
    virtual void newSwitch(ISwitchVectorProperty *svp);
    virtual void newLight(ILightVectorProperty *lvp);
    virtual void newBLOB(IBLOB *bp);
    virtual void newMessage(INDI::BaseDevice *dp, int messageID);

private:


};

#endif
