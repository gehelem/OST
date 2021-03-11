#include <QtCore>
#include <QtConcurrent>
#include <QStateMachine>
#include <basedevice.h>
#include "focus.h"

FocusModule::FocusModule()
{
    qRegisterMetaType<MapStringStr>("MapStringStr");
    _modulename = "focus";
    _devices["camera"]="";
    _devices["focuser"]="";
    indiclient=IndiCLient::getInstance();
    connect(indiclient,&IndiCLient::SigServerConnected,this,&FocusModule::OnIndiServerConnected);
    connect(indiclient,&IndiCLient::SigServerDisconnected,this,&FocusModule::OnIndiServerDisconnected);
//    connect(indiclient,&IndiCLient::SigNewProperty,this,&FocusModule::OnIndiNewProperty);
    connect(indiclient,&IndiCLient::SigRemoveProperty,this,&FocusModule::OnIndiRemoveProperty);
    connect(indiclient,&IndiCLient::SigNewText,this,&FocusModule::OnIndiNewText);
    connect(indiclient,&IndiCLient::SigNewSwitch,this,&FocusModule::OnIndiNewSwitch);
    connect(indiclient,&IndiCLient::SigNewLight,this,&FocusModule::OnIndiNewLight);
//    connect(indiclient,&IndiCLient::SigNewBLOB,this,&FocusModule::OnIndiNewBLOB);
//    connect(indiclient,&IndiCLient::SigNewNumber,this,&FocusModule::OnIndiNewNumber);
//    connect(indiclient, &IndiCLient::messageReceived, this, &FocusModule::OnIndiNewMessage);
}

FocusModule::~FocusModule()
{

}

void FocusModule::test0(QString txt)
{
    if (txt=="z")  emit abort();
    if (txt=="c")  connectIndi();
    if (txt=="a")  connectAllDevices();
    if (txt=="l")  loadDevicesConfs();
    if (txt=="d")  disconnectAllDevices();
    if (txt=="x")  disconnectIndi();
    if (txt=="f")  startCoarse();

}
void FocusModule::OnIndiNewNumber(INumberVectorProperty *nvp)
{
    if (
            (QString(nvp->device) == _devices["camera"] )
        &&  (nvp->s==IPS_ALERT)
       )
    {
        sendMessage("cameraAlert");
        emit cameraAlert();
    }
}

void FocusModule::OnIndiNewBLOB(IBLOB *bp)
{
    if (
            (QString(bp->bvp->device) == _devices["camera"])
       )
    {
        sendMessage("blobReceived " + QString(bp->bvp->device) + " - " + QString(bp->name) + " - " + QString::number(bp->bloblen));
        //machine->submitEvent(QLatin1String("expdone"));

        //emit expdone(bp);
        double vv;
        vv = 3.14116;
        emit expdone();
    }

}

void FocusModule::OnIndiNewSwitch(ISwitchVectorProperty *svp)
{
    if (
            (QString(svp->device) == _devices["camera"])
//        &&  (QString(svp->name)   =="CCD_FRAME_RESET")
        &&  (svp->s==IPS_ALERT)
       )
    {
        sendMessage("cameraAlert");
        emit cameraAlert();
    }

    if (
            (QString(svp->device) == _devices["focuser"])
        &&  (QString(svp->name)   =="ABS_FOCUS_POSITION")
        &&  (svp->s==IPS_OK)
       )
    {
        sendMessage("focuserReachedPosition");
        emit focuserReachedPosition();
    }
    if (
            (QString(svp->device) == _devices["camera"])
        &&  (QString(svp->name)   =="CCD_FRAME_RESET")
        &&  (svp->s==IPS_OK)
       )
    {
        sendMessage("frameResetDone");
        if (_machine->isRunning()) emit frameresetdone();
    }


}

void FocusModule::SMAbort()
{

    _machine->stop();
    sendMessage("machine stopped");
}

void FocusModule::startCoarse()
{

    _machine = new QStateMachine();

    /* states definitions */
    auto *loop        = new QState();
    QState *framereset  = new QState(loop);
    QState *exprequest  = new QState(loop);
    QState *expwait     = new QState(loop);
    QState *loadblob    = new QState(loop);
    QFinalState *abort  = new QFinalState();
    loop->setInitialState(framereset);

    /* actions to take when entering into state */
    connect(framereset, &QState::entered, this, &FocusModule::SMFrameReset);
    connect(exprequest, &QState::entered, this, &FocusModule::SMExpRequest);
    connect(loadblob,   &QState::entered, this, &FocusModule::SMLoadblob);
    connect(abort,      &QState::entered, this, &FocusModule::SMAbort);

    /* mapping signals to state transitions */
    framereset->addTransition(this, &FocusModule::frameresetdone, exprequest);
    exprequest->addTransition(this, &FocusModule::exprequestdone, expwait);
    expwait->   addTransition(this, &FocusModule::expdone, loadblob);
    loadblob->  addTransition(this, &FocusModule::blobloaded, exprequest);

    loop->addTransition(this, &FocusModule::abort, abort);

    _machine->addState(loop);
    _machine->addState(abort);

    _machine->setInitialState(loop);
    _machine->start();
    sendMessage("machine started");

}
void FocusModule::SMFrameReset()
{
    sendMessage("SMFrameReset");
    if (!frameReset(_devices["camera"]))
    {
            emit abort();
            return;
    }
    emit frameresetdone();
}
void FocusModule::SMExpRequest()
{
    sendMessage("SMExpRequest");
    if (!sendNewNumber(_devices["camera"],"CCD_EXPOSURE","CCD_EXPOSURE_VALUE",2))
    {
            emit abort();
            return;
    }
    emit exprequestdone();
}
void FocusModule::SMAlert()
{
    sendMessage("SMAlert");
    emit abort();
}
//void FocusModule::SMLoadblob(IBLOB *bp)
void FocusModule::SMLoadblob()
{
    double v = 12;
    //v=*val;
    sendMessage("SMLoadblob " + QString::number(v));
    //image->LoadFromBlob(bp);
    //machine.submitEvent("blobloaded");
    emit blobloaded();

}

