#include "mMainctl.h"


MMainctl::MMainctl(MyClient *cli) : Module(cli)
{
}
MMainctl::~MMainctl()
{
    //
}

void MMainctl::initProperties(void)
{
    Module::initProperties();
    props.module="mainctl";
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
    if ((el.type==ET_BTN) && (el.module==props.module) && (el.name=="connectindi") )
    {
        indiclient->connectIndi();
    }
    if ((el.type==ET_BTN) && (el.module==props.module) && (el.name=="connectdevices") )
    {
        indiclient->connectAllDevices();
    }
    if ((el.type==ET_BTN) && (el.module==props.module) && (el.name=="loadconfs") )
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
