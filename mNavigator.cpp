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

    createMyProp("results","Results",PT_NUM,"main","", OP_RO,OSR_NOFMANY,0,OPS_IDLE,"","",55);
    appendMyElt ("results","ra"    , 0         , "Solved RA (h)"  ,"","");
    appendMyElt ("results","dec"   , 0        , "Solved DEC (°)" ,"","");

    createMyProp("buttonsprop","Actions",PT_SWITCH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",20);
    appendMyElt ("buttonsprop","goto"       , OSS_OFF       , "Goto","","");
    appendMyElt ("buttonsprop","abort"      , OSS_OFF       , "Abort","","");

    createMyProp("parms","Parameters",PT_NUM,"main","", OP_RW,OSR_NOFMANY,0,OPS_IDLE,"","",30);
    appendMyElt ("parms","exposure"     , 10               , "Exposure","","");
    appendMyElt ("parms","tolerance"     , 0.05               , "Tolérance","","");

    createMyProp("image","Image",PT_IMAGE,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",50);
    appendMyElt ("image","imagesolv"  , IM_FULL       , "Image result","","","imagesolv.jpeg","/var/www/html");

    createMyProp("solver","Solver plot",PT_GRAPH,"main","", OP_RW,OSR_ATMOST1,0,OPS_IDLE,"","",60);
    OGraph gra;
    gra.name="solverdrift" ;
    gra.label="Solver drift";
    gra.gtype="2DPLOT";
    gra.V0label="RA";
    gra.V1label="DEC";
    appendMyElt ("solver","solverdrift"  , gra);


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
    QString radecp      = getMyTxt("indiprops","radecp");
    double ra           = getMyNum("target","ra");
    double dec          = getMyNum("target","dec");
    double iexposure    = getMyNum("parms","exposure");
    double tol          = getMyNum("parms","tolerance");
    resetMyGra("solver","solver");
    OGraphValue v;
    v.v0=ra;
    v.v1=dec;
    appendMyGra("solver","solver",v);

    addnewtask(TT_SPEC,"setradec","Sending RA/DEC",true,mount,"","",0,"",ISS_OFF);
    addnewtask(TT_WAIT_PROP,"waitradec","Waiting slew end",true,mount,radecp,"",0,"",ISS_OFF);
    addnewtaskFrameReset      ("fram00","Reset frame",false,camera);
    addnewtaskWaitFrameResetOk("fram01","Wait frame reset",false,camera);
    addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_ANALYSE_SOLVE,"fram3","Analyse request",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_WAIT_SOLVE,"fram4","Waiting analyse",false,camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_SPEC       ,"fram5","Save image",true,   camera,expp,expe,iexposure,"",ISS_OFF);
    addnewtask(TT_SPEC       ,"fram6","Check tolerance",true,   camera,expp,expe,tol,"",ISS_OFF);


    //dumpTasks();
    setMyElt("messages","messageselt",tasks.front().tasklabel);
    executeTask(tasks.front());

}
bool MNavigator::sendRaDec(double ra, double dec)
{
    QString mount       = getMyTxt("devices","mount");
    QString radecp      = getMyTxt("indiprops","radecp");
    QString rae         = getMyTxt("indiprops","rae");
    QString dece        = getMyTxt("indiprops","dece");

    INDI::BaseDevice *dp;
    dp = indiclient->getDevice(mount.toStdString().c_str());
    if (dp== nullptr)
    {
        qDebug() << "Error - unable to find " << mount << " device. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }
    INumberVectorProperty *prop = nullptr;
    prop = dp->getNumber(radecp.toStdString().c_str());
    if (prop == nullptr)
    {
        qDebug() << "Error - unable to find " << mount << "/" << radecp << " property. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }
    for (int i=0;i<prop->nnp;i++) {
        if (strcmp(prop->np[i].name, rae.toStdString().c_str()) == 0) {
            prop->np[i].value=ra;
        }
        if (strcmp(prop->np[i].name, dece.toStdString().c_str()) == 0) {
            prop->np[i].value=dec;
        }
    }
    indiclient->sendNewNumber(prop);
    return true;

}
void MNavigator::executeTaskSpec(Ttask task)
{

    if (task.taskname=="setradec") {
        setMyElt("messages","messageselt",tasks.front().tasklabel);
        double ra           = getMyNum("target","ra");
        double dec          = getMyNum("target","dec");
        if (sendRaDec(ra,dec)) {
            setMyElt("messages","messageselt","Waiting slew end");
            popnext();
        }

    }

    if (task.taskname=="fram5") {
        setMyElt("messages","messageselt",tasks.front().tasklabel);
        image->saveStretchedToJpeg(getMyImg("image","imagesolv").f+"/"+getMyImg("image","imagesolv").url,100);
        setMyElt("image","imagesolv",getMyImg("image","imagesolv").url,getMyImg("image","imagesolv").f);
        setMyElt("results","ra",24*image->stellarSolver->getSolution().ra/360);
        setMyElt("results","dec",image->stellarSolver->getSolution().dec);
        OGraphValue v;
        v.v0=24*image->stellarSolver->getSolution().ra/360;
        v.v1=image->stellarSolver->getSolution().dec;
        appendMyGra("solver","solverdrift",v);
        popnext();
        executeTask(tasks.front());
    }
    if (task.taskname=="fram6") {
        setMyElt("messages","messageselt",tasks.front().tasklabel);

        double ra           = getMyNum("target","ra");
        double dec          = getMyNum("target","dec");
        double sra           = getMyNum("results","ra");
        double sdec          = getMyNum("results","dec");
        double tol          = getMyNum("parms","tolerance");

        if ( (fabs(ra-sra)>tol)||(fabs(dec-sdec)>tol) ) {
            setMyElt("messages","messageselt","Too far, moving again");

            QString mount       = getMyTxt("devices","mount");
            QString radecp      = getMyTxt("indiprops","radecp");
            QString rae         = getMyTxt("indiprops","rae");
            QString dece        = getMyTxt("indiprops","dece");
            QString expp        = getMyTxt("indiprops","expp");
            QString expe        = getMyTxt("indiprops","expe");
            QString camera      = getMyTxt("devices","camera");
            double iexposure    = getMyNum("parms","exposure");


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
            for (int i=0;i<prop->nnp;i++) { // adjust diff = target - solved
                if (strcmp(prop->np[i].name, rae.toStdString().c_str()) == 0) {
                    prop->np[i].value=prop->np[i].value+ra-sra;
                }
                if (strcmp(prop->np[i].name, dece.toStdString().c_str()) == 0) {
                    prop->np[i].value=prop->np[i].value+dec-sdec;
                }
            }
            indiclient->sendNewNumber(prop);
            addnewtask(TT_WAIT_PROP,"waitradec","Waiting slew end",true,mount,radecp,"",0,"",ISS_OFF);
            addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,camera,expp,expe,iexposure,"",ISS_OFF);
            addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,camera,expp,expe,iexposure,"",ISS_OFF);
            addnewtask(TT_ANALYSE_SOLVE,"fram3","Analyse request",false,camera,expp,expe,iexposure,"",ISS_OFF);
            addnewtask(TT_WAIT_SOLVE,"fram4","Waiting analyse",false,camera,expp,expe,iexposure,"",ISS_OFF);
            addnewtask(TT_SPEC       ,"fram5","Save image",true,   camera,expp,expe,iexposure,"",ISS_OFF);
            addnewtask(TT_SPEC       ,"fram6","Check tolerance",true,   camera,expp,expe,tol,"",ISS_OFF);
            popnext();
            executeTask(tasks.front());

        } else {
            setMyElt("messages","messageselt","Target reached");
            popnext();
        }

        //executeTask(tasks.front());
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
    if (task.taskname=="waitradec") {

        if (   (strcmp(tasks.front().devicename.toStdString().c_str(),nvp->device)==0)
            && (strcmp(tasks.front().propertyname.toStdString().c_str(),nvp->name)==0)
            && (nvp->s==IPS_OK) ) {
            setMyElt("messages","messageselt","Slew finished");
            popnext();
            executeTask(tasks.front());
        }

    }
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
