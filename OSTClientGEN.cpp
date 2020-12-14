#include "OSTClientGEN.h"
#include <baseclientqt.h>

/**************************************************************************************
**
***************************************************************************************/
OSTClientGEN::OSTClientGEN(QObject *parent)// : INDI::BaseClientQt(parent)
{
     IDLog("%s OSTClientGEN instanciations\n",client_name.c_str());
}
OSTClientGEN::~OSTClientGEN()
{
    //clear();
}
void OSTClientGEN::askNewJob(std::string job)
{
    IDLog("%s Controler is asking %s\n",client_name.c_str(),job.c_str());
}


void OSTClientGEN::switchstate(std::string newstate)
{
    IDLog("%s switchstate %s->%s\n",client_name.c_str(),state.c_str(),newstate.c_str());
    state = newstate;
    //emit s_newstate();

}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::setOSTDevices(std::string wcamera,std::string wfocuser,std::string wmount,std::string wwheel,std::string wguider)
{
    if (LogLevel==OSTLOG_ALL) IDLog("setostdevices %s %s %s %s %s\n",wcamera.c_str(),wfocuser.c_str(),wmount.c_str(),wwheel.c_str(),wguider.c_str());
    camera_name = wcamera;
    watchDevice(wcamera.c_str());
    focuser_name = wfocuser;
    watchDevice(wfocuser.c_str());
    mount_name = wmount;
    watchDevice(wmount.c_str());
    wheel_name = wwheel;
    watchDevice(wwheel.c_str());
    guider_name = wguider;
    watchDevice(wguider.c_str());

}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::connectallOSTDevices(void)
{
    if (LogLevel==OSTLOG_ALL)  IDLog("OSTClientGEN connectallOSTDevices\n");
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
    if (LogLevel==OSTLOG_ALL) IDLog("OSTClientGEN connectallOSTDevices\n");
    if (camera!=nullptr) disconnectDevice(camera->getDeviceName());
    if (focuser!=nullptr) disconnectDevice(focuser->getDeviceName());
    if (mount!=nullptr) disconnectDevice(mount->getDeviceName());
    if (wheel!=nullptr) disconnectDevice(wheel->getDeviceName());
    if (guider!=nullptr) disconnectDevice(guider->getDeviceName());

}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::setClientname(std::string n)
{
    client_name=n;
}



/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::newDevice(INDI::BaseDevice *dp)
{
    if (LogLevel==OSTLOG_ALL)  IDLog("%s OSTClientGEN %s Device...\n", client_name.c_str(), dp->getDeviceName());
    if (strcmp(dp->getDeviceName(), camera_name.c_str()) == 0)
    {
        if (LogLevel==OSTLOG_ALL)  IDLog("%s OSTClientGEN %s watchdevice\n", client_name.c_str(), dp->getDeviceName());
        camera = getDevice(dp->getDeviceName());
        setBLOBMode(B_ALSO, camera->getDeviceName(), nullptr);

    }
    if (strcmp(dp->getDeviceName(), focuser_name.c_str()) == 0)
    {
        if (LogLevel==OSTLOG_ALL)  IDLog("%s OSTClientGEN %s watchdevice\n", client_name.c_str(), dp->getDeviceName());
        focuser = getDevice(dp->getDeviceName());

    }
    if (strcmp(dp->getDeviceName(), mount_name.c_str()) == 0)
    {
        if (LogLevel==OSTLOG_ALL)  IDLog("%s OSTClientGEN %s watchdevice\n", client_name.c_str(), dp->getDeviceName());
        mount = getDevice(dp->getDeviceName());

    }
    if (strcmp(dp->getDeviceName(), wheel_name.c_str()) == 0)
    {
        if (LogLevel==OSTLOG_ALL)  IDLog("%s OSTClientGEN %s watchdevice\n", client_name.c_str(), dp->getDeviceName());
        wheel = getDevice(dp->getDeviceName());

    }
    if (strcmp(dp->getDeviceName(), guider_name.c_str()) == 0)
    {
        if (LogLevel==OSTLOG_ALL)  IDLog("%s OSTClientGEN %s watchdevice\n", client_name.c_str(), dp->getDeviceName());
        guider = getDevice(dp->getDeviceName());
        setBLOBMode(B_ALSO, guider->getDeviceName(), nullptr);

    }

}

/**************************************************************************************
**
*************************************************************************************/
void OSTClientGEN::newProperty(INDI::Property *property)
{
    if (LogLevel==OSTLOG_ALL) IDLog("%s OSTClientGEN %s Device %s property\n",client_name.c_str(),property->getDeviceName(),property->getName());

}

/**************************************************************************************
**
*************************************************************************************/
void OSTClientGEN::removeProperty(INDI::Property *property)
{
    if (LogLevel==OSTLOG_ALL) IDLog("%s OSTClientGEN %s Device %s property removed\n",client_name.c_str(),property->getDeviceName(),property->getName());

}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::newNumber(INumberVectorProperty *nvp)
{
   if (LogLevel==OSTLOG_ALL) IDLog("%s OSTClientGEN Recieving new number %s %s : %g\n", client_name.c_str(),nvp->device,nvp->np[0].name,nvp->np[0].value);
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::newText(ITextVectorProperty *tvp)
{
   if (LogLevel==OSTLOG_ALL) IDLog("%s OSTClientGEN Receving new text %s %s : %s\n", client_name.c_str(), tvp->device,tvp->tp[0].name,tvp->tp[0].text);
}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::newMessage(INDI::BaseDevice *dp, int messageID)
{
    if (LogLevel==OSTLOG_ALL) IDLog("%s OSTClientGEN new message from server : %s\n", client_name.c_str(), dp->messageQueue(messageID).c_str());
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::newBLOB(IBLOB *bp)
{
    if (LogLevel==OSTLOG_ALL) IDLog("OSTClientGEN newblob\n");
}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::serverConnected(void)
{
    if (LogLevel==OSTLOG_ALL) IDLog("%s OSTClientGEN Server connected\n", client_name.c_str());
}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientGEN::serverDisconnected(int exit_code)
{
    if (LogLevel==OSTLOG_ALL) IDLog("%s OSTClientGEN Server disconnected %i\n", client_name.c_str(),exit_code);
}

