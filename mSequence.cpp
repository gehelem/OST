#include "mSequence.h"

MSequence::MSequence(MyClient *cli,OSTProperties *properties) : Module(cli,properties)
{
}
MSequence::~MSequence()
{
    //
}

void MSequence::initProperties(void)
{
    modulename="sequence";
    /*props.createText("camera","Camera","Guide Simulator");
    props.createText("camera2","Camera 2","Guide Simulator");
    props.createNum("exposure","Exposure",10);
    props.createNum("hfravg","Average HFR",99);
    props.createNum("starscount","Stars found",0);*/

}

void MSequence::test(void)
{
    qDebug() << "test";
}

void MSequence::slotvalueChangedFromCtl(elem el)
{
    if ((el.type==ET_BTN) && (el.modulename==modulename) && (el.elemname=="xxx") )
    {
    }
}


void MSequence::executeTaskSpec(Ttask task)
{

    if (task.taskname=="xxx") {
    }
}
void MSequence::executeTaskSpec(Ttask task,IBLOB *bp)
{
    Q_UNUSED(task);
    Q_UNUSED(bp);
}
void MSequence::executeTaskSpec(Ttask task,INumberVectorProperty *nvp)
{
    Q_UNUSED(task);
    Q_UNUSED(nvp);
}
void MSequence::executeTaskSpec(Ttask task,ISwitchVectorProperty *svp)
{
    Q_UNUSED(task);
    Q_UNUSED(svp);
}
void MSequence::executeTaskSpec(Ttask task,ITextVectorProperty *tvp)
{
    Q_UNUSED(task);
    Q_UNUSED(tvp);
}
void MSequence::executeTaskSpec(Ttask task,ILightVectorProperty *lvp)
{
    Q_UNUSED(task);
    Q_UNUSED(lvp);
}
