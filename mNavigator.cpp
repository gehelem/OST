#include "mNavigator.h"

MNavigator::MNavigator(MyClient *cli,Properties *properties) : Module(cli,properties)
{
}
MNavigator::~MNavigator()
{
    //
}

void MNavigator::initProperties(void)
{
    modulename="navigator";
    createMyModule("Navigator",40);
    createMyProp("messages","Messages",PT_MESSAGE,"","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",00);
    appendMyElt ("messages","messageselt","","Messages","","");

    createMyCateg("main","Main",10);
    createMyProp("statusprop","Status",PT_TEXT,"main","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("statusprop","status","","Status","","");

    createMyProp("devices","Devices",PT_TEXT,"main","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",99);
    appendMyElt ("devices","camera"  , "CCD Simulator"        ,"Camera","","");
    appendMyElt ("devices","mount"   , "Telescope Simulator"  ,"Mount","","");

    createMyProp("target","Target",PT_NUM,"main","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",50);
    appendMyElt ("target","ra"    , 1         , "RA (h)"  ,"","");
    appendMyElt ("target","dec"   , 85        , "DEC (°)" ,"","");

    createMyProp("buttonsprop","Actions",PT_SWITCH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",20);
    appendMyElt ("buttonsprop","goto"       , OSS_OFF       , "Goto","","");
    appendMyElt ("buttonsprop","abort"      , OSS_OFF       , "Abort","","");

    createMyCateg("adv","Advanced parameters",80);

    createMyProp("indiprops","Indi properties",PT_TEXT,"adv","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",10);
    appendMyElt ("indiprops","expp" , "CCD_EXPOSURE"           ,"Exposure property","","");
    appendMyElt ("indiprops","expe" , "CCD_EXPOSURE_VALUE"     ,"Exposure element","","");
    appendMyElt ("indiprops","radecp" , "EQUATORIAL_EOD_COORD" ,"RA/DEC property","","");
    appendMyElt ("indiprops","rae" , "RA"                      ,"RA element","","");
    appendMyElt ("indiprops","dece" , "DEC"                    ,"DEC property","","");


}

void MNavigator::test(void)
{
    qDebug() << "test";
}

void MNavigator::slotvalueChangedFromCtl(Prop prop)
{
    if (prop.modulename!=modulename) return;

    if ((prop.typ==PT_SWITCH) && (prop.propname=="buttonsprop") )
    {
        Prop myprop = getMyProp(prop.propname);
        myprop.state=OPS_BUSY;
        setMyProp(prop.propname,myprop);
        if (prop.s["goto"].s==OSS_ON) startGoto();
        if (prop.s["abort"].s==OSS_ON) abort();


    }
    if ((prop.typ==PT_NUM) && (prop.propname=="target"))
    {

        Prop myprop = getMyProp(prop.propname);
        foreach (ONumber onum, prop.n) {
            myprop.n[onum.name].value=onum.value;
        }
        setMyProp(prop.propname,myprop);
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

void MNavigator::startGoto(void)
{
    qDebug() << "goto request from ws" << tasks.size();

    QString expp        = getMyTxt("indiprops","expp");
    QString expe        = getMyTxt("indiprops","expe");
    QString camera      = getMyTxt("devices","camera");
    QString mount       = getMyTxt("devices","mount");
    double ra           = getMyNum("target","ra");
    double dec          = getMyNum("target","dec");

    addnewtask(TT_SPEC,"setradec","Sending RA/DEC",true,mount,"","",0,"",ISS_OFF);

    //dumpTasks();
    executeTask(tasks.front());

}
void MNavigator::executeTaskSpec(Ttask task)
{

    if (task.taskname=="setradec") {
        QString mount       = getMyTxt("devices","mount");
        QString radecp      = getMyTxt("indiprops","radecp");
        QString rae         = getMyTxt("indiprops","rae");
        QString dece        = getMyTxt("indiprops","dece");
        double ra           = getMyNum("target","ra");
        double dec          = getMyNum("target","dec");
        INDI::BaseDevice *dp;
        dp = indiclient->getDevice(mount.toStdString().c_str());
        if (dp== nullptr)
        {
            qDebug() << "Error - unable to find " << mount << " device. Aborting.";
            tasks =QQueue<Ttask>();
            return;
        }
        INumberVectorProperty *prop = nullptr;
        prop = dp->getNumber(radecp.toStdString().c_str());
        if (prop == nullptr)
        {
            qDebug() << "Error - unable to find " << mount << "/" << radecp << " property. Aborting.";
            tasks =QQueue<Ttask>();
            return;
        }
        for (int i=0;i<prop->nnp;i++) {
            if (strcmp(prop->np[i].name, rae.toStdString().c_str()) == 0) {
                prop->np[i].value=ra;
            }
        }
        for (int i=0;i<prop->nnp;i++) {
            if (strcmp(prop->np[i].name, dece.toStdString().c_str()) == 0) {
                prop->np[i].value=dec;
            }
        }
        indiclient->sendNewNumber(prop);
        popnext();
    }
}

void MNavigator::abort(void)
{
    qDebug() << "abort request from ws";
    tasks =QQueue<Ttask>();
    setMyElt("statusprop","status","idle");
    Prop prop = getMyProp("buttonsprop");
    prop.state=OPS_OK;
    setMyProp("buttonsprop",prop);
}
void MNavigator::executeTaskSpec(Ttask task,IBLOB *bp)
{
    Q_UNUSED(task);
    Q_UNUSED(bp);
}
void MNavigator::executeTaskSpec(Ttask task,INumberVectorProperty *nvp)
{
    Q_UNUSED(task);
    Q_UNUSED(nvp);
}
void MNavigator::executeTaskSpec(Ttask task,ISwitchVectorProperty *svp)
{
    Q_UNUSED(task);
    Q_UNUSED(svp);
}
void MNavigator::executeTaskSpec(Ttask task,ITextVectorProperty *tvp)
{
    Q_UNUSED(task);
    Q_UNUSED(tvp);
}
void MNavigator::executeTaskSpec(Ttask task,ILightVectorProperty *lvp)
{
    Q_UNUSED(task);
    Q_UNUSED(lvp);
}
