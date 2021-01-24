#include "mFocuser.h"

MFocuser::MFocuser(MyClient *cli,OSTProperties *properties) : Module(cli,properties)
{
}
MFocuser::~MFocuser()
{
    //
}

void MFocuser::initProperties(void)
{
    modulename="focuser";
    props->createMod(modulename,"OST focuser");
    props->createCateg(modulename,"main","Main control");

    props->createProp(modulename,"statusprop" ,"Status"  ,"","main","IP_RO","IPS_IDLE","");
    props->createText(modulename,"status","Status","statusprop","","main", "idle");

    props->createProp(modulename,"devices" ,"Devices" ,"","main","IP_RW","IPS_IDLE","");
    props->createText(modulename,"camera" ,"Camera" ,"devices","","main", "CCD Simulator");
    props->createText(modulename,"focuser","Focuser","devices","","main", "Focuser Simulator");

    props->createProp(modulename,"valuesprop","Values"  ,"","main","IP_RO","IPS_IDLE","");
    props->createNum(modulename,"hfravg"     ,"Average HFR","valuesprop","","main",99);
    props->createNum(modulename,"starscount" ,"Stars found","valuesprop","","main",0);
    props->createNum(modulename,"focuspos"   ,"Focuser position","valuesprop","","main",0);
    props->createNum(modulename,"bestpos"    ,"Best position","valuesprop","","main",0);
    props->createNum(modulename,"besthfravg" ,"Best HFR","valuesprop","","main",99);

    props->createProp(modulename,"buttonsprop","Buttons" ,"","main","IP_RO","IPS_IDLE","");
    props->createBTN(modulename,"loop"     ,"Continuous shooting","buttonsprop","","main");
    props->createBTN(modulename,"coarse"   ,"Start coarse focus","buttonsprop","","main");
    props->createBTN(modulename,"fine"     ,"Start fine focus","buttonsprop","","main");
    props->createBTN(modulename,"abort"    ,"Abortfocus","buttonsprop","","main");

    props->createProp(modulename,"focusprop" ,"Focuser positions"  ,"","main","IP_RW","IPS_IDLE","");
    props->createNum(modulename,"startpos"  ,"Start position"    ,"focusprop","","main",35000);
    props->createNum(modulename,"overshoot" ,"Backlash overshoot","focusprop","","main",200);
    props->createNum(modulename,"incre"     ,"Incrementation"    ,"focusprop","","main",500);
    props->createNum(modulename,"iterations","Iterations"        ,"focusprop","","main",10);
    props->createNum(modulename,"exposure"  ,"Exposure"          ,"focusprop","","main",10);

}

void MFocuser::test(void)
{
    qDebug() << "test";
}
void MFocuser::slotvalueChangedFromCtl(elem el)
{
    //qDebug() << "focuser" << el.type << el.module << el.name;
    if ((el.type==ET_BTN) && (el.modulename==modulename) && (el.elemname=="loop") )
    {
        startFraming();
    }
    if ((el.type==ET_BTN) && (el.modulename==modulename) && (el.elemname=="abort") )
    {
        tasks =QQueue<Ttask>();
        props->setText(modulename,"status","idle");
    }

}

void MFocuser::startFraming(void)
{

    QString prop = "CCD_EXPOSURE";
    QString elem = "CCD_EXPOSURE_VALUE";

    /* 7 lines below to be removed, just to test front end */
    props->setNum(modulename,"startpos"  ,35000);
    props->setNum(modulename,"overshoot" ,200);
    props->setNum(modulename,"incre"     ,500);
    props->setNum(modulename,"iterations",10);
    props->setNum(modulename,"exposure"  ,10);
    props->setText(modulename,"camera" ,"CCD Simulator");
    props->setText(modulename,"focuser","Focuser Simulator");


    props->setNum(modulename,"besthfravg",99);

    addnewtask(TT_SEND_NUMBER,"sendblpos","Asking focuser to go to backlash position",false,props->getText(modulename,"focuser"),\
               "ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION",props->getNum(modulename,"startpos")-props->getNum(modulename,"overshoot"),"",ISS_OFF);
    addnewtask(TT_WAIT_NUMBER,"waitblpos","Waiting focuser to go to backlash position",false,props->getText(modulename,"focuser"),\
               "ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION",props->getNum(modulename,"startpos")-props->getNum(modulename,"overshoot"),"",ISS_OFF);
    for (int i=0;i<props->getNum(modulename,"iterations");i++) {
        addnewtask(TT_SEND_NUMBER,"sendfocpos","Asking focuser to move",false,props->getText(modulename,"focuser"),\
                   "ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION",props->getNum(modulename,"startpos")+i*props->getNum(modulename,"incre"),"",ISS_OFF);
        addnewtask(TT_WAIT_NUMBER,"waitfocpos","Waiting focuser",false,props->getText(modulename,"focuser"),\
                   "ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION",props->getNum(modulename,"startpos")+i*props->getNum(modulename,"incre"),"",ISS_OFF);
        addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,   props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,   props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram5","store pos/hfr",true ,   props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"startpos")+i*props->getNum(modulename,"incre"),"",ISS_OFF);

    }
    addnewtask(TT_SPEC,"gobest","Asking focuser to go to best position",false,props->getText(modulename,"focuser"),"","",0,"",ISS_OFF);



    //dumpTasks();
    executeTask(tasks.front());
}


void MFocuser::executeTaskSpec(Ttask task)
{
    QString prop = "CCD_EXPOSURE";
    QString elem = "CCD_EXPOSURE_VALUE";

    if (task.taskname=="fram5") {
        props->setNum(modulename,"hfravg",image->HFRavg);
        props->setNum(modulename,"starscount",image->stars.count());
        if (props->getNum(modulename,"besthfravg")>image->HFRavg) {
            props->setNum(modulename,"besthfravg",image->HFRavg);
            props->setNum(modulename,"bestpos",task.value);
        }
        popnext();
        executeTask(tasks.front());
    }
    if (task.taskname=="gobest") {

        addnewtask(TT_SEND_NUMBER,"movebest","Asking focuser to go to best position",false,props->getText(modulename,"focuser"),\
                   "ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION",props->getNum(modulename,"bestpos"),"",ISS_OFF);
        addnewtask(TT_WAIT_NUMBER,"waitbest","Waiting focuser to go to best position",false,props->getText(modulename,"focuser"),\
                   "ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION",props->getNum(modulename,"bestpos"),"",ISS_OFF);
        addnewtask(TT_SEND_NUMBER,"check1","Check request",false,   props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"check2","Check waiting",false,   props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"check3","Check Analyse request",false,props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"check4","Check Waiting analyse",false,props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);

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
