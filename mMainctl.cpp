#include "mMainctl.h"


MMainctl::MMainctl(Properties *properties) : Module(properties)
{
}
MMainctl::~MMainctl()
{
    //
}

void MMainctl::initProperties(void)
{
    modulename="mainctl";
    createMyModule("Main control",10);
    createMyProp("messages","Messages",PT_MESSAGE,"","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",00);
    appendMyElt ("messages","messageselt","","Messages","","");

    createMyCateg("main","Main control",10);
    createMyCateg("democateg1","Démo catégorie 1",20);
    createMyCateg("democateg2","Démo catégorie 2",30);
    createMyCateg("democateg3","Démo catégorie 3",40);
    createMyGroup("democateg1","demogroup1C1","Démo group 1C1",10);
    createMyGroup("democateg2","demogroup1C2","Démo group 1C2",20);

    createMyProp("statusprop","Status",PT_TEXT,"main","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("statusprop","status","","Status","","");

    createMyProp("buttonsprop","Actions",PT_SWITCH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",20);
    appendMyElt ("buttonsprop","connectindi"   , OSS_OFF       , "Connect indi server","","");
    appendMyElt ("buttonsprop","connectdevices", OSS_OFF       , "Connect indi devices","","");
    appendMyElt ("buttonsprop","loadconfs"     , OSS_OFF       , "Load devices configurations","","");



    createMyProp("prop1","Démo prop1 level 0",PT_TEXT,"","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("prop1","det11", OSS_OFF       , "Détail 11","","");
    appendMyElt ("prop1","det12", OSS_OFF       , "Détail 12","","");
    createMyProp("prop2","Démo prop2 level 1",PT_TEXT,"democateg1","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("prop2","det21", OSS_OFF       , "Détail 21","","");
    appendMyElt ("prop2","det22", OSS_OFF       , "Détail 22","","");
    createMyProp("prop3","Démo prop3 level 2",PT_TEXT,"democateg1","demogroup1C1", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("prop3","det31", OSS_OFF       , "Détail 31","","");
    appendMyElt ("prop3","det32", OSS_OFF       , "Détail 32","","");
    createMyProp("prop4","Démo prop4 level 2",PT_TEXT,"democateg1","demogroup1C1", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",20);
    appendMyElt ("prop4","det41", OSS_OFF       , "Détail 41","","");
    appendMyElt ("prop4","det42", OSS_OFF       , "Détail 42","","");
    createMyProp("prop5","Démo prop5 level 2",PT_TEXT,"democateg2","demogroup1C2", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("prop5","det51", OSS_OFF       , "Détail 51","","");
    appendMyElt ("prop5","det52", OSS_OFF       , "Détail 52","","");
    createMyProp("prop6","Démo prop6 level 1",PT_TEXT,"democateg2","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("prop6","det61", OSS_OFF       , "Détail 61","","");
    appendMyElt ("prop6","det62", OSS_OFF       , "Détail 62","","");
    appendMyElt ("prop6","det63", OSS_OFF       , "Détail 66","","");
    appendMyElt ("prop6","det64", OSS_OFF       , "Détail 64","","");
    appendMyElt ("prop6","det65", OSS_OFF       , "Détail 65","","");
    appendMyElt ("prop6","det66", OSS_OFF       , "Détail 66","","");
    createMyProp("prop7","Démo prop7 level 1",PT_SWITCH,"democateg2","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",20);
    appendMyElt ("prop7","det71", OSS_ON        , "SW1","","");
    appendMyElt ("prop7","det71", OSS_OFF       , "SW2","","");
    appendMyElt ("prop7","det71", OSS_OFF       , "SW3","","");
    appendMyElt ("prop8","det71", OSS_OFF       , "SW4","","");
    createMyProp("prop7","Démo prop8 level 1",PT_LIGHT,"democateg2","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",30);
    appendMyElt ("prop7","det71", OSS_ON        , "LIGHT1","","");
    appendMyElt ("prop7","det71", OSS_OFF       , "LIGHT2","","");


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
    //qDebug() << "mainctl" << prop.propname << "received";
    if (prop.modulename!=modulename) return;

    if ((prop.typ==PT_SWITCH) && (prop.propname=="buttonsprop"))
    {
        Prop myprop = getMyProp(prop.propname);
        prop.state=OPS_BUSY;
        setMyProp("buttonsprop",myprop);

        if (prop.s["connectindi"].s==OSS_ON)
        {
            if (indiclient->connectIndi())
            {
                myprop.state=OPS_OK;
                setMyElt("statusprop","status","Indi server connected");
            }  else {
                setMyElt("statusprop","status","Could not connect to Indi server");
                myprop.state=OPS_ALERT;
            }
            myprop.s["connectindi"].s=OSS_OFF;
            setMyProp("buttonsprop",myprop);
        }

        if (prop.s["connectdevices"].s==OSS_ON)
        {
            indiclient->connectAllDevices();
            myprop.state=OPS_OK;
            myprop.s["connectdevices"].s=OSS_OFF;
            setMyProp("buttonsprop",myprop);
            setMyElt("statusprop","status","Connect devices");
        }

        if (prop.s["loadconfs"].s==OSS_ON)
        {
            indiclient->connectAllDevices();
            myprop.state=OPS_OK;
            myprop.s["loadconfs"].s=OSS_OFF;
            setMyProp("buttonsprop",myprop);
            setMyElt("statusprop","status","Load devices configuration");

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
