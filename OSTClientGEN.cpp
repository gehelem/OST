#include "OSTClientGEN.h"

#include <basedevice.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>


/**************************************************************************************
**
***************************************************************************************/
OSTClientGEN::OSTClientGEN()
{

}
void OSTClientGEN::askNewJob(std::string job)
{
    IDLog("%s Controler is asking %s\n",client_name.c_str(),job.c_str());
}


void OSTClientGEN::switchstate(std::string newstate)
{
    IDLog("%s switchstate %s->%s\n",client_name.c_str(),state.c_str(),newstate.c_str());
    state = newstate;
    emit s_newstate();
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::setOSTDevices(std::string wcamera,std::string wfocuser,std::string wmount,std::string wwheel,std::string wguider)
{
    camera_name = wcamera;
    focuser_name = wfocuser;
    mount_name = wmount;
    wheel_name = wwheel;
    guider_name = wguider;
}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::connectallOSTDevices(void)
{
    IDLog("OSTClientGEN connectallOSTDevices\n");
    if (camera!=nullptr) connectDevice(camera->getDeviceName());
    if (focuser!=nullptr) connectDevice(focuser->getDeviceName());
    if (mount!=nullptr) connectDevice(mount->getDeviceName());
    if (wheel!=nullptr) connectDevice(wheel->getDeviceName());
    if (guider!=nullptr) connectDevice(guider->getDeviceName());

}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::disconnectallOSTDevices(void)
{
    IDLog("OSTClientGEN connectallOSTDevices\n");
    if (camera!=nullptr) disconnectDevice(camera->getDeviceName());
    if (focuser!=nullptr) disconnectDevice(focuser->getDeviceName());
    if (mount!=nullptr) disconnectDevice(mount->getDeviceName());
    if (wheel!=nullptr) disconnectDevice(wheel->getDeviceName());
    if (guider!=nullptr) disconnectDevice(guider->getDeviceName());

}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::newDevice(INDI::BaseDevice *dp)
{
    IDLog("%s OSTClientGEN %s Device...\n", client_name.c_str(), dp->getDeviceName());
    if (strcmp(dp->getDeviceName(), camera_name.c_str()) == 0)
    {
        camera = getDevice(dp->getDeviceName());
        watchDevice(camera->getDeviceName());
        setBLOBMode(B_ALSO, camera->getDeviceName(), nullptr);

    }
    if (strcmp(dp->getDeviceName(), focuser_name.c_str()) == 0)
    {
        focuser = getDevice(dp->getDeviceName());
        watchDevice(focuser->getDeviceName());

    }

}

/**************************************************************************************
**
*************************************************************************************/
void OSTClientGEN::newProperty(INDI::Property *property)
{
    //IDLog("%s OSTClientGEN %s Device %s property\n",client_name.c_str(),property->getDeviceName(),property->getName());

}

/**************************************************************************************
**
*************************************************************************************/
void OSTClientGEN::removeProperty(INDI::Property *property)
{
    //IDLog("OSTClientGEN %s Device %s property removed\n",property->getDeviceName(),property->getName());

}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::newNumber(INumberVectorProperty *nvp)
{
   //IDLog("%s OSTClientGEN Recieving new number %s : %g C\n", client_name.c_str(),nvp->device,nvp->np[0].value);
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::newText(ITextVectorProperty *tvp)
{
   //IDLog("OSTClientGEN Receving new text %s : %g C\n", tvp->device,tvp->tp[0].value);
}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::newMessage(INDI::BaseDevice *dp, int messageID)
{

    //IDLog("OSTClientGEN Recveing message from Server:\n\n########################\n%s\n########################\n\n",dp->messageQueue(messageID).c_str());
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::newBLOB(IBLOB *bp)
{
    //IDLog("OSTClientGEN newblob\n");
}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::serverConnected(void)
{
    //IDLog("%s OSTClientGEN Server connected\n", client_name.c_str());
}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::serverDisconnected(int exit_code)
{
    //IDLog("%s OSTClientGEN Server disconnected %i\n", client_name,exit_code);
}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::setClientname(std::string n)
{
    client_name=n;
}
