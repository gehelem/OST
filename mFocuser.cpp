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
    qDebug() << "before";
    props->createCateg(modulename,"main","Main control");
    qDebug() << "before";

    props->createProp(modulename,"statusprop" ,"Status"  ,"","main","IP_RO","IPS_IDLE","");
    props->createProp(modulename,"cameraprop" ,"Cameras" ,"","main","IP_RO","IPS_IDLE","");
    props->createProp(modulename,"valuesprop" ,"Values"  ,"","main","IP_RW","IPS_IDLE","");
    props->createProp(modulename,"buttonsprop","Buttons" ,"","main","IP_RO","IPS_IDLE","");


    props->createText(modulename,"status","Status","statusprop","","main", "idle");
    props->createText(modulename,"camera","Camera","cameraprop","","main", "Guide Simulator");

    props->createNum(modulename,"exposure"  ,"Exposure"   ,"valuesprop","","main",10);
    props->createNum(modulename,"hfravg"    ,"Average HFR","valuesprop","","main",99);
    props->createNum(modulename,"starscount","Stars found","valuesprop","","main",0);

    props->createBTN(modulename,"loop"     ,"Continuous shooting","buttonsprop","","main");
    props->createBTN(modulename,"coarse"   ,"Start coarse focus","buttonsprop","","main");
    props->createBTN(modulename,"fine"     ,"Start fine focus","buttonsprop","","main");
    props->createBTN(modulename,"abort"    ,"Abortfocus","buttonsprop","","main");
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
    addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
    addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
    addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
    addnewtask(TT_SPEC       ,"fram0","Add next loop",true,props->getText(modulename,"camera"),prop,elem,props->getNum(modulename,"exposure"),"",ISS_OFF);
    //dumpTasks();
    executeTask(tasks.front());
}


void MFocuser::executeTaskSpec(Ttask task)
{

    if (task.taskname=="fram0") {
        props->setNum(modulename,"hfravg",image->HFRavg);
        props->setNum(modulename,"starscount",image->stars.count());
        //props.setNum("exposure",props.getNum("exposure")+10);

        addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,props->getText(modulename,"camera"),task.propertyname,task.elementname,props->getNum(modulename,"exposure"),"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,props->getText(modulename,"camera"),task.propertyname,task.elementname,task.value,"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,props->getText(modulename,"camera"),task.propertyname,task.elementname,task.value,"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,props->getText(modulename,"camera"),task.propertyname,task.elementname,task.value,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram0","Add next loop",true,props->getText(modulename,"camera"),task.propertyname,task.elementname,task.value,"",ISS_OFF);
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
