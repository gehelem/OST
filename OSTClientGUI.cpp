#include "OSTClientGEN.h"
#include "OSTClientGUI.h"
#include "OSTImage.h"
#include "OSTParser.h"
#include <basedevice.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <inttypes.h>
#include <baseclientqt.h>

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
OSTClientGUI::OSTClientGUI(QObject *parent)
{
    QJsonArray props,texts,switchs,nums,lights;
    QJsonObject num,cat,prop,text,swit,group,light;


    thismodule["modulename"]="OSTClientGUI";
    thismodule["modulelabel"]="Guider";
    thismodule["modulehasgroups"]="N";
    thismodule["modulehascategories"]="Y";
    thismodule["groups"]=QJsonArray();

    categories=QJsonArray();
    groups=QJsonArray();
    properties=QJsonArray();

    cat=QJsonObject();
    cat["categoryname"]="OSTGUICAT_PARAMS";
    cat["categorylabel"]="Parameters and controls";
    cat["modulename"]=thismodule["modulename"];
    categories.append(cat);

    cat=QJsonObject();
    cat["categoryname"]="OSTGUICAT_DEVICES";
    cat["categorylabel"]="Devices";
    cat["modulename"]=thismodule["modulename"];
    categories.append(cat);

    prop=QJsonObject();
    texts=QJsonArray();
    text=QJsonObject();
    text=Otext("OSTGUIPRO_DEVICES_MOUNT","Mount","Telescope Simulator");
    texts.append(text);
    text=QJsonObject();
    text=Otext("OSTGUIPRO_DEVICES_GUIDER","Camera","Guide Simulator");
    texts.append(text);
    prop=Oproperty("OSTGUIPRO_DEVICES","Devices","INDI_TEXT","IP_RW","IPS_IDLE","C","OSTGUICAT_DEVICES","",
                   texts,
                   thismodule["modulename"].toString(),"OSTGUICAT_DEVICES","");
    properties.append(prop);

    prop=QJsonObject();
    prop = Oproperty(thismodule["modulename"].toString()+"MESSAGE","Message","INDI_MESSAGE","IP_RW","IPS_IDLE","C","OSTGUICAT_PARAMS","",
                   QJsonArray(),
                   thismodule["modulename"].toString(),"OSTGUICAT_PARAMS","");
    properties.append(prop);
    prop=QJsonObject();

    prop = Oproperty(thismodule["modulename"].toString()+"IMAGE","Image","INDI_IMAGE","IP_RW","IPS_IDLE","C","OSTGUICAT_PARAMS","",
                   QJsonArray(),
                   thismodule["modulename"].toString(),"OSTGUICAT_PARAMS","");
    properties.append(prop);



    prop=QJsonObject();
    switchs=QJsonArray();
    swit=QJsonObject();
    swit=Oswitch("OSTGUIPRO_ACTIONS_ABORT","Abort","ISS_OFF");
    switchs.append(swit);
    swit=QJsonObject();
    swit=Oswitch("OSTGUIPRO_ACTIONS_FRAMING","Framing","ISS_OFF");
    switchs.append(swit);
    swit=QJsonObject();
    swit=Oswitch("OSTGUIPRO_ACTIONS_CALIBRATE","Calibrate","ISS_OFF");
    switchs.append(swit);
    swit=QJsonObject();
    swit=Oswitch("OSTGUIPRO_ACTIONS_GUIDE","Guide","ISS_OFF");
    switchs.append(swit);
    prop=Oproperty("OSTGUIPRO_ACTIONS","Controls","INDI_SWITCH","IP_RW","IPS_IDLE","C","OSTGUICAT_PARAMS","ISS_NORULE",
                   switchs,
                   thismodule["modulename"].toString(),"OSTGUICAT_PARAMS","");
    properties.append(prop);


    state="idle";

}
OSTClientGUI::~OSTClientGUI()
{
    //clear();
}

void OSTClientGUI::newDevice(INDI::BaseDevice *dp)
{
    //IDLog("NEWDEVICE %s %s\n",dp->getDeviceName(),OGetText("OSTGUIPRO_DEVICES_CAMERA" ,properties).toStdString().c_str());
    if (strcmp(dp->getDeviceName(), OGetText("OSTGUIPRO_DEVICES_GUIDER" ,properties).toStdString().c_str()) == 0) {
        guiderdevice = getDevice(dp->getDeviceName());
        setBLOBMode(B_ALSO, guiderdevice->getDeviceName(), nullptr);
        messagelog("guiderdevice OK");
    }
    if (strcmp(dp->getDeviceName(), OGetText("OSTGUIPRO_DEVICES_MOUNT" ,properties).toStdString().c_str()) == 0) {
        mountdevice = getDevice(dp->getDeviceName());
        messagelog("mountdevice OK");

    }
}

