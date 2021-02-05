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
    createMyCateg("main","Main",10);

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
        //prop.state=OPS_BUSY;
        //setMyProp("buttonsprop",prop);
        /*if (prop.s["something"].s==OSS_ON) startSomething();*/

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
