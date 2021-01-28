#include "mMainctl.h"


MMainctl::MMainctl(MyClient *cli,Properties *properties) : Module(cli,properties)
{
}
MMainctl::~MMainctl()
{
    //
}

void MMainctl::initProperties(void)
{
    modulename="mainctl";
    createMyModule("Main control");
    createMyCateg("main","Main");

    createMyProp("buttonsprop","Actions",PT_SWITCH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","");
    appendMyElt ("buttonsprop","connectindi"   , OSS_OFF       , "Connect indi server","","");
    appendMyElt ("buttonsprop","connectdevices", OSS_OFF       , "Connect indi devices","","");
    appendMyElt ("buttonsprop","loadconfs"     , OSS_OFF       , "Load devices configurations","","");
    /*props->createProp(modulename,"statusprop" ,"Status"  ,"","main","IP_RO","IPS_IDLE","");
    props->createText(modulename,"status","Status","statusprop","","main", "idle");

    props->createProp(modulename,"actions" ,"Actions"  ,"","main","IP_RW","IPS_IDLE","");
    props->createBool(modulename,"startfocus","Start focuser","actions","","main",false);
    props->createBool(modulename,"startnav","Start navigator","actions","","main",false);*/

}

void MMainctl::test(void)
{
    qDebug() << "test";
}

void MMainctl::slotvalueChangedFromCtl(Prop prop)
{
    //qDebug() << "mainctl" << el.type << el.module << el.name;
    if ((prop.typ==PT_SWITCH) && (prop.propname=="buttonsprop") )
    {
        prop.state=OPS_BUSY;
        setMyProp("buttonsprop",prop);

        if (prop.s["connectindi"].s==OSS_ON)
        {
            if (indiclient->connectIndi())
            {
                prop.state=OPS_OK;
            }  else {
                prop.state=OPS_ALERT;
            }
            prop.s["connectindi"].s=OSS_OFF;
            setMyProp("buttonsprop",prop);
        }

        if (prop.s["connectdevices"].s==OSS_ON)
        {
            indiclient->connectAllDevices();
            prop.state=OPS_OK;
            prop.s["connectdevices"].s=OSS_OFF;
            setMyProp("buttonsprop",prop);
        }

        if (prop.s["loadconfs"].s==OSS_ON)
        {
            indiclient->connectAllDevices();
            prop.state=OPS_OK;
            prop.s["loadconfs"].s=OSS_OFF;
            setMyProp("buttonsprop",prop);
        }
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
