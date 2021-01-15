#include <basedevice.h>
#include "client.h"
#include "image.h"
#include "job.h"
#include "jFocuser.h"

JFocuser::JFocuser(MyClient *cli) : Job(cli)
{
    Job::setProperties();
    setProperties();
}
JFocuser::~JFocuser()
{
    //
}
void JFocuser::setProperties(void)
{
    props.setProperty("Focus","this is mine");

}
void JFocuser::startFraming(void)
{
    addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,"CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
    addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,"CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
    addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,"CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
    addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,"CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
    addnewtask(TT_SPEC       ,"fram0","loopng",true,"","","",0,"",ISS_OFF);
    executeTask(tasks.front());
}


void JFocuser::executeTaskSpec(Ttask task)
{
    Q_UNUSED(task);
    if (strcmp(task.taskname.toStdString().c_str(),"fram0") == 0) {
        addnewtask(TT_SEND_NUMBER,"fram1","Exp. request",false,"CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
        addnewtask(TT_WAIT_BLOB  ,"fram2","Exp. waiting",false,"CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
        addnewtask(TT_ANALYSE_SEP,"fram3","Analyse request",false,"CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
        addnewtask(TT_WAIT_SEP   ,"fram4","Waiting analyse",false,"CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
        addnewtask(TT_SPEC       ,"fram0","loopng",true,"","","",0,"",ISS_OFF);
        popnext();
        executeTask(tasks.front());
    }
}
void JFocuser::executeTaskSpec(Ttask task,IBLOB *bp)
{
    Q_UNUSED(task);

}
void JFocuser::executeTaskSpec(Ttask task,INumberVectorProperty *nvp)
{
    Q_UNUSED(task);
    Q_UNUSED(nvp);
}
void JFocuser::executeTaskSpec(Ttask task,ISwitchVectorProperty *svp)
{
    Q_UNUSED(task);
    Q_UNUSED(svp);
}
void JFocuser::executeTaskSpec(Ttask task,ITextVectorProperty *tvp)
{
    Q_UNUSED(task);
    Q_UNUSED(tvp);
}
void JFocuser::executeTaskSpec(Ttask task,ILightVectorProperty *lvp)
{
    Q_UNUSED(task);
    Q_UNUSED(lvp);
}
