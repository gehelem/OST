#include "client.h"
#include <QApplication>
#include <basedevice.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>
#include <QtNetwork>
#include <baseclientqt.h>

/**************************************************************************************
**
***************************************************************************************/


/* singleton  : getting unique instance if exists, create it if not */
MyClient* MyClient::getInstance()
{
    if (instance == 0)
    {
        instance = new MyClient();
    }

    return instance;
}
/* Null, because instance will be initialized on demand. */
MyClient* MyClient::instance = 0;

MyClient::MyClient()
{
}

void MyClient::serverConnected(void)
{
    emit gotserverConnected();
}
void MyClient::serverDisconnected(int exit_code)
{
    emit gotserverDisconnected(exit_code);
}
void MyClient::newDevice(INDI::BaseDevice *dp)
{
    emit gotnewDevice(dp);
}
void MyClient::removeDevice(INDI::BaseDevice *dp)
{
    emit gotremoveDevice(dp);
}
void MyClient::newProperty(INDI::Property *property)
{
    qDebug() << "new prop" << property->getName();
    emit gotnewProperty(property);
}
void MyClient::removeProperty(INDI::Property *property)
{
    emit gotremoveProperty(property);
}
void MyClient::newNumber(INumberVectorProperty *nvp)
{
    //if (strcmp(svp->name,"CONFIG_PROCESS")==0)
    //for (int i=0;i<nvp->nnp;i++) IDLog("Got number %s %s %s %f\n",nvp->device,nvp->name,nvp->np[i].name,nvp->np[i].value);
    emit gotnewNumber(nvp);
}
void MyClient::newText(ITextVectorProperty *tvp)
{
    emit gotnewText(tvp);
}
void MyClient::newSwitch(ISwitchVectorProperty *svp)
{
    //if (strcmp(svp->name,"CONFIG_PROCESS")==0)
    //for (int i=0;i<svp->nsp;i++) IDLog("Got switch %s %s %s\n",svp->device,svp->name,svp->sp[i].name);
    emit gotnewSwitch(svp);
}
void MyClient::newLight(ILightVectorProperty *lvp)
{
    emit gotnewLight(lvp);
}
void MyClient::newMessage(INDI::BaseDevice *dp, int messageID)
{
    emit gotnewMessage(dp,messageID);
}
void MyClient::newBLOB(IBLOB *bp)
{
    emit gotnewBLOB(bp);
}

