#include <QtCore>
#include <QtConcurrent>
#include <QScxmlStateMachine>
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
    connect(indiclient,&IndiCLient::SigNewDevice,this,&FocusModule::OnIndiNewDevice);
    connect(indiclient,&IndiCLient::SigRemoveDevice,this,&FocusModule::OnIndiRemoveDevice);
    connect(indiclient,&IndiCLient::SigNewProperty,this,&FocusModule::OnIndiNewProperty);
    connect(indiclient,&IndiCLient::SigRemoveProperty,this,&FocusModule::OnIndiRemoveProperty);
    connect(indiclient,&IndiCLient::SigNewText,this,&FocusModule::OnIndiNewText);
    connect(indiclient,&IndiCLient::SigNewSwitch,this,&FocusModule::OnIndiNewSwitch);
    connect(indiclient,&IndiCLient::SigNewLight,this,&FocusModule::OnIndiNewLight);
    connect(indiclient,&IndiCLient::SigNewBLOB,this,&FocusModule::OnIndiNewBLOB);
    connect(indiclient,&IndiCLient::SigNewNumber,this,&FocusModule::OnIndiNewNumber);
    connect(indiclient,&IndiCLient::SigNewMessage,this,&FocusModule::OnIndiNewMessage);
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
    if (txt=="f")
    {
        _startpos = 36000;
        _steps = 50;
        _iteration = 0;
        _iterations = 10;
        _exposure =2;
        _backlash=100;
        _besthfr=99;
        startCoarse();
    }

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
    if (
            (QString(nvp->device) == _devices["focuser"])
        &&  (QString(nvp->name)   =="ABS_FOCUS_POSITION")
        &&  (nvp->s==IPS_OK)
       )
    {
        sendMessage("focuserReachedPosition");

        emit GotoBestDone();
        emit BacklashBestDone();
        emit BacklashDone();
        emit GotoNextDone();
        emit GotoStartDone();
    }
}

void FocusModule::OnIndiNewBLOB(IBLOB *bp)
{
    if (
            (QString(bp->bvp->device) == _devices["camera"])
       )
    {
        image.reset(new Image());
        connect(image.get(),&Image::successSEP        ,this,&FocusModule::OnSucessSEP);
        image->LoadFromBlob(bp);
        emit ExposureDone();
        emit ExposureBestDone();
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
            (QString(svp->device) == _devices["camera"])
        &&  (QString(svp->name)   =="CCD_FRAME_RESET")
        &&  (svp->s==IPS_OK)
       )
    {
        sendMessage("FrameResetDone");
        if (_machine.isRunning()) emit FrameResetDone();
    }


}

void FocusModule::SMAbort()
{

    _machine.stop();
    sendMessage("machine stopped");
}

void FocusModule::startCoarse()
{

    //_machine = new QStateMachine();

    /* states definitions */
    auto *CoarseFocus = new QState();
    auto *Abort       = new QState();
    auto *Error       = new QState();
    auto *Init        = new QState(CoarseFocus);
    auto *Loop        = new QState(CoarseFocus);
    auto *Finish      = new QState(CoarseFocus);
    auto *Final       = new QFinalState();

    auto *RequestFrameReset   = new QState(Init);
    auto *WaitFrameReset      = new QState(Init);
    auto *RequestBacklash     = new QState(Init);
    auto *WaitBacklash        = new QState(Init);
    auto *RequestGotoStart    = new QState(Init);
    auto *WaitGotoStart       = new QState(Init);

    auto *RequestExposure     = new QState(Loop);
    auto *WaitExposure        = new QState(Loop);
    auto *FindStars           = new QState(Loop);
    auto *Compute             = new QState(Loop);
    auto *RequestGotoNext     = new QState(Loop);
    auto *WaitGotoNext        = new QState(Loop);

    auto *RequestBacklashBest   = new QState(Finish);
    auto *WaitBacklashBest      = new QState(Finish);
    auto *RequestGotoBest       = new QState(Finish);
    auto *WaitGotoBest          = new QState(Finish);
    auto *RequestExposureBest   = new QState(Finish);
    auto *WaitExposureBest      = new QState(Finish);
    auto *ComputeResult         = new QState(Finish);

    _machine.addState(CoarseFocus);
    _machine.addState(Abort);
    _machine.addState(Error);
    _machine.addState(Final);

    /* Set initial states */
    Init->setInitialState(RequestFrameReset);
    Loop->setInitialState(RequestExposure);
    Finish->setInitialState(RequestBacklashBest);
    CoarseFocus->setInitialState(Init);
    _machine.setInitialState(CoarseFocus);

    /* actions to take when entering into state */
    connect(RequestFrameReset,  &QState::entered, this, &FocusModule::SMRequestFrameReset);
    connect(RequestBacklash,    &QState::entered, this, &FocusModule::SMRequestBacklash);
    connect(RequestGotoStart,   &QState::entered, this, &FocusModule::SMRequestGotoStart);
    connect(RequestExposure,    &QState::entered, this, &FocusModule::SMRequestExposure);
    connect(FindStars,          &QState::entered, this, &FocusModule::SMFindStars);
    connect(Compute,            &QState::entered, this, &FocusModule::SMCompute);
    connect(RequestGotoNext,    &QState::entered, this, &FocusModule::SMRequestGotoNext);
    connect(RequestBacklashBest,&QState::entered, this, &FocusModule::SMRequestBacklashBest);
    connect(RequestGotoBest,    &QState::entered, this, &FocusModule::SMRequestGotoBest);
    connect(RequestExposureBest,&QState::entered, this, &FocusModule::SMRequestExposureBest);
    connect(ComputeResult,      &QState::entered, this, &FocusModule::SMComputeResult);

    /* mapping signals to state transitions */
    RequestFrameReset-> addTransition(this,&FocusModule::RequestFrameResetDone,WaitFrameReset);
    WaitFrameReset->    addTransition(this,&FocusModule::FrameResetDone,       RequestBacklash);
    RequestBacklash->   addTransition(this,&FocusModule::RequestBacklashDone,  WaitBacklash);
    WaitBacklash->      addTransition(this,&FocusModule::BacklashDone,         RequestGotoStart);
    RequestGotoStart->  addTransition(this,&FocusModule::RequestGotoStartDone, WaitGotoStart);
    WaitGotoStart->     addTransition(this,&FocusModule::GotoStartDone,        Loop);

    RequestExposure->   addTransition(this,&FocusModule::RequestExposureDone,   WaitExposure);
    WaitExposure->      addTransition(this,&FocusModule::ExposureDone,          FindStars);
    FindStars->         addTransition(this,&FocusModule::FindStarsDone,         Compute);
    Compute->           addTransition(this,&FocusModule::LoopFinished,          Finish);
    Compute->           addTransition(this,&FocusModule::NextLoop,              RequestGotoNext);
    RequestGotoNext->   addTransition(this,&FocusModule::RequestGotoNextDone,   WaitGotoNext);
    WaitGotoNext->      addTransition(this,&FocusModule::GotoNextDone,          RequestExposure);

    RequestBacklashBest->   addTransition(this,&FocusModule::RequestBacklashBestDone,   WaitBacklashBest);
    WaitBacklashBest->      addTransition(this,&FocusModule::BacklashBestDone,          RequestGotoBest);
    RequestGotoBest->       addTransition(this,&FocusModule::RequestGotoBestDone,       WaitGotoBest);
    WaitGotoBest->          addTransition(this,&FocusModule::GotoBestDone,              RequestExposureBest);
    RequestExposureBest->   addTransition(this,&FocusModule::RequestExposureBestDone,   WaitExposureBest);
    WaitExposureBest->      addTransition(this,&FocusModule::ExposureBestDone,          ComputeResult);
    ComputeResult->         addTransition(this,&FocusModule::ComputeResultDone,         Final);




    //QSet<QAbstractState *>::iterator i;
    /*qDebug() << "conf count" << _machine.configuration().count();
    for (QSet<QAbstractState *>::iterator i=_machine.configuration().begin();i !=_machine.configuration().end();i++)
    {
        qDebug() << *i;
    }*/
    _machine.start();
    sendMessage("machine started");

}

