#include "OSTClientSEQ.h"

#include <basedevice.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

/**************************************************************************************
**
***************************************************************************************/
OSTClientSEQ::OSTClientSEQ(QObject *parent) :  OSTClientGEN(parent)
{
//    ostcamera = nullptr;
}
OSTClientSEQ::~OSTClientSEQ()
{
    //clear();
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientSEQ::newDevice(INDI::BaseDevice *dp)
{
    IDLog("OSTClientSEQ %s Device...\n", dp->getDeviceName());
}

/**************************************************************************************
**
*************************************************************************************/
void OSTClientSEQ::newProperty(INDI::Property *property)
{
    IDLog("OSTClientSEQ %s Device %s property\n",property->getDeviceName(),property->getName());

}

/**************************************************************************************
**
***************************************************************************************/
/*void OSTClientSEQ::newNumber(INumberVectorProperty *nvp)
{
    IDLog("OSTClientSEQ Receving new number %s : %g C\n", nvp->device,nvp->np[0].value);
}*/

/**************************************************************************************
**
***************************************************************************************/
void OSTClientSEQ::newMessage(INDI::BaseDevice *dp, int messageID)
{
    IDLog("OSTClientSEQ Recveing message from Server:\n\n########################\n%s\n########################\n\n",
          dp->messageQueue(messageID).c_str());
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientSEQ::newBLOB(IBLOB *bp)
{
    IDLog("OSTClientSEQ newblob\n");

}
