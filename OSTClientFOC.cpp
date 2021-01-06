#include "OSTClientGEN.h"
#include "OSTClientFOC.h"
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
OSTClientFOC::OSTClientFOC(QObject *parent)
{
    QJsonArray props,texts,switchs,nums,lights;
    QJsonObject num,cat,prop,text,swit,group,light;


    thismodule["modulename"]="OSTClientFOC";
    thismodule["modulelabel"]="Focus assistant";
    thismodule["modulehasgroups"]="N";
    thismodule["modulehascategories"]="Y";
    thismodule["groups"]=QJsonArray();

    categories=QJsonArray();
    groups=QJsonArray();
    properties=QJsonArray();

    cat=QJsonObject();
    cat["categoryname"]="OSTFOCCAT_PARAMS";
    cat["categorylabel"]="Parameters and controls";
    cat["modulename"]=thismodule["modulename"];
    categories.append(cat);

    cat=QJsonObject();
    cat["categoryname"]="OSTFOCCAT_DEVICES";
    cat["categorylabel"]="Devices";
    cat["modulename"]=thismodule["modulename"];
    categories.append(cat);

    prop=QJsonObject();
    texts=QJsonArray();
    text=QJsonObject();
    text=Otext("OSTFOCPRO_DEVICES_FOCUSER","Focuser","Focuser Simulator");
    texts.append(text);
    text=QJsonObject();
    text=Otext("OSTFOCPRO_DEVICES_CAMERA","Camera","CCD Simulator");
    texts.append(text);
    prop=Oproperty("OSTFOCPRO_DEVICES","Devices","INDI_TEXT","IP_RW","IPS_IDLE","C","OSTFOCCAT_DEVICES","",
                   texts,
                   thismodule["modulename"].toString(),"OSTFOCCAT_DEVICES","");
    properties.append(prop);

    prop=QJsonObject();
    prop = Oproperty(thismodule["modulename"].toString()+"MESSAGE","Message","INDI_MESSAGE","IP_RW","IPS_IDLE","C","OSTFOCCAT_PARAMS","",
                   QJsonArray(),
                   thismodule["modulename"].toString(),"OSTFOCCAT_PARAMS","");
    properties.append(prop);
    prop=QJsonObject();

    prop = Oproperty(thismodule["modulename"].toString()+"IMAGE","Image","INDI_IMAGE","IP_RW","IPS_IDLE","C","OSTFOCCAT_PARAMS","",
                   QJsonArray(),
                   thismodule["modulename"].toString(),"OSTFOCCAT_PARAMS","");
    properties.append(prop);

    prop=QJsonObject();
    nums=QJsonArray();
    num=QJsonObject();
    num=Onumber("OSTFOCPRO_PARAMS_STARTPOS","Start position",35000,"%.f",0,1000000,1);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTFOCPRO_PARAMS_BACKLASH","Backlash overshoot (inwards)",100,"%.f",0,1000000,1);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTFOCPRO_PARAMS_INCRE","Increment",500,"%.f",0,50000,1);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTFOCPRO_PARAMS_ITERATIONS","Iterations",5,"%.f",0,100,1);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTFOCPRO_PARAMS_EXP","Exposure",5,"%.f",0,100,1);
    nums.append(num);
    prop=Oproperty("OSTFOCPRO_PARAM","Parameters","INDI_NUMBER","IP_RW","IPS_IDLE","C","OSTFOCCAT_PARAMS","",
                   nums,
                   thismodule["modulename"].toString(),"OSTFOCCAT_PARAMS","");
    properties.append(prop);

    prop=QJsonObject();
    nums=QJsonArray();
    num=QJsonObject();
    num=Onumber("OSTFOCPRO_IMGDTA_BESTPOS","Best position",0,"%.f",0,1000000,1);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTFOCPRO_IMGDTA_BESTHFR","Best HFR",0,"%.f",0,1000000,1);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTFOCPRO_IMGDTA_FOCPOS","Focuser position",0,"%.f",0,50000,1);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTFOCPRO_IMGDTA_AVGHFR","Average HFR",0,"%.f",0,50000,1);
    nums.append(num);

    prop=Oproperty("OSTFOCPRO_IMGDTA","Results","INDI_NUMBER","IP_RO","IPS_IDLE","C","OSTFOCCAT_PARAMS","",
                   nums,
                   thismodule["modulename"].toString(),"OSTFOCCAT_PARAMS","");
    properties.append(prop);

    prop=QJsonObject();
    switchs=QJsonArray();
    swit=QJsonObject();
    swit=Oswitch("OSTFOCPRO_ACTIONS_ABORT","Abort","ISS_OFF");
    switchs.append(swit);
    swit=QJsonObject();
    swit=Oswitch("OSTFOCPRO_ACTIONS_COARSE","Coarse focus","ISS_OFF");
    switchs.append(swit);
    swit=QJsonObject();
    swit=Oswitch("OSTFOCPRO_ACTIONS_FINE","Fine focus","ISS_OFF");
    switchs.append(swit);
    swit=QJsonObject();
    swit=Oswitch("OSTFOCPRO_ACTIONS_FRAMING","Start framing","ISS_OFF");
    switchs.append(swit);
    prop=Oproperty("OSTFOCPRO_ACTIONS","Controls","INDI_SWITCH","IP_RW","IPS_IDLE","C","OSTFOCCAT_PARAMS","ISS_NORULE",
                   switchs,
                   thismodule["modulename"].toString(),"OSTFOCCAT_PARAMS","");
    properties.append(prop);


    state="idle";

}
OSTClientFOC::~OSTClientFOC()
{
    //clear();
}

