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
    emit gotserverConnected();
}
void IndiCLient::serverDisconnected(int exit_code)
{
    emit gotserverDisconnected(exit_code);
}
void IndiCLient::newDevice(INDI::BaseDevice *dp)
{
    emit gotnewDevice(dp);
}
void IndiCLient::removeDevice(INDI::BaseDevice *dp)
{
    emit gotremoveDevice(dp);
}
void IndiCLient::newProperty(INDI::Property *property)
{
    emit gotnewProperty(property);
}
void IndiCLient::removeProperty(INDI::Property *property)
{
    emit gotremoveProperty(property);
}
void IndiCLient::newNumber(INumberVectorProperty *nvp)
{
    //if (strcmp(svp->name,"CONFIG_PROCESS")==0)
    //for (int i=0;i<nvp->nnp;i++) IDLog("Got number %s %s %s %f\n",nvp->device,nvp->name,nvp->np[i].name,nvp->np[i].value);
    emit SigNewNumber(nvp);
}
void IndiCLient::newText(ITextVectorProperty *tvp)
{
    emit gotnewText(tvp);
}
void IndiCLient::newSwitch(ISwitchVectorProperty *svp)
{
    //if (strcmp(svp->name,"CONFIG_PROCESS")==0)
    //for (int i=0;i<svp->nsp;i++) IDLog("Got switch %s %s %s\n",svp->device,svp->name,svp->sp[i].name);
    emit gotnewSwitch(svp);
}
void IndiCLient::newLight(ILightVectorProperty *lvp)
{
    emit gotnewLight(lvp);
}
void IndiCLient::newMessage(INDI::BaseDevice *dp, int messageID)
{
    emit gotnewMessage(dp,messageID);
}
void IndiCLient::newBLOB(IBLOB *bp)
{
    emit SigNewBLOB(bp);
}

bool IndiCLient::sssendNewNumber(QString deviceName, QString propertyName,QString  elementName, double value)
{
    //qDebug() << "taskSendNewNumber" << " " << deviceName << " " << propertyName<< " " << elementName;
    INDI::BaseDevice *dp;
    dp = getDevice(deviceName.toStdString().c_str());

    if (dp== nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << " device. Aborting.";
        return false;
    }
    INumberVectorProperty *prop = nullptr;
    prop = dp->getNumber(propertyName.toStdString().c_str());
    if (prop == nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << " property. Aborting.";
        return false;
    }

    for (int i=0;i<prop->nnp;i++) {
        if (strcmp(prop->np[i].name, elementName.toStdString().c_str()) == 0) {
            prop->np[i].value=value;
            sendNewNumber(prop);
            return true;
        }
    }
    qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << "/" << elementName << " element. Aborting.";
    return false;

}