void FocusModule::SMRequestFrameReset()
{
    sendMessage("SMRequestFrameReset");
    if (!frameReset(_devices["camera"]))
    {
            emit abort();
            return;
    }
    emit RequestFrameResetDone();
}

void FocusModule::SMRequestBacklash()
{
    sendMessage("SMRequestBacklash");
    if (!sendNewNumber(_devices["focuser"],"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION", _startpos - _backlash))
    {
        emit abort();
        return;
    }
    emit RequestBacklashDone();
}

void FocusModule::SMRequestGotoStart()
{
    sendMessage("SMRequestGotoStart");
    if (!sendNewNumber(_devices["focuser"],"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION", _startpos))
    {
        emit abort();
        return;
    }
    emit RequestGotoStartDone();
}

void FocusModule::SMRequestExposure()
{
    sendMessage("SMRequestExposure");
    if (!sendNewNumber(_devices["camera"],"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure))
    {
        emit abort();
        return;
    }
    emit RequestExposureDone();
}

void FocusModule::SMFindStars()
{
    sendMessage("SMFindStars");
    image->FindStars();
}

void FocusModule::OnSucessSEP()
{
    sendMessage("FindStarsDone");
    emit FindStarsDone();
}

void FocusModule::SMCompute()
{
    sendMessage("SMCompute");

    if (image->HFRavg < _besthfr )
    {
        _besthfr=image->HFRavg;
        _bestpos=_startpos + _iteration*_steps;
    }

    if (_iteration <_iterations )
    {
        _iteration++;
        emit NextLoop();
    }
    else
    {
        emit LoopFinished();
    }
}

void FocusModule::SMRequestGotoNext()
{
    sendMessage("SMRequestGotoNext");
    if (!sendNewNumber(_devices["focuser"],"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION", _startpos + _iteration*_steps))
    {
        emit abort();
        return;
    }
    emit RequestGotoNextDone();
}

void FocusModule::SMRequestBacklashBest()
{
    sendMessage("SMRequestBacklashBest");
    if (!sendNewNumber(_devices["focuser"],"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION", _bestpos - _backlash))
    {
        emit abort();
        return;
    }
    emit RequestBacklashBestDone();
}

void FocusModule::SMRequestGotoBest()
{
    sendMessage("SMRequestGotoBest");
    if (!sendNewNumber(_devices["focuser"],"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION", _bestpos))
    {
        emit abort();
        return;
    }
    emit RequestGotoBestDone();
}

void FocusModule::SMRequestExposureBest()
{
    sendMessage("SMRequestExposureBest");
    if (!sendNewNumber(_devices["camera"],"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure))
    {
        emit abort();
        return;
    }
    emit RequestExposureBestDone();
}

void FocusModule::SMComputeResult()
{
    sendMessage("SMComputeResult");
    // what should i do here ?
    emit ComputeResultDone();
}

void FocusModule::SMAlert()
{
    sendMessage("SMAlert");
    emit abort();
}



