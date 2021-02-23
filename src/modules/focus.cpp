#include <QtCore>
#include <QtConcurrent>
#include <QScxmlStateMachine>
#include <QStateMachine>
#include <basedevice.h>
#include "focus.h"

FocusModule::FocusModule()
{
    _modulename = "focus";
    indiclient=IndiCLient::getInstance();
    connect(indiclient,&IndiCLient::SigServerConnected,this,&FocusModule::IndiServerConnected);
    connect(indiclient,&IndiCLient::SigServerDisconnected,this,&FocusModule::IndiServerDisconnected);
    connect(indiclient,&IndiCLient::SigNewDevice,this,&FocusModule::IndiNewDevice);
    connect(indiclient,&IndiCLient::SigRemoveDevice,this,&FocusModule::IndiRemoveDevice);
    connect(indiclient,&IndiCLient::SigNewProperty,this,&FocusModule::IndiNewProperty);
    connect(indiclient,&IndiCLient::SigRemoveProperty,this,&FocusModule::IndiRemoveProperty);
    connect(indiclient,&IndiCLient::SigNewText,this,&FocusModule::IndiNewText);
    connect(indiclient,&IndiCLient::SigNewSwitch,this,&FocusModule::IndiNewSwitch);
    connect(indiclient,&IndiCLient::SigNewLight,this,&FocusModule::IndiNewLight);
    connect(indiclient,&IndiCLient::SigNewBLOB,this,&FocusModule::IndiNewBLOB);
    connect(indiclient,&IndiCLient::SigNewNumber,this,&FocusModule::IndiNewNumber);
    connect(indiclient,&IndiCLient::SigNewMessage,this,&FocusModule::IndiNewMessage);
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
void FocusModule::IndiNewNumber(INumberVectorProperty *nvp)
{
    if (
            (QString(nvp->device) == _camera )
        &&  (nvp->s==IPS_ALERT)
       )
    {
        sendMessage("cameraAlert");
        emit cameraAlert();
    }
}

void FocusModule::IndiNewBLOB(IBLOB *bp)
{
    if (
            (QString(bp->bvp->device) == _camera)
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

void FocusModule::IndiNewSwitch(ISwitchVectorProperty *svp)
{
    if (
            (QString(svp->device) == _camera)
//        &&  (QString(svp->name)   =="CCD_FRAME_RESET")
        &&  (svp->s==IPS_ALERT)
       )
    {
        sendMessage("cameraAlert");
        emit cameraAlert();
    }

    if (
            (QString(svp->device) == _focuser)
        &&  (QString(svp->name)   =="ABS_FOCUS_POSITION")
        &&  (svp->s==IPS_OK)
       )
    {
        sendMessage("focuserReachedPosition");
        emit focuserReachedPosition();
    }
    if (
            (QString(svp->device) == _camera)
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
    QState *framereset  = new QState();
    QState *exprequest  = new QState();
    QState *expwait     = new QState();
    QState *loadblob    = new QState();
    QFinalState *abort  = new QFinalState();

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

    framereset->addTransition(this, &FocusModule::abort, abort);
    exprequest->addTransition(this, &FocusModule::abort, abort);
    expwait->   addTransition(this, &FocusModule::abort, abort);
    loadblob->  addTransition(this, &FocusModule::abort, abort);

    _machine->addState(framereset);
    _machine->addState(exprequest);
    _machine->addState(expwait);
    _machine->addState(loadblob);
    _machine->addState(abort);

    _machine->setInitialState(framereset);
    _machine->start();
    sendMessage("machine started");

}
void FocusModule::SMFrameReset()
{
    sendMessage("SMFrameReset");
    frameReset(_camera);
    emit frameresetdone();
}
void FocusModule::SMExpRequest()
{
    sendMessage("SMExpRequest");
    sendNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE",2);
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

