#include "mGuider.h"

MGuider::MGuider(MyClient *cli,Properties *properties) : Module(cli,properties)
{
}
MGuider::~MGuider()
{
    //
}

void MGuider::initProperties(void)
{
    modulename="guider";
    createMyModule("Guider",30);
    createMyProp("messages","Messages",PT_MESSAGE,"","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",00);
    appendMyElt ("messages","messageselt","","Messages","","");

    createMyCateg("main","Main",10);
    createMyProp("statusprop","Status",PT_TEXT,"main","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("statusprop","status","","Status","","");

    createMyProp("devices","Devices",PT_TEXT,"main","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",99);
    appendMyElt ("devices","camera"  , "CCD Simulator"        ,"Camera","","");
    appendMyElt ("devices","mount"   , "Telescope Simulator"  ,"Mount","","");

    createMyProp("results","Results",PT_NUM,"main","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",55);
    appendMyElt ("results","dra"    , 0        , "RA (h) drift"  ,"","");
    appendMyElt ("results","ddec"   , 0        , "DEC (°) drift" ,"","");
    appendMyElt ("results","dx"     , 0        , "X drift (pix)" ,"","");
    appendMyElt ("results","dy"     , 0        , "Y drift (pix)" ,"","");

    createMyProp("buttonsprop","Actions",PT_SWITCH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",20);
    appendMyElt ("buttonsprop","calib"      , OSS_OFF       , "Calibration","","");
    appendMyElt ("buttonsprop","abort"      , OSS_OFF       , "Abort","","");

    createMyProp("parms","Parameters",PT_NUM,"main","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",30);
    appendMyElt ("parms","exposure"     , 1                   , "Exposure","","");
    appendMyElt ("parms","calpulse"     , 20               , "Calibration pulse (ms)","","");
    appendMyElt ("parms","box"          , 100              , "Box size (pixels)","","");

    createMyProp("image","Image",PT_IMAGE,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",50);
    appendMyElt ("image","imagesep"  , IM_FULL       , "Image result","","","imageguide.jpeg","/var/www/html");

    createMyProp("guider","Guider plot",PT_GRAPH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",60);
    OGraph gra;
    gra.name="guiderdrift" ;
    gra.label="Guider drift";
    gra.gtype="2DPLOT";
    gra.V0label="RA";
    gra.V1label="DEC";
    appendMyElt ("guider","guiderdrift"  , gra);
    gra.name="guidercalib" ;
    gra.label="Guider calibration";
    gra.gtype="2DPLOT";
    gra.V0label="RA";
    gra.V1label="DEC";
    appendMyElt ("guider","guidercalib"  , gra);


    createMyCateg("adv","Advanced parameters",80);
    createMyProp("indiprops","Indi properties",PT_TEXT,"adv","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("indiprops","expp" , "CCD_EXPOSURE"           ,"Exposure property","","");
    appendMyElt ("indiprops","expe" , "CCD_EXPOSURE_VALUE"     ,"Exposure element","","");
    appendMyElt ("indiprops","guideNS" , "TELESCOPE_TIMED_GUIDE_NS" ,"Pulse N/S","","");
    appendMyElt ("indiprops","guideWE" , "TELESCOPE_TIMED_GUIDE_WE" ,"Pulse W/E","","");
    appendMyElt ("indiprops","guideN"  , "TIMED_GUIDE_N" ,"Pulse N","","");
    appendMyElt ("indiprops","guideS"  , "TIMED_GUIDE_S" ,"Pulse S","","");
    appendMyElt ("indiprops","guideW"  , "TIMED_GUIDE_W" ,"Pulse W","","");
    appendMyElt ("indiprops","guideE"  , "TIMED_GUIDE_E" ,"Pulse E","","");
}

void MGuider::test(void)
{
    qDebug() << "test";
}

void MGuider::slotvalueChangedFromCtl(Prop prop)
{
    if (prop.modulename!=modulename) return;

    if ((prop.typ==PT_SWITCH) && (prop.propname=="buttonsprop") )
    {
        prop.state=OPS_BUSY;
        setMyProp("buttonsprop",prop);
        /*if (prop.s["something"].s==OSS_ON) startSomething();*/

    }
    if ((prop.typ==PT_SWITCH) && (prop.propname=="buttonsprop") )
    {
        Prop myprop = getMyProp(prop.propname);
        myprop.state=OPS_BUSY;
        setMyProp(prop.propname,myprop);
        if (prop.s["calib"].s==OSS_ON) startCalibration();
        if (prop.s["abort"].s==OSS_ON) abort();


    }
}

void MGuider::abort(void)
{
    qDebug() << "abort request from ws";
    tasks =QQueue<Ttask>();
    setMyElt("statusprop","status","idle");
    Prop prop = getMyProp("buttonsprop");
    prop.state=OPS_OK;
    setMyProp("buttonsprop",prop);
}
void MGuider::startCalibration(void)
{
    qDebug() << "Calibration request from ws" << tasks.size();
    setMyElt("messages","messageselt","Start calibration");

    QString expp        = getMyTxt("indiprops","expp");
    QString expe        = getMyTxt("indiprops","expe");
    QString camera      = getMyTxt("devices","camera");
    QString mount       = getMyTxt("devices","mount");
    QString radecp      = getMyTxt("indiprops","radecp");

    double calpulse    = getMyNum("parms","calpulse");
    double box         = getMyNum("parms","box");
    double iexposure   = getMyNum("parms","exposure");

    resetMyGra("guider","guiderdrift");
    resetMyGra("guider","guidercalib");

    addnewtask(TT_SEND_SWITCH,"fram01","Reset frame",true,   camera,"CCD_FRAME_RESET","RESET",0,"",ISS_ON);
    addnewtask(TT_WAIT_PROP,  "fram02","Wait frame reset",true, camera,"CCD_FRAME_RESET","RESET",0,"",ISS_OFF);
    addnewtask(TT_SEND_NUMBER,"fram1" ,"Exp. request",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,"fram2" ,"Exp. waiting",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_ANALYSE_SEP,"fram3" ,"Analyse request",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_WAIT_SEP   ,"fram4" ,"Waiting analyse",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_SPEC       ,"fram5" ,"Save image",true,   camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_SPEC       ,"fram6" ,"Select star",true,   camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_WAIT_PROP,  "fram7","Wait frame reset",true, camera,"CCD_FRAME","",0,"",ISS_OFF);
    addnewtask(TT_SEND_NUMBER,"fram1" ,"Exp. request",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,"fram2" ,"Exp. waiting",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_ANALYSE_SEP,"fram3" ,"Analyse request",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_WAIT_SEP   ,"fram4" ,"Waiting analyse",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_SPEC       ,"fram5" ,"Save image",true,   camera,expp,expe,iexposure,"",ISS_OFF);
    for (int i=0;i<10;i++) {
        addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram5","save image",true,   camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram8" ,"diplay drif",true,   camera,expp,expe,iexposure,"",ISS_OFF);
    }
    addnewtask(TT_SPEC       ,"fram5" ,"Save image",true,   camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_WAIT_PROP,  "fram7","Wait frame reset",true, camera,"CCD_FRAME","",0,"",ISS_OFF);


    executeTask(tasks.front());

}

void MGuider::executeTaskSpec(Ttask task)
{
    QString expp        = getMyTxt("indiprops","expp");
    QString expe        = getMyTxt("indiprops","expe");
    QString camera      = getMyTxt("devices","camera");
    QString mount       = getMyTxt("devices","mount");

    double calpulse    = getMyNum("parms","calpulse");
    double box         = getMyNum("parms","box");
    double iexposure   = getMyNum("parms","exposure");

    if (task.taskname=="fram5") {

        image->saveStretchedToJpeg(getMyImg("image","imagesep").f+"/"+getMyImg("image","imagesep").url,100);
        setMyElt("image","imagesep",getMyImg("image","imagesep").url,getMyImg("image","imagesep").f);
        setMyElt("messages","messageselt","Image saved");

        popnext();
        executeTask(tasks.front());
    }
    if (task.taskname=="fram6") {

        INDI::BaseDevice *dp;
        dp = indiclient->getDevice(camera.toStdString().c_str());
        if (dp== nullptr)
        {
            qDebug() << "Error - unable to find " << camera << " device. Aborting.";
            tasks =QQueue<Ttask>();
            return;
        }

        INumberVectorProperty *prop = nullptr;
        prop = dp->getNumber("CCD_FRAME");
        if (prop == nullptr)
        {
            qDebug() << "Error - unable to find " << camera << "/" << "CCD_FRAME" << " property. Aborting.";
            tasks =QQueue<Ttask>();
            return ;
        }

        for (int i=0;i<prop->nnp;i++) {
            if (strcmp(prop->np[i].name, "X") == 0) {
                prop->np[i].value=image->stars[0].x-box/2;
            }
            if (strcmp(prop->np[i].name, "Y") == 0) {
                prop->np[i].value=image->stars[0].y-box/2;
            }
            if (strcmp(prop->np[i].name, "WIDTH") == 0) {
                prop->np[i].value=box;
            }
            if (strcmp(prop->np[i].name, "HEIGHT") == 0) {
                prop->np[i].value=box;
            }
        }

        indiclient->sendNewNumber(prop);
        popnext();
        //executeTask(tasks.front());

    }
    if (task.taskname=="fram8") {

        qDebug() << "drift " <<  stars[0].x-box/2 <<  stars[0].y-box/2;
        setMyElt("messages","messageselt","Drift");
        setMyElt("results","dx",stars[0].x-box/2);
        setMyElt("results","dy",stars[0].y-box/2);

        popnext();
        executeTask(tasks.front());
    }
}
void MGuider::executeTaskSpec(Ttask task,IBLOB *bp)
{
    Q_UNUSED(task);
    Q_UNUSED(bp);
}
void MGuider::executeTaskSpec(Ttask task,INumberVectorProperty *nvp)
{
    if (task.taskname=="fram7") {

        if (   (strcmp(tasks.front().devicename.toStdString().c_str(),nvp->device)==0)
            && (strcmp(tasks.front().propertyname.toStdString().c_str(),nvp->name)==0)
            && (nvp->s==IPS_OK) ) {
            setMyElt("messages","messageselt","Frame box OK");
            popnext();
            executeTask(tasks.front());
        }

    }
}
void MGuider::executeTaskSpec(Ttask task,ISwitchVectorProperty *svp)
{
    if (task.taskname=="fram02") {

        if (   (strcmp(tasks.front().devicename.toStdString().c_str(),svp->device)==0)
            && (strcmp(tasks.front().propertyname.toStdString().c_str(),svp->name)==0)
            && (svp->s==IPS_OK) ) {
            setMyElt("messages","messageselt","Frame reset OK");
            popnext();
            executeTask(tasks.front());
        }

    }

}
void MGuider::executeTaskSpec(Ttask task,ITextVectorProperty *tvp)
{
    Q_UNUSED(task);
    Q_UNUSED(tvp);
}
void MGuider::executeTaskSpec(Ttask task,ILightVectorProperty *lvp)
{
    Q_UNUSED(task);
    Q_UNUSED(lvp);
}
