#include "indiclient.h"
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
IndiCLient* IndiCLient::getInstance()
{
    if (instance == 0)
    {
        instance = new IndiCLient();
    }

    return instance;
}
/* Null, because instance will be initialized on demand. */
IndiCLient* IndiCLient::instance = 0;

IndiCLient::IndiCLient()
{
}
void IndiCLient::serverConnected(void)
{
    emit SigServerConnected();
    _nbprops =0;
}
void IndiCLient::serverDisconnected(int exit_code)
{
    emit SigServerDisconnected(exit_code);
    _nbprops =0;
}
void IndiCLient::newDevice(INDI::BaseDevice *dp)
{
    emit SigNewDevice(dp);
}
void IndiCLient::removeDevice(INDI::BaseDevice *dp)
{
    emit SigRemoveDevice(dp);
}
void IndiCLient::newProperty(INDI::Property *property)
{
    emit SigNewProperty(property);
    //_props.insert(QString(property->getDeviceName()) + QString(property->getName()));
    //qDebug() << "add prop - " << _props.size();
}
void IndiCLient::removeProperty(INDI::Property *property)
{
    emit SigRemoveProperty(property);
    //_props.remove(QString(property->getDeviceName()) + QString(property->getName()));
    //qDebug() << "del prop - " << _props.size();
}
void IndiCLient::newNumber(INumberVectorProperty *nvp)
{
    //if (strcmp(svp->name,"CONFIG_PROCESS")==0)
    //for (int i=0;i<nvp->nnp;i++) IDLog("Got number %s %s %s %f\n",nvp->device,nvp->name,nvp->np[i].name,nvp->np[i].value);
    emit SigNewNumber(nvp);
}
void IndiCLient::newText(ITextVectorProperty *tvp)
{
    emit SigNewText(tvp);
}
void IndiCLient::newSwitch(ISwitchVectorProperty *svp)
{
    //if (strcmp(svp->name,"CONFIG_PROCESS")==0)
    //for (int i=0;i<svp->nsp;i++) IDLog("Got switch %s %s %s\n",svp->device,svp->name,svp->sp[i].name);
    emit SigNewSwitch(svp);
}
void IndiCLient::newLight(ILightVectorProperty *lvp)
{
    emit SigNewLight(lvp);
}
void IndiCLient::newMessage(INDI::BaseDevice *dp, int messageID)
{
    emit SigNewMessage(dp,messageID);
}
void IndiCLient::newBLOB(IBLOB *bp)
{
    emit SigNewBLOB(bp);
}

