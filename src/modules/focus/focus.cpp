#include "focus.h"
#include "polynomialfit.h"

FocusModule *initialize()
{
    FocusModule *basemodule = new FocusModule();
    return basemodule;
}

FocusModule::FocusModule()
{
    properties=Properties::getInstance();
    _devices["camera"]="";
    _devices["focuser"]="";
    properties->createDevcat(_modulename,"ctl","Control",1);
    properties->createProp(_modulename,"values","Valeurs"    ,PT_NUM,"ctl","",OP_RO,OSRule::OSR_NOFMANY,0,OPState::OPS_IDLE,"","",1);
    properties->createProp(_modulename,"params","Paramètres" ,PT_NUM,"ctl","",OP_RW,OSRule::OSR_NOFMANY,0,OPState::OPS_IDLE,"","",1);
    properties->appendElt(_modulename,"values","loopHFRavg",0,"Average HFR","","");
    properties->appendElt(_modulename,"values","focpos"    ,0,"Focuser position","","");
    properties->appendElt(_modulename,"values","imgHFR",0,"Last imgage HFR","","");

    properties->appendElt(_modulename,"params","startpos",0,"Départ","","");
    properties->appendElt(_modulename,"params","steps"    ,0,"Incrément","","");
    properties->appendElt(_modulename,"params","iterations",0,"Nombre d'incréments","","");
    properties->appendElt(_modulename,"params","loopIterations"    ,0,"Moyenne sur","","");
    properties->appendElt(_modulename,"params","exposure",0,"Exposition","","");
    properties->appendElt(_modulename,"params","backlash"    ,0,"Backlash","","");

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
    if (txt=="w")  properties->dumproperties();
    if (txt=="f")
    {
        _startpos = 30000;
        _steps = 1000;
        _iterations = 10;
        _loopIterations = 4;
        _exposure =2;
        _backlash=100;
        properties->setElt(_modulename,"params","startpos",_startpos);
        properties->setElt(_modulename,"params","steps",_steps);
        properties->setElt(_modulename,"params","iterations",_iterations);
        properties->setElt(_modulename,"params","loopIterations",_loopIterations);
        properties->setElt(_modulename,"params","exposure",_exposure);
        properties->setElt(_modulename,"params","backlash",_backlash);
        properties->emitProp(_modulename,"params");
        startCoarse();
    }

}
void FocusModule::newNumber(INumberVectorProperty *nvp)
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
       )
    {
        properties->setElt(_modulename,"values","focpos",nvp->np[0].value);
        properties->emitProp(_modulename,"values");
        if (nvp->s==IPS_OK)
        {
            sendMessage("focuserReachedPosition");
            emit GotoBestDone();
            emit BacklashBestDone();
            emit BacklashDone();
            emit GotoNextDone();
            emit GotoStartDone();
        }
    }

}

void FocusModule::newBLOB(IBLOB *bp)
{
    if (
            (QString(bp->bvp->device) == _devices["camera"])
       )
    {
        image.reset(new Image());
        connect(image.get(),&Image::successSEP        ,this,&FocusModule::OnSucessSEP);
        image->LoadFromBlob(bp);
        if (_machine.isRunning()) {
            emit ExposureDone();
            emit ExposureBestDone();
        }
    }

}

void FocusModule::newSwitch(ISwitchVectorProperty *svp)
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
    _posvector.clear();
    _hfdvector.clear();
    _coefficients.clear();
    _iteration=0;
    _besthfr=99;
    _bestposfit=99;

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

    auto *Compute             = new QState(Loop);
    auto *RequestGotoNext     = new QState(Loop);
    auto *WaitGotoNext        = new QState(Loop);
    auto *LoopFrame           = new QState(Loop);

    auto *InitLoopFrame     = new QState(LoopFrame);
    auto *RequestExposure   = new QState(LoopFrame);
    auto *WaitExposure      = new QState(LoopFrame);
    auto *FindStars         = new QState(LoopFrame);
    auto *ComputeLoopFrame  = new QState(LoopFrame);



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
    Loop->setInitialState(LoopFrame);
    LoopFrame->setInitialState(InitLoopFrame);
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
    connect(ComputeLoopFrame,   &QState::entered, this, &FocusModule::SMComputeLoopFrame);
    connect(InitLoopFrame,      &QState::entered, this, &FocusModule::SMInitLoopFrame);

    /* mapping signals to state transitions */
    RequestFrameReset-> addTransition(this,&FocusModule::RequestFrameResetDone,WaitFrameReset);
    WaitFrameReset->    addTransition(this,&FocusModule::FrameResetDone,       RequestBacklash);
    RequestBacklash->   addTransition(this,&FocusModule::RequestBacklashDone,  WaitBacklash);
    WaitBacklash->      addTransition(this,&FocusModule::BacklashDone,         RequestGotoStart);
    RequestGotoStart->  addTransition(this,&FocusModule::RequestGotoStartDone, WaitGotoStart);
    WaitGotoStart->     addTransition(this,&FocusModule::GotoStartDone,        Loop);

    RequestExposure->   addTransition(this,&FocusModule::RequestExposureDone,   WaitExposure);
    WaitExposure->      addTransition(this,&FocusModule::ExposureDone,          FindStars);
    FindStars->         addTransition(this,&FocusModule::FindStarsDone,         ComputeLoopFrame);
    Compute->           addTransition(this,&FocusModule::LoopFinished,          Finish);

    Compute->           addTransition(this,&FocusModule::NextLoop,              RequestGotoNext);
    RequestGotoNext->   addTransition(this,&FocusModule::RequestGotoNextDone,   WaitGotoNext);
    WaitGotoNext->      addTransition(this,&FocusModule::GotoNextDone,          LoopFrame);

    RequestBacklashBest->   addTransition(this,&FocusModule::RequestBacklashBestDone,   WaitBacklashBest);
    WaitBacklashBest->      addTransition(this,&FocusModule::BacklashBestDone,          RequestGotoBest);
    RequestGotoBest->       addTransition(this,&FocusModule::RequestGotoBestDone,       WaitGotoBest);
    WaitGotoBest->          addTransition(this,&FocusModule::GotoBestDone,              RequestExposureBest);
    RequestExposureBest->   addTransition(this,&FocusModule::RequestExposureBestDone,   WaitExposureBest);
    WaitExposureBest->      addTransition(this,&FocusModule::ExposureBestDone,          ComputeResult);
    ComputeResult->         addTransition(this,&FocusModule::ComputeResultDone,         Final);

    InitLoopFrame-> addTransition(this ,&FocusModule::InitLoopFrameDone,RequestExposure );
    ComputeLoopFrame->addTransition(this,&FocusModule::NextFrame,RequestExposure);
    ComputeLoopFrame->addTransition(this,&FocusModule::LoopFrameDone,Compute);





    _machine.start();
    sendMessage("machine started");
    qDebug() << "Start coarse focus";
}