void OSTClientFOC::newDevice(INDI::BaseDevice *dp)
{
    //IDLog("NEWDEVICE %s %s\n",dp->getDeviceName(),OGetText("OSTFOCPRO_DEVICES_CAMERA" ,properties).toStdString().c_str());
    if (strcmp(dp->getDeviceName(), OGetText("OSTFOCPRO_DEVICES_CAMERA" ,properties).toStdString().c_str()) == 0) {
        cameradevice = getDevice(dp->getDeviceName());
        setBLOBMode(B_ALSO, cameradevice->getDeviceName(), nullptr);
        messagelog("cameradevice OK");
    }
    if (strcmp(dp->getDeviceName(), OGetText("OSTFOCPRO_DEVICES_FOCUSER" ,properties).toStdString().c_str()) == 0) {
        focuserdevice = getDevice(dp->getDeviceName());
        messagelog("focuserdevice OK");

    }
}

void OSTClientFOC::processTextMessage(QString message)
{
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8()); // garder
    QJsonObject  obj = jsonResponse.object(); // garder

        if (obj["module"].toString()==thismodule["modulename"].toString()) {
            IDLog("%s message received-------------------------------------\n",thismodule["modulename"].toString().toStdString().c_str());
            IDLog("%s\n",QJsonDocument(obj).toJson(QJsonDocument::Indented).toStdString().c_str());
            if (obj["message"].toString()=="readmodule") {
                updatemodule(thismodule["modulename"].toString());
            }
            if ((obj["message"].toString()=="setswitch")&&(obj["switchname"].toString()=="OSTFOCPRO_ACTIONS_COARSE")) {
                messagelog("Start coarse focus request");
                startFocusing();

            }
            if ((obj["message"].toString()=="setswitch")&&(obj["switchname"].toString()=="OSTFOCPRO_ACTIONS_FINE")) {
                messagelog("Start coarse focus request");
                startFinefocus();

            }            if ((obj["message"].toString()=="setswitch")&&(obj["switchname"].toString()=="OSTFOCPRO_ACTIONS_FRAMING")) {
                messagelog("Start framing request");
                startFraming();

            }
            if ((obj["message"].toString()=="setswitch")&&(obj["switchname"].toString()=="OSTFOCPRO_ACTIONS_ABORT")) {
                messagelog("Abort task");
                tasks =QQueue<Ttask>();
            }
        }

}

