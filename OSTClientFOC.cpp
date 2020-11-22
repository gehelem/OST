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
    img.reset(new OSTImage());
    connect(img.get(),&OSTImage::success,this,&OSTClientFOC::sssuccess);
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::sssuccess()
{
    if (state=="FN4") {
        IDLog("%s Analyse %i done HFRavg= %f \n",client_name.c_str(),Fs,img->HFRavg);
        Fs++;
        if (Fs>=Fnb) {
            /*...*/
            switchstate("F5");
        } else {
            IDLog("%s moving to next position %i\n",client_name.c_str(),Fs);
            INumberVectorProperty *pos = nullptr;
            pos = focuser->getNumber("ABS_FOCUS_POSITION");
            if (pos== nullptr)
            {
                IDLog("%s Error: unable to find %s ABS_FOCUS_POSITION property...\n",client_name.c_str(),focuser->getDeviceName());
                return;
            }
            pos->np[0].value = Fpos+Fs*Fincr;
            sendNewNumber(pos);
            switchstate("FN2");

        }
    }
}

/**************************************************************************************
**
*************************************************************************************/
void OSTClientFOC::newProperty(INDI::Property *property)
{
    if ((strcmp(property->getName(), "ABS_FOCUS_POSITION") == 0)&&(strcmp(property->getDeviceName(), focuser_name.c_str()) == 0))
    {
        IDLog("%s %s Device %s property\n",client_name.c_str(),property->getDeviceName(),property->getName());
        //watchProperty(property->getDeviceName(),property->getName());
    }

}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::newNumber(INumberVectorProperty *nvp)
{
    if ((strcmp(nvp->name, "ABS_FOCUS_POSITION") == 0)&&(strcmp(nvp->device, focuser->getDeviceName()) == 0))
    {
       IDLog("%s Device %s new %s value : %g\n",client_name.c_str(),nvp->device,nvp->name,nvp->np[0].value);

       if ((state=="F1")&&(nvp->np[0].value=Fpos-Fbl)) {
           IDLog("%s first position reached, going to start position\n",client_name.c_str());
           nvp->np[0].value = Fpos;
           sendNewNumber(nvp);
           switchstate("FN2");
       }

       if ((state=="FN2") && (nvp->np[0].value=Fpos+Fs*Fincr)) {
           //Fs++;
           if (Fs >= Fnb) {
               /*...*/

           } else {
               IDLog("%s position %i reached, taking exposure\n",client_name.c_str(),Fs);
               INumberVectorProperty *ccd_exposure = nullptr;
               ccd_exposure = camera->getNumber("CCD_EXPOSURE");
               if (ccd_exposure == nullptr)
               {
                   IDLog("Error: unable to find CCD Simulator CCD_EXPOSURE property...\n");
                   return;
               }
               ccd_exposure->np[0].value = 100;
               sendNewNumber(ccd_exposure);
               switchstate("FN3");

           }
       }
    }

}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::newMessage(INDI::BaseDevice *dp, int messageID)
{
    //IDLog("%s Recveing message from Server:\n\n########################\n%s\n########################\n\n",client_name.c_str(),dp->messageQueue(messageID).c_str());
}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::newBLOB(IBLOB *bp)
{
    IDLog("%s newblob from %s\n",client_name.c_str(),bp->name);
    if (state=="FN3") {
        IDLog("%s exposure %i done : analysing\n",client_name.c_str(),Fs);
        switchstate("FN4");
        img->LoadFromBlob(bp);
    }
}

/**************************************************************************************
**
***************************************************************************************/
bool OSTClientFOC::startFocusing(int start, int backlash,int incr, int nb)
{
    if (!(state=="idle")) {
        IDLog("%s startFocusing\n",client_name.c_str());
        return false;
    }
    Fpos=start;
    Fbl=backlash;
    Fincr=incr;
    Fnb=nb;
    Fs=0;
    switchstate("init");
    IDLog("%s startFocusing begin=%i backlash=%i incr=%i nb=%i\n",client_name.c_str(),Fpos,Fbl,Fincr,Fnb);
    INumberVectorProperty *pos = nullptr;
    pos = focuser->getNumber("ABS_FOCUS_POSITION");
    if (pos== nullptr)
    {
        IDLog("%s Error: unable to find %s ABS_FOCUS_POSITION property...\n",client_name.c_str(),focuser->getDeviceName());
        return false;
    }
    pos->np[0].value = Fpos-Fbl;
    sendNewNumber(pos);
    switchstate("F1");
    return true;
}
