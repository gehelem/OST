#include "mMainctl.h"


MMainctl::MMainctl(MyClient *cli,OSTProperties *properties) : Module(cli,properties)
{
}
MMainctl::~MMainctl()
{
    //
}

void MMainctl::initProperties(void)
{
    modulename="mainctl";
    props->createCateg(modulename,"main","Main control of mainctl");

    props->createProp(modulename,"statusprop" ,"Status"  ,"","main","IP_RO","IPS_IDLE","");


    props->createText(modulename,"status","Status","statusprop","","main", "idle");
    /*props.createBTN("connectindi","Connect to indi server");
    props.createBTN("connectdevices","Connect indidevices");
    props.createBTN("loadconfs","Load indi devices configurations");*/

}

void MMainctl::test(void)
{
    qDebug() << "test";
}

void MMainctl::slotvalueChangedFromCtl(elem el)
{
    //qDebug() << "mainctl" << el.type << el.module << el.name;
    if ((el.type==ET_BTN) && (el.modulename==modulename) && (el.elemname=="connectindi") )
    {
        indiclient->connectIndi();
    }
    if ((el.type==ET_BTN) && (el.modulename==modulename) && (el.elemname=="connectdevices") )
    {
        indiclient->connectAllDevices();
    }
    if ((el.type==ET_BTN) && (el.modulename==modulename) && (el.elemname=="loadconfs") )
    {
        indiclient->loadDevicesConfs();
    }
}


void MMainctl::executeTaskSpec(Ttask task)
{

    if (task.taskname=="xxx") {
    }
}
void MMainctl::executeTaskSpec(Ttask task,IBLOB *bp)
{
    Q_UNUSED(task);
    Q_UNUSED(bp);
}
void MMainctl::executeTaskSpec(Ttask task,INumberVectorProperty *nvp)
{
    Q_UNUSED(task);
    Q_UNUSED(nvp);
}
void MMainctl::executeTaskSpec(Ttask task,ISwitchVectorProperty *svp)
{
    Q_UNUSED(task);
    Q_UNUSED(svp);
}
void MMainctl::executeTaskSpec(Ttask task,ITextVectorProperty *tvp)
{
    Q_UNUSED(task);
    Q_UNUSED(tvp);
}
void MMainctl::executeTaskSpec(Ttask task,ILightVectorProperty *lvp)
{
    Q_UNUSED(task);
    Q_UNUSED(lvp);
}
