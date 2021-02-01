#include "mSequence.h"

MSequence::MSequence(MyClient *cli,Properties *properties) : Module(cli,properties)
{
}
MSequence::~MSequence()
{
    //
}

void MSequence::initProperties(void)
{
    modulename="sequence";
    createMyModule("Sequencer");
    createMyCateg("main","Main");

}

void MSequence::test(void)
{
    qDebug() << "test";
}

void MSequence::slotvalueChangedFromCtl(Prop prop)
{
    if (prop.modulename!=modulename) return;

    if ((prop.typ==PT_SWITCH) && (prop.propname=="buttonsprop") )
    {
        prop.state=OPS_BUSY;
        setMyProp("buttonsprop",prop);
        /*if (prop.s["something"].s==OSS_ON) startSomething();*/

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
