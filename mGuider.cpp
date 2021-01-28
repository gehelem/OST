#include "mGuider.h"

MGuider::MGuider(MyClient *cli,Properties *properties) : Module(cli,properties)
{
}
MGuider::~MGuider()
{
    //
}

void MGuider::initProperties(void)
{
    modulename="guider";
    createMyModule("Guider");
    createMyCateg("main","Main");

}

void MGuider::test(void)
{
    qDebug() << "test";
}

void MGuider::slotvalueChangedFromCtl(Prop prop)
{
    if ((prop.typ==PT_SWITCH) && (prop.propname=="buttonsprop") )
    {
        prop.state=OPS_BUSY;
        setMyProp("buttonsprop",prop);
        /*if (prop.s["something"].s==OSS_ON) startSomething();*/

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
