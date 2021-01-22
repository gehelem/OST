#include "mFocuser.h"

MFocuser::MFocuser(MyClient *cli) : Module(cli)
{
}
MFocuser::~MFocuser()
{
    //
}

void MFocuser::initProperties(void)
{
    props.module="focuser";
    props.createCategory("main","Main control");
    props.createProp("status","Status","","main","IP_RO","IPS_IDLE","");
    props.createText("status","Status","idle","status");
    props.createProp("camera","Camera","","main","IP_RO","IPS_IDLE","");
    props.createText("camera","Camera","Guide Simulator","camera");

    props.createProp("values","Values","","main","IP_RW","IPS_IDLE","");
    props.createNum("exposure","Exposure",10,"values");
    props.createNum("hfravg","Average HFR",99,"values");
    props.createNum("starscount","Stars found",0,"values");

    props.createProp("buttons","Buttons","","main","IP_RO","IPS_IDLE","");
    props.createBTN("loop","Continuous shooting","buttons");
    props.createBTN("coarse","Start coarse focus","buttons");
    props.createBTN("fine","Start fine focus","buttons");
    props.createBTN("abort","Abortfocus","buttons");
}

void MFocuser::test(void)
{
    qDebug() << "test";
}
void MFocuser::slotvalueChangedFromCtl(elem el)
{
    //qDebug() << "focuser" << el.type << el.module << el.name;

    if ((el.type==ET_BTN) && (el.module==props.module) && (el.name=="loop") )
    {
        startFraming();
    }
    if ((el.type==ET_BTN) && (el.module==props.module) && (el.name=="abort") )
    {
        tasks =QQueue<Ttask>();
        props.setText("status","idle");
    }

}

void MFocuser::startFraming(void)
{

    QString prop = "CCD_EXPOSURE";
    QString elem = "CCD_EXPOSURE_VALUE";
    addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,props.getText("camera"),prop,elem,props.getNum("exposure"),"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,props.getText("camera"),prop,elem,props.getNum("exposure"),"",ISS_OFF);
    addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,props.getText("camera"),prop,elem,props.getNum("exposure"),"",ISS_OFF);
    addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,props.getText("camera"),prop,elem,props.getNum("exposure"),"",ISS_OFF);
    addnewtask(TT_SPEC       ,"fram0","Add next loop",true,props.getText("camera"),prop,elem,props.getNum("exposure"),"",ISS_OFF);
    //dumpTasks();
    executeTask(tasks.front());
}


void MFocuser::executeTaskSpec(Ttask task)
{

    if (task.taskname=="fram0") {
        props.setNum("hfravg",image->HFRavg);
        props.setNum("starscount",image->stars.count());
        //props.setNum("exposure",props.getNum("exposure")+10);

        addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,props.getText("camera"),task.propertyname,task.elementname,props.getNum("exposure"),"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,props.getText("camera"),task.propertyname,task.elementname,task.value,"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,props.getText("camera"),task.propertyname,task.elementname,task.value,"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,props.getText("camera"),task.propertyname,task.elementname,task.value,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram0","Add next loop",true,props.getText("camera"),task.propertyname,task.elementname,task.value,"",ISS_OFF);
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
