#include "mGuider.h"

MGuider::MGuider(MyClient *cli,OSTProperties *properties) : Module(cli,properties)
{
}
MGuider::~MGuider()
{
    //
}

void MGuider::initProperties(void)
{
    modulename="guider";
    props->createMod(modulename,"OST Guider");

    /*props.createText("camera","Camera","Guide Simulator");
    props.createNum("exposure","Exposure",1);
    props.createNum("hfravg","Average HFR",99);
    props.createNum("starscount","Stars found",0);
    props.createBTN("calibration","Start calibration");
    props.createBTN("guide","Start guiding");
    props.createBTN("abort","Abort");*/

}

void MGuider::test(void)
{
    qDebug() << "test";
}

void MGuider::slotvalueChangedFromCtl(elem el)
{
    if ((el.type==ET_BTN) && (el.modulename==modulename) && (el.elemname=="xxx") )
    {
    }
}


void MGuider::executeTaskSpec(Ttask task)
{

    if (task.taskname=="xxx") {
    }
}
void MGuider::executeTaskSpec(Ttask task,IBLOB *bp)
{
    Q_UNUSED(task);
    Q_UNUSED(bp);
}
void MGuider::executeTaskSpec(Ttask task,INumberVectorProperty *nvp)
{
    Q_UNUSED(task);
    Q_UNUSED(nvp);
}
void MGuider::executeTaskSpec(Ttask task,ISwitchVectorProperty *svp)
{
    Q_UNUSED(task);
    Q_UNUSED(svp);
}
void MGuider::executeTaskSpec(Ttask task,ITextVectorProperty *tvp)
{
    Q_UNUSED(task);
    Q_UNUSED(tvp);
}
void MGuider::executeTaskSpec(Ttask task,ILightVectorProperty *lvp)
{
    Q_UNUSED(task);
    Q_UNUSED(lvp);
}
