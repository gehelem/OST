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
MyClient::MyClient(QObject *parent)
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
    emit gotnewProperty(property);
}
void MyClient::removeProperty(INDI::Property *property)
{
    emit gotremoveProperty(property);
}
void MyClient::newNumber(INumberVectorProperty *nvp)
{
    emit gotnewNumber(nvp);
}
void MyClient::newText(ITextVectorProperty *tvp)
{
    emit gotnewText(tvp);
}
void MyClient::newSwitch(ISwitchVectorProperty *svp)
{
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

