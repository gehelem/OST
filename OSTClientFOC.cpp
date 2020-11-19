#include "OSTClientFOC.h"
#include "OSTImage.h"
#include <basedevice.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <inttypes.h>

//using namespace std;

//Includes for this project
#include <structuredefinitions.h>
#include <stellarsolver.h>
//QT Includes
//QT Includes
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>


/**************************************************************************************
**
***************************************************************************************/
OSTClientFOC::OSTClientFOC()
{
    //IDLog("OSTClientFOC %s Device...\n", dp->getDeviceName());
    img.reset(new OSTImage());

}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::newDevice(INDI::BaseDevice *dp)
{
    IDLog("OSTClientFOC %s Device...\n", dp->getDeviceName());
}

/**************************************************************************************
**
*************************************************************************************/
void OSTClientFOC::newProperty(INDI::Property *property)
{
    IDLog("OSTClientFOC %s Device %s property\n",property->getDeviceName(),property->getName());

}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::newNumber(INumberVectorProperty *nvp)
{
    //IDLog("OSTClientFOC Receving new number %s : %g C\n", nvp->device,nvp->np[0].value);
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::newMessage(INDI::BaseDevice *dp, int messageID)
{
    IDLog("OSTClientFOC Recveing message from Server:\n\n########################\n%s\n########################\n\n",
          dp->messageQueue(messageID).c_str());
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::newBLOB(IBLOB *bp)
{
    IDLog("OSTClientFOC newblob from %s\n",bp->name);
    img->LoadFromBlob(bp);
}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::setOSTDevices(std::string wcamera,std::string wfocuser,std::string wmount,std::string wwheel,std::string wguider)
{
    IDLog("OSTClientFOC setOSTDevices %s-%s \n", wcamera.c_str(),wfocuser.c_str());

    focuser = getDevice(wfocuser.c_str());
    if (focuser!=nullptr) {
        IDLog("OSTClientFOC able to find focuser device %s\n", wfocuser.c_str());
        watchDevice(wfocuser.c_str());
    } else {
        IDLog("OSTClientFOC unable to find focuser device %s\n", wfocuser.c_str());
        return;
    }

    camera = getDevice(wcamera.c_str());
    if (camera!=nullptr) {
        IDLog("OSTClientFOC able to find camera device %s\n", wcamera.c_str());
        watchDevice(wcamera.c_str());
        setBLOBMode(B_ALSO, wcamera.c_str(), nullptr);
    } else {
        IDLog("OSTClientFOC unable to find camera device %s\n", wcamera.c_str());
        return;
    }

}
/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::startFocusing(void)
{

    IDLog("OSTClientFOC startFocusing\n");
    INumberVectorProperty *ccd_exposure = nullptr;
    IDLog("OSTClientFOC startFocusing2\n");
    ccd_exposure = camera->getNumber("CCD_EXPOSURE");
    if (ccd_exposure == nullptr)
    {
        IDLog("Error: unable to find %s CCD_EXPOSURE property...\n",camera->getDeviceName());
        return;
    }
    IDLog("OSTClientFOC startFocusing3\n");
    ccd_exposure->np[0].value = 100;
    IDLog("OSTClientFOC startFocusing4\n");
    sendNewNumber(ccd_exposure);
    IDLog("OSTClientFOC startFocusing5\n");
}