/**************************************************************************************
**
***************************************************************************************/
void OSTClientFOC::sssuccess()
{
    if (state=="FN4") {
        //properties["OSTFOCPRO_IMGDTA"];
        //IDLog("%s Analyse %g pos=%g done HFRavg= %g \n",thismodule["modulename"].toString().toStdString().c_str(),Fs,Factpos,img->HFRavg);
        if (img->HFRavg < FBestHFR){
            FBestHFR = img->HFRavg;
            FBestpos = Factpos;
            IDLog("%s New best position found %g %g\n",thismodule["modulename"].toString().toStdString().c_str(),FBestpos,img->HFRavg);
        }
        properties = OSetNumber("OSTFOCPRO_IMGDTA_FOCPOS",properties,Factpos);
        properties = OSetNumber("OSTFOCPRO_IMGDTA_AVGHFR",properties,img->HFRavg);
        properties = OSetNumber("OSTFOCPRO_IMGDTA_BESTPOS",properties,FBestpos);
        properties = OSetNumber("OSTFOCPRO_IMGDTA_BESTHFR",properties,FBestHFR);
        updateproperty(thismodule["modulename"].toString(),"OSTFOCCAT_PARAMS","","OSTFOCPRO_IMGDTA");
        Fs++;
        if (Fs>=Fnb) {
            /*...*/
            //IDLog("%s moving to best position %g\n",thismodule["modulename"].toString().toStdString().c_str(),FBestpos);
            INumberVectorProperty *pos = nullptr;
            pos = focuserdevice->getNumber("ABS_FOCUS_POSITION");
            if (pos== nullptr)
            {
                IDLog("%s Error: unable to find %s ABS_FOCUS_POSITION property...\n",thismodule["modulename"].toString().toStdString().c_str(),focuserdevice->getDeviceName());
                return;
            }
            pos->np[0].value = FBestpos;
            sendNewNumber(pos);
            state="idle";
            properties = OSetNumber("OSTFOCPRO_IMGDTA_FOCPOS",properties,FBestpos);
            updateproperty(thismodule["modulename"].toString(),"OSTFOCCAT_PARAMS","","OSTFOCPRO_IMGDTA");
            emit focusdone();
        } else {
            //IDLog("%s moving to next position %g\n",thismodule["modulename"].toString().toStdString().c_str(),Fs);
            INumberVectorProperty *pos = nullptr;
            pos = focuserdevice->getNumber("ABS_FOCUS_POSITION");
            if (pos== nullptr)
            {
                IDLog("%s Error: unable to find %s ABS_FOCUS_POSITION property...\n",thismodule["modulename"].toString().toStdString().c_str(),focuserdevice->getDeviceName());
                return;
            }
            Factpos = Fpos+Fs*Fincr;
            pos->np[0].value = Factpos;
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
    //if (LogLevel==OSTLOG_ALL) IDLog("%s OSTClientFOC %s Device %s property\n",thismodule["modulename"].toString().toStdString().c_str(),property->getDeviceName(),property->getName());

    if ((strcmp(property->getName(), "ABS_FOCUS_POSITION") == 0)&&(strcmp(property->getDeviceName(), OGetText("OSTFOCPRO_DEVICES_FOCUSER",properties).toStdString().c_str()) == 0))
    {
        //IDLog("%s %s Device %s property\n",thismodule["modulename"].toString().toStdString().c_str(),property->getDeviceName(),property->getName());
        //watchProperty(property->getDeviceName(),property->getName());
        //startFocusing(30000,100,200,10);
    }


}

bool OSTClientFOC::startFinefocus(void)
{

    if (tasks.size()) {
        messagelog("Can't start focusing, i'm busy doing this now :" + tasks.front().tasklabel);
        return true;
    }
}
bool OSTClientFOC::startFocusing(void)
{

    if (tasks.size()) {
        messagelog("Can't start focusing, i'm busy doing this now :" + tasks.front().tasklabel);
        return true;
    }
    Fpos=OGetNumber("OSTFOCPRO_PARAMS_STARTPOS",properties);
    Fbl=OGetNumber("OSTFOCPRO_PARAMS_BACKLASH",properties);
    Fincr=OGetNumber("OSTFOCPRO_PARAMS_INCRE",properties);
    Fnb=OGetNumber("OSTFOCPRO_PARAMS_ITERATIONS",properties);
    Fexp=OGetNumber("OSTFOCPRO_PARAMS_EXP",properties);
    Fs=0;
    INumberVectorProperty *number = nullptr;
    number = focuserdevice->getNumber("ABS_FOCUS_POSITION");
    if (number == nullptr)
    {
        IDLog("Error - unable to find %s - %s property : aborting\n","","ABS_FOCUS_POSITION");
        tasks =QQueue<Ttask>();
        return false;
    }
    FBestpos =number->np[0].value;
    FBestHFR = 999;

    addnewtask(TT_SEND_NUMBER,true,"Coarse focus","init","sendblpos","Asking focuser to go to backlash position",focuserdevice->getDeviceName(),"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION",Fpos-Fbl,"",ISS_OFF);
    addnewtask(TT_WAIT_NUMBER,false,"Coarse focus","init","waitblpos","waiting focuser to go to backlash position",focuserdevice->getDeviceName(),"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION",Fpos-Fbl,"",ISS_OFF);
    for (int i=0;i<Fnb;i++) {
        addnewtask(TT_SEND_NUMBER,true,"Coarse focus","loop","sendfocpos","Asking focuser to go to position",focuserdevice->getDeviceName(),"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION",Fpos + i*Fincr,"",ISS_OFF);
        addnewtask(TT_WAIT_NUMBER,false,"Coarse focus","loop","waitfocpos","waiting focuser to go to position",focuserdevice->getDeviceName(),"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION",Fpos + i*Fincr,"",ISS_OFF);
        addnewtask(TT_SEND_NUMBER,true,"Coarse focus","loop","exprequest","exposure request",cameradevice->getDeviceName(),"CCD_EXPOSURE","CCD_EXPOSURE_VALUE",Fexp,"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,false,"Coarse focus","loop","waitblob","waiting exp","CCD1","CCD1","");
        addnewtask(TT_ANALYSE_SEP,true,"Coarse focus","loop","askanalyse","asking to find stars","CCD1","CCD1","");
        addnewtask(TT_WAIT_SEP,false,"Coarse focus","loop","waitanalyse","waiting to find stars","CCD1","CCD1","");
        addnewtask(TT_SPEC,true,"Coarse focus","loop","store","store pos/hfr","","","",Fpos + i*Fincr,"",ISS_OFF);
    }
    addnewtask(TT_SPEC,true,"Coarse focus","finish","movbest","moving to best position","","","");
    /*addnewtask(TT_WAIT_BLOB  ,false,"Coarse focus","finish","waitblob","waiting exp","CCD1","CCD1","");
    addnewtask(TT_ANALYSE_SEP,true,"Coarse focus","finish","askanalyse","asking to find stars","CCD1","CCD1","");
    addnewtask(TT_WAIT_SEP,false,"Coarse focus","finish","waitanalyse","waiting to find stars","CCD1","CCD1","");*/



    executecurrenttask();
    return true;
}

bool OSTClientFOC::startFraming(void)
{

    if (tasks.size()) {
        messagelog("Can't start framing, i'm busy doing this now :" + tasks.front().tasklabel);
        return true;
    }
    Fexp=OGetNumber("OSTFOCPRO_PARAMS_EXP",properties);
    INumberVectorProperty *number = nullptr;
    number = focuserdevice->getNumber("ABS_FOCUS_POSITION");
    if (number == nullptr)
    {
        IDLog("Error - unable to find %s - %s property : aborting\n","","ABS_FOCUS_POSITION");
        tasks =QQueue<Ttask>();
        return false;
    }
    FBestpos =number->np[0].value;
    FBestHFR = 999;

    addnewtask(TT_SEND_NUMBER,true,"Framing","loop","exprequest","exposure request",cameradevice->getDeviceName(),"CCD_EXPOSURE","CCD_EXPOSURE_VALUE",Fexp,"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,false,"Framing","loop","waitblob","waiting exp","CCD1","CCD1","");
    addnewtask(TT_ANALYSE_SEP,true,"Framing","loop","askanalyse","asking to find stars","CCD1","CCD1","");
    addnewtask(TT_WAIT_SEP,false,"Framing","loop","waitanalyse","waiting to find stars","CCD1","CCD1","");
    addnewtask(TT_SPEC,true,"Framing","loop","addnextloop","Sending new loop","","","");


    executecurrenttask();
    return true;
}


void OSTClientFOC::executespecificcurrenttask(INumberVectorProperty *nvp,ITextVectorProperty *tvp,ISwitchVectorProperty *svp,ILightVectorProperty *lvp,IBLOB *bp)
{
    Ttask task=tasks.front();
    if ((task.taskname=="store")&&(image->FindStarsFinished)) {
        IDLog("store values %f/%f\n",task.value,image->HFRavg);
        if (image->HFRavg < FBestHFR ) {
            FBestpos = task.value;
            FBestHFR = image->HFRavg;
        }
        popnext();
    }
    if (task.taskname=="movbest") {
        INumberVectorProperty *number = nullptr;
        number = focuserdevice->getNumber("ABS_FOCUS_POSITION");
        if (number == nullptr)
        {
            IDLog("Error - unable to find %s - %s property : aborting\n","","ABS_FOCUS_POSITION");
            tasks =QQueue<Ttask>();
            return;
        }
        number->np[0].value = FBestpos;
        sendNewNumber(number);
        popnext();
    }
    if (task.taskname=="addnextloop") {
        addnewtask(TT_SEND_NUMBER,true,"Framing","loop","exprequest","exposure request",cameradevice->getDeviceName(),"CCD_EXPOSURE","CCD_EXPOSURE_VALUE",Fexp,"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,false,"Framing","loop","waitblob","waiting exp","CCD1","CCD1","");
        addnewtask(TT_ANALYSE_SEP,true,"Framing","loop","askanalyse","asking to find stars","CCD1","CCD1","");
        addnewtask(TT_WAIT_SEP,false,"Framing","loop","waitanalyse","waiting to find stars","CCD1","CCD1","");
        addnewtask(TT_SPEC,true,"Framing","loop","addnextloop","Sending new loop","","","");
        popnext();
    }

}
