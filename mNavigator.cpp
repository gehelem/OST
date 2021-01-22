#include "mNavigator.h"

MNavigator::MNavigator(MyClient *cli) : Module(cli)
{
}
MNavigator::~MNavigator()
{
    //
}

void MNavigator::initProperties(void)
{
    Module::initProperties();
    props.module="navigator";
    /*props.createText("camera","Camera","CCD Simulator");
    props.createText("mount","Mount","Telescope Simulator");
    props.createText("target","Target","");
    props.createNum("exposure","Exposure",10);
    props.createNum("hfravg","Average HFR",99);
    props.createNum("starscount","Stars found",0);
    props.createBTN("search","Search target");
    props.createBTN("abort","Abort");*/

}

void MNavigator::test(void)
{
    qDebug() << "test";
}

void MNavigator::slotvalueChangedFromCtl(elem el)
{
    if ((el.type==ET_BTN) && (el.module==props.module) && (el.name=="xxx") )
    {
    }
}

void MNavigator::executeTaskSpec(Ttask task)
{

    if (task.taskname=="xxx") {
    }
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