void OSTClientGUI::processTextMessage(QString message)
{
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8()); // garder
    QJsonObject  obj = jsonResponse.object(); // garder

        if (obj["module"].toString()==thismodule["modulename"].toString()) {
            IDLog("%s message received-------------------------------------\n",thismodule["modulename"].toString().toStdString().c_str());
            IDLog("%s\n",QJsonDocument(obj).toJson(QJsonDocument::Indented).toStdString().c_str());
            if (obj["message"].toString()=="readmodule") {
                updatemodule(thismodule["modulename"].toString());
            }
            if ((obj["message"].toString()=="setswitch")&&(obj["switchname"].toString()=="OSTGUIPRO_ACTIONS_FRAMING")) {
                messagelog("Start framing");
                startFraming();
            }
            if ((obj["message"].toString()=="setswitch")&&(obj["switchname"].toString()=="OSTGUIPRO_ACTIONS_ABORT")) {
                messagelog("Idle");
                switchstate("idle");
            }

        }

}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGUI::analyse()
{
    if (state=="FRAMING-STELLARSOLVER1") {
        starsref = img->stars;
        IDLog("GUI %i stars found\n",starsref.size());
        messagelog("first image, start framing to biggest star");
        INumberVectorProperty *ccd_frame = nullptr;
        ccd_frame = guiderdevice->getNumber("CCD_FRAME");
        if (ccd_frame == nullptr)
        {
            IDLog("Error: unable to find CCD_FRAME property...\n");
            return;
        }
        ccd_frame->np[0].value = img->stars[0].x - 32; // left
        ccd_frame->np[1].value = img->stars[0].y - 32; // top
        ccd_frame->np[2].value = 64; // width
        ccd_frame->np[3].value = 64; // height*/
        sendNewNumber(ccd_frame);



        INumberVectorProperty *ccd_exposure = nullptr;
        ccd_exposure = guiderdevice->getNumber("CCD_EXPOSURE");
        if (ccd_exposure == nullptr)
        {
            IDLog("Error: unable to find CCD Simulator CCD_EXPOSURE property...\n");
            return;
        }
        ccd_exposure->np[0].value = 1;
        sendNewNumber(ccd_exposure);
        switchstate("FRAMING-EXP2");

    }

    if (state=="FRAMING-STELLARSOLVER2") {
        starsref = img->stars;
        IDLog("GUI %i stars found\n",starsref.size());
        messagelog("first image, start framing to box around first star");

        INumberVectorProperty *ccd_exposure = nullptr;
        ccd_exposure = guiderdevice->getNumber("CCD_EXPOSURE");
        if (ccd_exposure == nullptr)
        {
            IDLog("Error: unable to find CCD Simulator CCD_EXPOSURE property...\n");
            return;
        }
        ccd_exposure->np[0].value = 1;
        sendNewNumber(ccd_exposure);
        switchstate("FRAMING-EXPN");

    }
    if (state=="FRAMING-STELLARSOLVERN") {
        // E(arcs/pixel) = 206 xpixelsize/F
        IDLog("GUI dx %f dy %f \n",img->stars[0].x-starsref[0].x,img->stars[0].y-starsref[0].y);

        INumberVectorProperty *ccd_exposure = nullptr;
        ccd_exposure = guiderdevice->getNumber("CCD_EXPOSURE");
        if (ccd_exposure == nullptr)
        {
            IDLog("Error: unable to find CCD Simulator CCD_EXPOSURE property...\n");
            return;
        }
        ccd_exposure->np[0].value = 1;
        sendNewNumber(ccd_exposure);
        switchstate("FRAMING-EXPN");
    }

}

/**************************************************************************************
**
*************************************************************************************/
void OSTClientGUI::newProperty(INDI::Property *property)
{

}

void OSTClientGUI::newText(ITextVectorProperty *tvp)
{

}
void OSTClientGUI::newSwitch(ISwitchVectorProperty *svp)
{

}