void FocusModule::SMRequestFrameReset()
{
    sendMessage("SMRequestFrameReset");



    /*qDebug() << "conf count" << _machine.configuration().count();
    QSet<QAbstractState *>::iterator i;
    for (i=_machine.configuration().begin();i !=_machine.configuration().end();i++)
    {
        qDebug() << (*i)->objectName();
    }*/

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
    if (!sendModNewNumber(_focuser,"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION", _startpos - _backlash))
    {
        emit abort();
        return;
    }
    emit RequestBacklashDone();
}

void FocusModule::SMRequestGotoStart()
{
    sendMessage("SMRequestGotoStart");
    if (!sendModNewNumber(_focuser,"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION", _startpos))
    {
        emit abort();
        return;
    }
    emit RequestGotoStartDone();
}

void FocusModule::SMRequestExposure()
{
    sendMessage("SMRequestExposure");
    if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure))
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
    properties->setElt(_modulename,"values","imgHFR",image->HFRavg);
    properties->emitProp(_modulename,"values");
    emit FindStarsDone();
}

void FocusModule::SMCompute()
{
    sendMessage("SMCompute");

    _posvector.push_back(_startpos + _iteration*_steps);
    _hfdvector.push_back(_loopHFRavg);
    //properties->setElt(_modulename,"values","loopHFRavg",_loopHFRavg);
    //properties->setElt(_modulename,"values","focpos",_startpos + _iteration*_steps);
    //properties->emitProp(_modulename,"values");

    if (_posvector.size() > 2)
    {
        double coeff[3];
        polynomialfit(_posvector.size(), 3, _posvector.data(), _hfdvector.data(), coeff);
        _bestposfit= -coeff[1]/(2*coeff[2]);
    }

    if ( _loopHFRavg < _besthfr )
    {
        _besthfr=_loopHFRavg;
        //emit valueChanged(_loopHFRavg);
        _bestpos=_startpos + _iteration*_steps;
    }
    qDebug() << "Compute " << _iteration << "/" << _iterations << "=" << _loopHFRavg << "bestpos/pos" << _bestpos << "/" << _startpos + _iteration*_steps << "polfit=" << _bestposfit;

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
    if (!sendModNewNumber(_focuser,"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION", _startpos + _iteration*_steps))
    {
        emit abort();
        return;
    }
    emit RequestGotoNextDone();
}

void FocusModule::SMRequestBacklashBest()
{
    sendMessage("SMRequestBacklashBest");
    if (!sendModNewNumber(_focuser,"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION", _bestpos - _backlash))
    {
        emit abort();
        return;
    }
    emit RequestBacklashBestDone();
}

void FocusModule::SMRequestGotoBest()
{
    sendMessage("SMRequestGotoBest");
    if (!sendModNewNumber(_focuser,"ABS_FOCUS_POSITION","FOCUS_ABSOLUTE_POSITION", _bestpos))
    {
        emit abort();
        return;
    }
    emit RequestGotoBestDone();
}

void FocusModule::SMRequestExposureBest()
{
    sendMessage("SMRequestExposureBest");
    if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure))
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




void FocusModule::SMInitLoopFrame()
{
    sendMessage("SMInitLoopFrame");
    _loopIteration=0;
    _loopHFRavg=99;
    properties->setElt(_modulename,"values","loopHFRavg",_loopHFRavg);
    properties->emitProp(_modulename,"values");

    emit InitLoopFrameDone();
}

void FocusModule::SMComputeLoopFrame()
{
    sendMessage("SMComputeLoopFrame");
    _loopIteration++;
    _loopHFRavg=((_loopIteration-1)*_loopHFRavg + image->HFRavg)/_loopIteration;
    properties->setElt(_modulename,"values","loopHFRavg",_loopHFRavg);
    properties->emitProp(_modulename,"values");

    qDebug() << "Loop    " << _loopIteration << "/" << _loopIterations << " = " <<  image->HFRavg;


    if (_loopIteration < _loopIterations )
    {
        emit NextFrame();
    }
    else
    {
        emit LoopFrameDone();
    }
}

void FocusModule::SMAlert()
{
    sendMessage("SMAlert");
    emit abort();
}

