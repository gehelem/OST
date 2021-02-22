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
        //machine->submitEvent(QLatin1String("expdone"));

        emit expdone();
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
        emit frameresetdone();
    }


}

void FocusModule::CallStartCoarse()
{

    machine->stop();
    sendMessage("machine stopped");
}

void FocusModule::startCoarse()
{

    auto *machine = QScxmlStateMachine::fromFile("/home/gilles/OST/scxml/coarsefocus.scxml");
    for(QScxmlError& error:machine->parseErrors())
    {
        sendMessage("Can't load machine" + error.description());
    }
    if (machine->parseErrors().size()>0) return;



    /* connecting machine events to module methods */
    machine->connectToEvent("SMExpRequest",this, &FocusModule::SMExpRequest);
    machine->connectToEvent("SMFrameReset",this, &FocusModule::SMFrameReset);
    machine->connectToEvent("SMAlert",this, &FocusModule::SMAlert);

    /* connecting module events to machine events */
    connect(this, &FocusModule::abort,
                [machine] {
                machine->submitEvent("abort");
        });
    connect(this, &FocusModule::cameraAlert,
                [machine] {
                machine->submitEvent("cameraAlert");
        });
    connect(this, &FocusModule::expdone,
                [machine] {
                machine->submitEvent("expdone");
        });
    connect(this, &FocusModule::exprequestdone,
                [machine] {
                machine->submitEvent("exprequestdone");
        });
    connect(this, &FocusModule::frameresetdone,
                [machine] {
                machine->submitEvent("frameresetdone");
        });


    machine->start();
    sendMessage("machine started");

}
void FocusModule::SMFrameReset()
{
    sendMessage("SMFrameReset");
    frameReset("CCD Simulator");
    emit frameresetdone();
}
void FocusModule::SMExpRequest()
{
    sendMessage("SMExpRequest");
    sendNewNumber("CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",2);
    emit exprequestdone();
}
void FocusModule::SMAlert()
{
    sendMessage("SMAlert");
    emit abort();
}
