#include <QtCore>
#include <QtConcurrent>
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
            (QString(nvp->device) =="CCD Simulator")
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
            (QString(bp->bvp->device) =="CCD Simulator")
       )
    {
        sendMessage("blobReceived " + QString(bp->bvp->device) + " - " + QString(bp->name) + " - " + QString::number(bp->bloblen));
        emit blobReceived();
    }

}

void FocusModule::IndiNewSwitch(ISwitchVectorProperty *svp)
{
    if (
            (QString(svp->device) =="CCD Simulator")
//        &&  (QString(svp->name)   =="CCD_FRAME_RESET")
        &&  (svp->s==IPS_ALERT)
       )
    {
        sendMessage("cameraAlert");
        emit cameraAlert();
    }

    if (
            (QString(svp->device) =="Focuser Simulator")
        &&  (QString(svp->name)   =="ABS_FOCUS_POSITION")
        &&  (svp->s==IPS_OK)
       )
    {
        sendMessage("focuserReachedPosition");
        emit focuserReachedPosition();
    }
    if (
            (QString(svp->device) =="CCD Simulator")
        &&  (QString(svp->name)   =="CCD_FRAME_RESET")
        &&  (svp->s==IPS_OK)
       )
    {
        sendMessage("frameResetDone");
        emit frameResetDone();
    }


}

void FocusModule::CallStartCoarse(void)
{

    machine.stop();
    sendMessage("machine stopped");
}

void FocusModule::startCoarse(void)
{

    QState *framereset = new QState();
    QObject::connect(framereset, &QState::entered, this, &FocusModule::SMFrameReset);


    QState *exprequest = new QState();
    QObject::connect(exprequest, &QState::entered, this, &FocusModule::SMExpRequest);

    QState *alert = new QState();
    QObject::connect(alert, &QState::entered, this, &FocusModule::SMAlert);


    QFinalState *abort = new QFinalState();
    QObject::connect(abort, &QState::entered, this, &FocusModule::CallStartCoarse);

    exprequest->addTransition(this, &FocusModule::cameraAlert, alert);
    exprequest->addTransition(this, &FocusModule::blobReceived, exprequest);
    exprequest->addTransition(this, &FocusModule::abort, abort);

    framereset->addTransition(this, &FocusModule::cameraAlert, alert);
    framereset->addTransition(this, &FocusModule::abort, abort);
    framereset->addTransition(this, &FocusModule::frameResetDone, exprequest);

    alert->addTransition(this, &FocusModule::abort, abort);


    machine.addState(exprequest);
    machine.addState(framereset);
    machine.addState(abort);
    machine.addState(alert);

    machine.setInitialState(framereset);
    machine.start();
    sendMessage("machine started");

}
void FocusModule::SMFrameReset(void)
{
    sendMessage("SMFrameReset");
    frameReset("CCD Simulator");
}
void FocusModule::SMExpRequest(void)
{
    sendMessage("SMExpRequest");
    sendNewNumber("CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",2);
}
void FocusModule::SMAlert(void)
{
    sendMessage("SMAlert");
    emit abort();
//    sendNewNumber("CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",2);
}
