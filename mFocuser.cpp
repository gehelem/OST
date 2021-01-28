#include "mFocuser.h"

MFocuser::MFocuser(MyClient *cli,Properties *properties) : Module(cli,properties)
{
}
MFocuser::~MFocuser()
{
    //
}

void MFocuser::initProperties(void)
{
    modulename="focuser";
    createMyModule("Focuser");
    createMyCateg("main","Main control");
    createMyProp("statusprop","Status",PT_TEXT,"main","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","");
    appendMyElt ("statusprop","status","","Status","","");

    createMyProp("devices","Devices",PT_TEXT,"main","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","");
    appendMyElt ("devices","camera"  , "CCD Simulator"        ,"Camera","","");
    appendMyElt ("devices","focuser" , "Focuser Simulator"    ,"Focuser","","");

    createMyProp("valuesprop","Values",PT_NUM,"main","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","");
    appendMyElt ("valuesprop","hfravg"       , 99       , "Average HFR","","");
    appendMyElt ("valuesprop","starscount"   , 0        , "Stars found","","");
    appendMyElt ("valuesprop","focuspos"     , 0        , "Focuser position","","");
    appendMyElt ("valuesprop","bestpos"      , 0        , "Best position","","");
    appendMyElt ("valuesprop","besthfravg"   , 0        , "Best HFR","","");

    createMyProp("buttonsprop","Actions",PT_SWITCH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","");
    appendMyElt ("buttonsprop","loop"       , OSS_OFF       , "Continuous shooting","","");
    appendMyElt ("buttonsprop","coarse"     , OSS_OFF       , "Coarse focus","","");
    appendMyElt ("buttonsprop","fine"       , OSS_OFF       , "Fine focus","","");
    appendMyElt ("buttonsprop","abort"      , OSS_OFF       , "Abort","","");

    createMyProp("parms","Focus parameters",PT_NUM,"main","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","");
    appendMyElt ("parms","startpos"     , 350000        , "Start position","","");
    appendMyElt ("parms","overshoot"    , 200           , "Backlash overshoot","","");
    appendMyElt ("parms","incre"        , 500           , "Incrementation","","");
    appendMyElt ("parms","iterations"   , 10            , "Iterations","","");
    appendMyElt ("parms","exposure"     , 2             , "Exposure","","");

    createMyCateg("adv","Advanced parameters");

    createMyProp("indiprops","Indi properties",PT_TEXT,"adv","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","");
    appendMyElt ("indiprops","expp" , "CCD_EXPOSURE"           ,"Exposure property","","");
    appendMyElt ("indiprops","expe" , "CCD_EXPOSURE_VALUE"     ,"Exposure element","","");
    appendMyElt ("indiprops","focp" , "ABS_FOCUS_POSITION"     ,"Position property","","");
    appendMyElt ("indiprops","foce" , "FOCUS_ABSOLUTE_POSITION","Position element","","");


}

void MFocuser::test(void)
{
    qDebug() << "test";
}
void MFocuser::slotvalueChangedFromCtl(Prop prop)
{
    //qDebug() << "focuser" << el.type << el.module << el.name;
    if ((prop.typ==PT_SWITCH) && (prop.propname=="buttonsprop") )
    {
        prop.state=OPS_BUSY;
        setMyProp("buttonsprop",prop);
        if (prop.s["coarse"].s==OSS_ON) startCoarse();
        if (prop.s["fine"].s==OSS_ON) startFine();
        if (prop.s["framing"].s==OSS_ON) startFraming();
        if (prop.s["abort"].s==OSS_ON) abort();

    }

}

void MFocuser::startFraming(void)
{

}
void MFocuser::startFine(void)
{

}

void MFocuser::startCoarse(void)
{

    QString expp        = getMyTxt("indiprops","expp");
    QString expe        = getMyTxt("indiprops","expe");
    QString focp        = getMyTxt("indiprops","focp");
    QString foce        = getMyTxt("indiprops","foce");
    QString camera      = getMyTxt("devices","camerar");
    QString focuser     = getMyTxt("devices","focuser");
    double istartpos    = getMyNum("parms","startpos");
    double iovershoot   = getMyNum("parms","overshoot");
    double iincre       = getMyNum("parms","incre");
    double iiterations  = getMyNum("parms","iterations");
    double iexposure    = getMyNum("parms","exposure");

    setMyElt("valuesprop","besthfravg",99);

    addnewtask(TT_SEND_NUMBER,"sendblpos","Asking focuser to go to backlash position",false,focuser,focp,foce,istartpos-iovershoot,"",ISS_OFF);
    addnewtask(TT_WAIT_NUMBER,"waitblpos","Waiting focuser to go to backlash position",false,focuser,focp,foce,istartpos-iovershoot,"",ISS_OFF);
    for (int i=0;i<iiterations;i++) {
        addnewtask(TT_SEND_NUMBER,"sendfocpos","Asking focuser to move",false,focuser,focp,foce,istartpos+i*iincre,"",ISS_OFF);
        addnewtask(TT_WAIT_NUMBER,"waitfocpos","Waiting focuser",false,focuser,focp,foce,istartpos+i*iincre,"",ISS_OFF);
        addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram5","store pos/hfr",true ,   camera,expp,expe,istartpos+i*iincre,"",ISS_OFF);

    }
    addnewtask(TT_SPEC,"gobest","Asking focuser to go to best position",false,focuser,"","",0,"",ISS_OFF);



    //dumpTasks();
    executeTask(tasks.front());
}

void MFocuser::abort(void)
{
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
    QString camera      = getMyTxt("devices","camerar");
    QString focuser     = getMyTxt("devices","focuser");
    double iexposure    = getMyNum("parms","exposure");

    if (task.taskname=="fram5") {
        setMyElt("valuesprop","hfravg",image->HFRavg);
        setMyElt("valuesprop","starscount",image->stars.count());
        if (getMyNum("valuesprop","besthfravg")>image->HFRavg) {
            setMyElt("valuesprop","besthfravg",image->HFRavg);
            setMyElt("valuesprop","bestpos",task.value);
        }
        popnext();
        executeTask(tasks.front());
    }
    if (task.taskname=="gobest") {

        addnewtask(TT_SEND_NUMBER,"movebest","Asking focuser to go to best position",false,focuser,focp,foce,getMyNum("valuesprop","bestpos"),"",ISS_OFF);
        addnewtask(TT_WAIT_NUMBER,"waitbest","Waiting focuser to go to best position",false,focuser,focp,foce,getMyNum("valuesprop","bestpos"),"",ISS_OFF);
        addnewtask(TT_SEND_NUMBER,"check1","Check request",false,   camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"check2","Check waiting",false,   camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"check3","Check Analyse request",false,camera,expp,expe,iexposure,"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"check4","Check Waiting analyse",false,camera,expp,expe,iexposure,"",ISS_OFF);

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
