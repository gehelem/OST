#include "mFocuser.h"
#include "polynomialfit.h"

MFocuser::MFocuser(Properties *properties) : Module(properties)
{
}
MFocuser::~MFocuser()
{
    //
}

void MFocuser::initProperties(void)
{
    modulename="focuser";
    createMyModule("Focuser",20);
    createMyProp("messages","Messages",PT_MESSAGE,"","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",00);
    appendMyElt ("messages","messageselt","","Messages","","");

    createMyCateg("main","Main control",10);
    createMyProp("statusprop","Status",PT_TEXT,"main","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("statusprop","status","","Status","","");

    createMyProp("devices","Devices",PT_TEXT,"main","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",99);
    appendMyElt ("devices","camera"  , "CCD Simulator"        ,"Camera","","");
    appendMyElt ("devices","focuser" , "Focuser Simulator"    ,"Focuser","","");

    createMyProp("valuesprop","Values",PT_NUM,"main","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",50);
    appendMyElt ("valuesprop","hfravg"       , 99       , "Average HFR","","");
    appendMyElt ("valuesprop","starscount"   , 0        , "Stars found","","");
    appendMyElt ("valuesprop","focuspos"     , 0        , "Focuser position","","");
    appendMyElt ("valuesprop","bestpos"      , 0        , "Best position","","");
    appendMyElt ("valuesprop","besthfravg"   , 0        , "Best HFR","","");

    createMyProp("image","Image",PT_IMAGE,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",50);
    //appendMyElt ("image","imagefoc"  , IM_FULL       , "focus image","","","imagefoc.jpeg","/var/www/html");
    appendMyElt ("image","imagefocst"  , IM_FULL       , "stretched focus image","","","imagefocst.jpeg","/var/www/html");

    createMyProp("curve","Focus plot",PT_GRAPH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",60);
    OGraph gra;
    gra.name="curve1" ;
    gra.label="HFR";
    gra.gtype="2D";
    gra.V0label="Position";
    gra.V1label="HFR";
    appendMyElt ("curve","curve1"  , gra);

    /*createMyProp("histo","Histogramme",PT_GRAPH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",70);
    OGraph grafoc;
    grafoc.name="histofoc" ;
    grafoc.label="Val";
    grafoc.gtype="2D";
    grafoc.V0label="ADU";
    grafoc.V1label="Intensity";
    appendMyElt ("histo","histofoc"  , grafoc);*/

    createMyProp("buttonsprop","Actions",PT_SWITCH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",20);
    appendMyElt ("buttonsprop","loop"       , OSS_OFF       , "Continuous shooting","","");
    appendMyElt ("buttonsprop","coarse"     , OSS_OFF       , "Coarse focus","","");
    appendMyElt ("buttonsprop","fine"       , OSS_OFF       , "Fine focus","","");
    appendMyElt ("buttonsprop","abort"      , OSS_OFF       , "Abort","","");

    createMyProp("parms","Focus parameters",PT_NUM,"main","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",30);
    appendMyElt ("parms","startpos"     , 32000         , "Start position","","");
    appendMyElt ("parms","overshoot"    , 200           , "Backlash overshoot","","");
    appendMyElt ("parms","incre"        , 500           , "Incrementation","","");
    appendMyElt ("parms","iterations"   , 20             , "Iterations","","");
    appendMyElt ("parms","exposure"     , 100             , "Exposure","","");

    createMyCateg("adv","Advanced parameters",80);

    createMyProp("indiprops","Indi properties",PT_TEXT,"adv","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("indiprops","expp" , "CCD_EXPOSURE"           ,"Exposure property","","");
    appendMyElt ("indiprops","expe" , "CCD_EXPOSURE_VALUE"     ,"Exposure element","","");
    appendMyElt ("indiprops","focp" , "ABS_FOCUS_POSITION"     ,"Position property","","");
    appendMyElt ("indiprops","foce" , "FOCUS_ABSOLUTE_POSITION","Position element","","");


}

void MFocuser::slotvalueChangedFromCtl(Prop prop)
{
    if (prop.modulename!=modulename) return;

    if ((prop.typ==PT_SWITCH) && (prop.propname=="buttonsprop"))
    {
        Prop myprop = getMyProp(prop.propname);
        myprop.state=OPS_BUSY;
        setMyProp(prop.propname,myprop);
        if (prop.s["coarse"].s==OSS_ON) startCoarse();
        if (prop.s["fine"].s==OSS_ON) startFine();
        if (prop.s["framing"].s==OSS_ON) startFraming();
        if (prop.s["abort"].s==OSS_ON) abort();

    }
    if ((prop.typ==PT_NUM) && (prop.propname=="parms"))
    {

        Prop myprop = getMyProp(prop.propname);
        foreach (ONumber onum, prop.n) {
            myprop.n[onum.name].value=onum.value;
        }
        setMyProp(prop.propname,myprop);
    }
    if ((prop.typ==PT_TEXT) && (prop.propname=="devices"))
    {
        Prop myprop = getMyProp(prop.propname);
        foreach (OText otext, prop.t) {
            myprop.t[otext.name].text=otext.text;
        }
        setMyProp(prop.propname,myprop);
    }

}

void MFocuser::startFraming(void)
{
    qDebug() << "start framing request from ws";
}
void MFocuser::startFine(void)
{
    qDebug() << "start fine focus request from ws";
    posvector.clear();
    hfdvector.clear();
    coefficients.clear();
    QString expp        = getMyTxt("indiprops","expp");
    QString expe        = getMyTxt("indiprops","expe");
    QString focp        = getMyTxt("indiprops","focp");
    QString foce        = getMyTxt("indiprops","foce");
    QString camera      = getMyTxt("devices","camera");
    QString focuser     = getMyTxt("devices","focuser");
    double istartpos    = getMyNum("parms","startpos");
    double iovershoot   = getMyNum("parms","overshoot");
    double iincre       = getMyNum("parms","incre");
    double iiterations  = getMyNum("parms","iterations");
    double iexposure    = getMyNum("parms","exposure");
    resetMyGra("curve","curve1");

    setMyElt("valuesprop","besthfravg",99);

    addnewtask(TT_SEND_NUMBER,"sendblpos","Asking focuser to go to backlash position",false,focuser,focp,foce,istartpos-iovershoot,"",ISS_OFF);
    addnewtask(TT_WAIT_NUMBER,"waitblpos","Waiting focuser to go to backlash position",false,focuser,focp,foce,istartpos-iovershoot,"",ISS_OFF);
    addnewtaskFrameReset      ("fram00","Reset frame",false,camera);
    addnewtaskWaitFrameResetOk("fram01","Wait frame reset",false,camera);
    for (int i=0;i<iiterations;i++) {
        addnewtask(TT_SEND_NUMBER,"sendfocpos","Asking focuser to move",false,focuser,focp,foce,istartpos+i*iincre,"",ISS_OFF);
        addnewtask(TT_WAIT_NUMBER,"waitfocpos","Waiting focuser",false,focuser,focp,foce,istartpos+i*iincre,"",ISS_OFF);
        addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram45","save image",true,   camera,expp,expe,istartpos+i*iincre,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram55","store pos/hfr",true,   camera,expp,expe,istartpos+i*iincre,"",ISS_OFF);

    }
    addnewtask(TT_SPEC,"gobest","Asking focuser to go to best position",true,focuser,"","",0,"",ISS_OFF);



    //dumpTasks();
    executeTask(tasks.front());
}

void MFocuser::startCoarse(void)
{
    qDebug() << "start coarse request from ws" << tasks.size();

    QString expp        = getMyTxt("indiprops","expp");
    QString expe        = getMyTxt("indiprops","expe");
    QString focp        = getMyTxt("indiprops","focp");
    QString foce        = getMyTxt("indiprops","foce");
    QString camera      = getMyTxt("devices","camera");
    QString focuser     = getMyTxt("devices","focuser");
    double istartpos    = getMyNum("parms","startpos");
    double iovershoot   = getMyNum("parms","overshoot");
    double iincre       = getMyNum("parms","incre");
    double iiterations  = getMyNum("parms","iterations");
    double iexposure    = getMyNum("parms","exposure");
    resetMyGra("curve","curve1");

    setMyElt("valuesprop","besthfravg",99);

    addnewtask(TT_SEND_NUMBER,"sendblpos","Asking focuser to go to backlash position",false,focuser,focp,foce,istartpos-iovershoot,"",ISS_OFF);
    addnewtask(TT_WAIT_NUMBER,"waitblpos","Waiting focuser to go to backlash position",false,focuser,focp,foce,istartpos-iovershoot,"",ISS_OFF);
    addnewtaskFrameReset      ("fram00","Reset frame",false,camera);
    addnewtaskWaitFrameResetOk("fram01","Wait frame reset",false,camera);
    for (int i=0;i<iiterations;i++) {
        addnewtask(TT_SEND_NUMBER,"sendfocpos","Asking focuser to move",false,focuser,focp,foce,istartpos+i*iincre,"",ISS_OFF);
        addnewtask(TT_WAIT_NUMBER,"waitfocpos","Waiting focuser",false,focuser,focp,foce,istartpos+i*iincre,"",ISS_OFF);
        addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram45","save image",true,   camera,expp,expe,istartpos+i*iincre,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram5","store pos/hfr",true,   camera,expp,expe,istartpos+i*iincre,"",ISS_OFF);

    }
    addnewtask(TT_SPEC,"gobest","Asking focuser to go to best position",true,focuser,"","",0,"",ISS_OFF);



    //dumpTasks();
    executeTask(tasks.front());
}

void MFocuser::abort(void)
{
    qDebug() << "abort request from ws";
    tasks =QQueue<Ttask>();
    setMyElt("statusprop","status","idle");
    Prop prop = getMyProp("buttonsprop");
    prop.state=OPS_OK;
    setMyProp("buttonsprop",prop);
}


void MFocuser::executeTaskSpec(Ttask task)
{
    QString expp        = getMyTxt("indiprops","expp");
    QString expe        = getMyTxt("indiprops","expe");
    QString focp        = getMyTxt("indiprops","focp");
    QString foce        = getMyTxt("indiprops","foce");
    QString camera      = getMyTxt("devices","camera");
    QString focuser     = getMyTxt("devices","focuser");
    double iexposure    = getMyNum("parms","exposure");

    if (task.taskname=="fram5") {
        Prop prop = getMyProp("valuesprop");
        prop.n["hfravg"].value=image->HFRavg;
        prop.n["starscount"].value=image->stars.count();
        prop.n["focuspos"].value=task.value;
        OGraphValue v;
        v.v0=task.value;
        v.v1=image->HFRavg;
        appendMyGra("curve","curve1",v);

        if (prop.n["besthfravg"].value>image->HFRavg) {
            prop.n["besthfravg"].value=image->HFRavg;
            prop.n["bestpos"].value=task.value;
        }
        setMyProp(prop.propname,prop);
        popnext();
        executeTask(tasks.front());
    }

    if (task.taskname=="fram55") {
        Prop prop = getMyProp("valuesprop");
        prop.n["hfravg"].value=image->HFRavg;
        prop.n["starscount"].value=image->stars.count();
        prop.n["focuspos"].value=task.value;
        OGraphValue v;
        v.v0=task.value;
        v.v1=image->HFRavg;
        appendMyGra("curve","curve1",v);
        posvector.push_back(task.value);
        hfdvector.push_back(image->HFRavg);

        if (posvector.size() > 2)
        {
            double coeff[3];
            polynomialfit(posvector.size(), 3, posvector.data(), hfdvector.data(), coeff);
            prop.n["bestpos"].value = -coeff[1]/(2*coeff[2]);
        }

        setMyProp(prop.propname,prop);
        popnext();
        executeTask(tasks.front());
    }

    if (task.taskname=="check5") {
        Prop prop = getMyProp("valuesprop");
        prop.n["hfravg"].value=image->HFRavg;
        prop.n["starscount"].value=image->stars.count();
        prop.n["focuspos"].value=task.value;

        setMyProp(prop.propname,prop);
        popnext();
    }
    if (task.taskname=="gobest") {

        addnewtask(TT_SEND_NUMBER,"movebest","Asking focuser to go to best position",false,focuser,focp,foce,getMyNum("valuesprop","bestpos"),"",ISS_OFF);
        addnewtaskWaitPropOk(     "waitbest","Waiting focuser to go to best position",false,focuser,focp);
        addnewtask(TT_SEND_NUMBER,"check1","Check request",false,   camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"check2","Check waiting",false,   camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"check3","Check Analyse request",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"check4","Check Waiting analyse",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"check5","Finish",true,   camera,expp,expe,getMyNum("valuesprop","bestpos"),"",ISS_OFF);


        popnext();
        executeTask(tasks.front());
    }
    if (task.taskname=="fram45") {
        //image->appendStarsFound();

        //image->saveToJpeg(getMyImg("image","imagefoc").f+"/"+getMyImg("image","imagefoc").url,100);
        //setMyElt("image","imagefoc",getMyImg("image","imagefoc").url,getMyImg("image","imagefoc").f);
        image->saveStretchedToJpeg(getMyImg("image","imagefocst").f+"/"+getMyImg("image","imagefocst").url,100);
        setMyElt("image","imagefocst",getMyImg("image","imagefocst").url,getMyImg("image","imagefocst").f);
        /*OGraph graphf = getMyGraph("histo","histofoc");
        graphf.values.clear();
        OGraphValue val;
        for (int i=0;i<256;i++) {
            val.v0=i;
            val.v1=image->histogram256[i];
            graphf.values.append(val);
        };
        setMyElt("histo","histofoc",graphf);*/

        popnext();
        executeTask(tasks.front());
    }

}
void MFocuser::executeTaskSpec(Ttask task,IBLOB *bp)
{
    Q_UNUSED(task);
    Q_UNUSED(bp);
}
void MFocuser::executeTaskSpec(Ttask task,INumberVectorProperty *nvp)
{
    Q_UNUSED(task);
    Q_UNUSED(nvp);
}
void MFocuser::executeTaskSpec(Ttask task,ISwitchVectorProperty *svp)
{
    Q_UNUSED(task);
    Q_UNUSED(svp);
}
void MFocuser::executeTaskSpec(Ttask task,ITextVectorProperty *tvp)
{
    Q_UNUSED(task);
    Q_UNUSED(tvp);
}
void MFocuser::executeTaskSpec(Ttask task,ILightVectorProperty *lvp)
{
    Q_UNUSED(task);
    Q_UNUSED(lvp);
}
