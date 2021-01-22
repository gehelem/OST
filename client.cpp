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
    //if (strcmp(svp->name,"CONFIG_PROCESS")==0)
    //for (int i=0;i<svp->nsp;i++) IDLog("Got switch %s %s/%s\n",svp->device,svp->name,svp->sp[i].name);
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
/*!
 * Connects to indi server
 * Should we add host/port ??
 * IMHO this would be overkill as headless instance of OST and indiserver should be 99% hosted on the same machine
 * It would be easy to manage 1% remaining with indiserver chaining capabilities
 */
void MyClient::connectIndi()
{

    if (isServerConnected())
    {
        IDLog("Indi server connected\n");
        return;
    } else {
        setServer("localhost", 7624);
        connectServer();
        return;
    }
    IDLog("Could not connect to indi server\n");

}

/*!
 * Asks every device to connect
 */
void MyClient::connectAllDevices()
{
    std::vector<INDI::BaseDevice *> devs = getDevices();
    for(std::size_t i = 0; i < devs.size(); i++) {
        ISwitchVectorProperty *svp = devs[i]->getSwitch("CONNECTION");

        if (svp==nullptr) {
            IDLog("Couldn't find CONNECTION switch\n");
        } else {
            for (int j=0;j<svp->nsp;j++) {
                if (strcmp(svp->sp[j].name,"CONNECT")==0) {
                    svp->sp[j].s=ISS_ON;
                } else {
                    svp->sp[j].s=ISS_OFF;
                }
            }

            sendNewSwitch(svp);
            if (devs[i]->getDriverInterface() & INDI::BaseDevice::CCD_INTERFACE)
            {
                IDLog("Setting blob mode %s \n",devs[i]->getDeviceName());
                setBLOBMode(B_ALSO,devs[i]->getDeviceName(),nullptr);
            }

        }

    }


}

/*!
 * Asks every device to disconnect
 */
void MyClient::disconnectAllDevices()
{
    std::vector<INDI::BaseDevice *> devs = getDevices();

    for(std::size_t i = 0; i < devs.size(); i++) {
        ISwitchVectorProperty *svp = devs[i]->getSwitch("CONNECTION");

        if (svp==nullptr) {
            IDLog("Couldn't find CONNECTION switch\n");
        } else {
            for (int j=0;j<svp->nsp;j++) {
                if (strcmp(svp->sp[j].name,"DISCONNECT")==0) {
                    svp->sp[j].s=ISS_ON;
                } else {
                    svp->sp[j].s=ISS_OFF;
                }
            }
            sendNewSwitch(svp);

        }

    }


}

/*!
 * Asks every device to load saved configuration
 */
void MyClient::loadDevicesConfs()
{
    std::vector<INDI::BaseDevice *> devs = getDevices();
    qDebug() << "loadDevicesConfs";
    for(std::size_t i = 0; i < devs.size(); i++) {
        IDLog("Loading device conf %s \n",devs[i]->getDeviceName());
        if (devs[i]->isConnected()) {
            ISwitchVectorProperty *svp = devs[i]->getSwitch("CONFIG_PROCESS");

            if (svp==nullptr) {
                IDLog("Couldn't find CONFIG_PROCESS switch\n");
            } else {
                for (int j=0;j<svp->nsp;j++) {
                    if (strcmp(svp->sp[j].name,"CONFIG_LOAD")==0) {
                        svp->sp[j].s=ISS_ON;
                    } else {
                        svp->sp[j].s=ISS_OFF;
                    }
                }
                sendNewSwitch(svp);
            }

        }
    }
}