/**************************************************************************************
**
***************************************************************************************/
void OSTClientGUI::newNumber(INumberVectorProperty *nvp)
{



}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientGUI::newMessage(INDI::BaseDevice *dp, int messageID)
{
    //IDLog("%s Recveing message from Server:\n\n########################\n%s\n########################\n\n",thismodule["modulename"].toString().toStdString().c_str(),dp->messageQueue(messageID).c_str());
}

/**************************************************************************************
**
***************************************************************************************/
/*void OSTClientGUI::newBLOB(IBLOB *bp)
{
    if (state=="FRAMING-EXP1") {
        IDLog("%s exposure done : analysing first\n",thismodule["modulename"].toString().toStdString().c_str());
        switchstate("FRAMING-STELLARSOLVER1");
        img->LoadFromBlob(bp);
    }
    if (state=="FRAMING-EXP2") {
        IDLog("%s exposure done : analysing first\n",thismodule["modulename"].toString().toStdString().c_str());
        switchstate("FRAMING-STELLARSOLVER2");
        img->LoadFromBlob(bp);
    }
    if (state=="FRAMING-EXPN") {
        IDLog("%s exposure done : analysing current\n",thismodule["modulename"].toString().toStdString().c_str());
        switchstate("FRAMING-STELLARSOLVERN");
        img->LoadFromBlob(bp);
    }
}
*/
/**************************************************************************************
**
***************************************************************************************/
bool OSTClientGUI::startFraming(void)
{
    if (tasks.size()) {
        IDLog("i'm already busy");
        messagelog("Can't start framing, i'm busy doing this now :" + tasks.front().tasklabel);
        return true;
    }

    addnewtask(TT_SEND_NUMBER,true,"Framing","TEST","First exposure request","TEST","CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",1,"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,false,"Framing","TEST","waiting blob","TEST","CCD1","CCD1","");
    addnewtask(TT_SEND_NUMBER,true,"Framing","TEST","second exp request","TEST","CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",1,"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,false,"Framing","TEST","waiting second blob","TEST","CCD1","CCD1","");
    addnewtask(TT_ANALYSE_SEP,true,"Framing","TEST","analyse request","TEST","CCD1","CCD1","");
    addnewtask(TT_WAIT_SEP   ,false,"Framing","TEST","waiting analyse","TEST","CCD1","CCD1","");

    addnewtask(TT_SEND_NUMBER,true,"Framing","TEST","third exposure request","TEST","CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",100,"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,false,"Framing","TEST","TEST8","TEST","CCD1","CCD1","");
    addnewtask(TT_ANALYSE_SEP,true,"Framing","TEST","TEST9","TEST","CCD1","CCD1","");
    addnewtask(TT_WAIT_SEP   ,false,"Framing","TEST","TEST10","TEST","CCD1","CCD1","");

    addnewtask(TT_SEND_NUMBER,true,"Framing","TEST","TEST11","TEST","CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",100,"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,false,"Framing","TEST","TEST12","TEST","CCD1","CCD1","");
    addnewtask(TT_ANALYSE_SEP,true,"Framing","TEST","TEST13","TEST","CCD1","CCD1","");
    addnewtask(TT_WAIT_SEP   ,false,"Framing","TEST","TEST14","TEST","CCD1","CCD1","");

    executecurrenttask();
    return true;

    img.reset(new OSTImage());
    connect(img.get(),&OSTImage::successSEP,this,&OSTClientGUI::analyse);

    ISwitchVectorProperty *ccd_frame_reset = nullptr;
    ccd_frame_reset = guiderdevice->getSwitch("CCD_FRAME_RESET");
    if (ccd_frame_reset == nullptr)
    {
        IDLog("Error: unable to find CCD_FRAME_RESET property...\n");
        return false;
    }
    ccd_frame_reset->sp[0].s = ISS_ON;
    sendNewSwitch(ccd_frame_reset);

    INumberVectorProperty *ccd_exposure = nullptr;
    ccd_exposure = guiderdevice->getNumber("CCD_EXPOSURE");
    if (ccd_exposure == nullptr)
    {
        IDLog("Error: unable to find CCD_EXPOSURE property...\n");
        return false;
    }
    ccd_exposure->np[0].value = 1;
    sendNewNumber(ccd_exposure);
    switchstate("FRAMING-EXP1");
    return true;
}
