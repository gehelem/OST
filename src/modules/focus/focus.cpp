#include "focus.h"
#include "polynomialfit.h"

FocusModule *initialize(QString name,QString label)
{
    FocusModule *basemodule = new FocusModule(name,label);
    return basemodule;
}

FocusModule::FocusModule(QString name,QString label)
    : Basemodule(name,label)

{
    _devices = new TextProperty(_modulename,"Options","root","devices","Devices",2,0);
    _devices->addText(new TextValue("camera","Camera","hint",_camera));
    _devices->addText(new TextValue("focuser","Focuser","hint",_focuser));
    emit propertyCreated(_devices,&_modulename);
    _propertyStore.add(_devices);

    _values = new NumberProperty(_modulename,"Control","root","values","Values",0,0);
    _values->addNumber(new NumberValue("loopHFRavg","Average HFR","hint",0,"",0,99,0));
    _values->addNumber(new NumberValue("focpos","Focuser position","hint",0,"",0,99,0));
    _values->addNumber(new NumberValue("bestpos","Best position","hint",0,"",0,99,0));
    _values->addNumber(new NumberValue("bestposfit","Pol. fit position","hint",0,"",0,99,0));
    _values->addNumber(new NumberValue("imgHFR","Last imgage HFR","hint",0,"",0,99,0));
    _values->addNumber(new NumberValue("iteration","Iteration","hint",0,"",0,99,0));
    emit propertyCreated(_values,&_modulename);
    _propertyStore.add(_values);

    _actions = new SwitchProperty(_modulename,"Control","root","actions","Actions",2,0,1);
    _actions->addSwitch(new SwitchValue("condev","Connect devices","hint",0));
    _actions->addSwitch(new SwitchValue("coarse","Autofocus","hint",0));
    _actions->addSwitch(new SwitchValue("loop","Infinite loop","hint",0));
    _actions->addSwitch(new SwitchValue("abort","Abort","hint",0));
    _actions->addSwitch(new SwitchValue("loadconfs","Load devices conf","hint",0));
    emit propertyCreated(_actions,&_modulename);
    _propertyStore.add(_actions);

    _parameters = new NumberProperty(_modulename,"Control","root","parameters","Parameters",2,0);
    _parameters->addNumber(new NumberValue("startpos"      ,"Start position","hint",_startpos,"",0,100000,100));
    _parameters->addNumber(new NumberValue("steps"         ,"Steps gap","hint",_steps,"",0,2000,100));
    _parameters->addNumber(new NumberValue("iterations"    ,"Iterations","hint",_iterations,"",0,99,1));
    _parameters->addNumber(new NumberValue("loopIterations","Average over","hint",_loopIterations,"",0,99,1));
    _parameters->addNumber(new NumberValue("exposure"      ,"Exposure","hint",_exposure,"",0,120,1));
    _parameters->addNumber(new NumberValue("backlash"      ,"Backlash overshoot","hint",_backlash,"",0,1000,1));
    emit propertyCreated(_parameters,&_modulename);
    _propertyStore.add(_parameters);

    ImageProperty* img = new ImageProperty(_modulename,"Control","root","viewer","Image property label",0,0,0);
    emit propertyCreated(img,&_modulename);
    _propertyStore.add(img);
}

FocusModule::~FocusModule()
{

}
void FocusModule::OnSetPropertyNumber(NumberProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
    _propertyStore.add(prop);

    QList<NumberValue*> numbers=prop->getNumbers();
    for (int j = 0; j < numbers.size(); ++j) {
        if (numbers[j]->name()=="startpos")        _startpos=numbers[j]->getValue();
        if (numbers[j]->name()=="steps")           _steps=numbers[j]->getValue();
        if (numbers[j]->name()=="iterations")      _iterations=numbers[j]->getValue();
        if (numbers[j]->name()=="loopIterations")  _loopIterations=numbers[j]->getValue();
        if (numbers[j]->name()=="exposure")        _exposure=numbers[j]->getValue();
        if (numbers[j]->name()=="backlash")        _backlash=numbers[j]->getValue();
        emit propertyUpdated(prop,&_modulename);
        BOOST_LOG_TRIVIAL(debug) << "Focus number property item modified " << prop->getName().toStdString();
    }
}
void FocusModule::OnSetPropertyText(TextProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
}

void FocusModule::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
    SwitchProperty* wprop = _propertyStore.getSwitch(prop->getDeviceName(),prop->getGroupName(),prop->getName());
    QList<SwitchValue*> switchs=prop->getSwitches();
    for (int j = 0; j < switchs.size(); ++j) {
        if (switchs[j]->name()=="coarse") {
            wprop->setSwitch(switchs[j]->name(),true);
            startCoarse();
        }
        if (switchs[j]->name()=="loadconfs") {
            wprop->setSwitch(switchs[j]->name(),true);
            loadDevicesConfs();
        }
        if (switchs[j]->name()=="abort")  {
            wprop->setSwitch(switchs[j]->name(),true);
            emit abort();
        }
        if (switchs[j]->name()=="condev") {
            wprop->setSwitch(switchs[j]->name(),true);
            connectAllDevices();
        }
        //prop->setSwitches(switchs);
        _propertyStore.update(wprop);
        emit propertyUpdated(wprop,&_modulename);
        BOOST_LOG_TRIVIAL(debug) << "Focus switch property item modified " << wprop->getName().toStdString();
    }
}

void FocusModule::newNumber(INumberVectorProperty *nvp)
{
    if (
            (QString(nvp->device) == _camera )
        &&  (nvp->s==IPS_ALERT)
       )
    {
        sendMessage("cameraAlert");
        emit cameraAlert();
    }
    if (
            (QString(nvp->device) == _focuser)
        &&  (QString(nvp->name)   =="ABS_FOCUS_POSITION")
       )
    {
        NumberProperty* prop = _propertyStore.getNumber("Control","root","values");
        prop->setNumber("focpos",nvp->np[0].value);
        _propertyStore.update(prop);
        emit propertyUpdated(prop,&_modulename);

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
            (QString(bp->bvp->device) == _camera)
       )
    {
        image = new Image();
        image->LoadFromBlob(bp);
        image->CalcStats();
        image->computeHistogram();
        image->saveStretchedToJpeg(_webroot+"/"+QString(bp->bvp->device)+".jpeg",100);

        ImageProperty* img = new ImageProperty(_modulename,"Control","root","viewer","Image property label",0,0,0);
        img->setURL(QString(bp->bvp->device)+".jpeg");
        emit propertyUpdated(img,&_modulename);
        _propertyStore.add(img);
        if (_machine.isRunning()) {
            emit ExposureDone();
            emit ExposureBestDone();
        }
    }

}

void FocusModule::newSwitch(ISwitchVectorProperty *svp)
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
            (QString(svp->device) == _camera)
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
    connect(Abort,              &QState::entered, this, &FocusModule::SMAbort);
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
    CoarseFocus->       addTransition(this,&FocusModule::abort,                Abort);
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

    if (!frameReset(_camera))
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
    _solver.ResetSolver(image->stats,image->m_ImageBuffer);
    connect(&_solver,&Solver::successSEP,this,&FocusModule::OnSucessSEP);
    _solver.FindStars();
}

void FocusModule::OnSucessSEP()
{
    //sendMessage("FindStarsDone");
    NumberProperty* prop = _propertyStore.getNumber("Control","root","values");
    prop->setNumber("imgHFR",_solver.HFRavg);
    _propertyStore.update(prop);
    emit propertyUpdated(prop,&_modulename);
    emit FindStarsDone();
}

void FocusModule::SMCompute()
{
    sendMessage("SMCompute");

    _posvector.push_back(_startpos + _iteration*_steps);
    _hfdvector.push_back(_loopHFRavg);

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

    NumberProperty* prop = _propertyStore.getNumber("Control","root","values");
    prop->setNumber("loopHFRavg",_loopHFRavg);
    prop->setNumber("bestpos",_bestpos);
    prop->setNumber("bestposfit",_bestposfit);
    prop->setNumber("focpos",_startpos + _iteration*_steps);
    prop->setNumber("iteration",_iteration);
    _propertyStore.update(prop);
    emit propertyUpdated(prop,&_modulename);

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
    NumberProperty* prop = _propertyStore.getNumber("Control","root","values");
    prop->setNumber("imgHFR",_solver.HFRavg);
    _propertyStore.update(prop);
    emit propertyUpdated(prop,&_modulename);
    // what should i do here ?
    emit ComputeResultDone();
}




void FocusModule::SMInitLoopFrame()
{
    sendMessage("SMInitLoopFrame");
    _loopIteration=0;
    _loopHFRavg=99;
    NumberProperty* prop = _propertyStore.getNumber("Control","root","values");
    prop->setNumber("loopHFRavg",_loopHFRavg);
    _propertyStore.update(prop);
    emit propertyUpdated(prop,&_modulename);
    emit InitLoopFrameDone();
}

void FocusModule::SMComputeLoopFrame()
{
    sendMessage("SMComputeLoopFrame");
    _loopIteration++;
    _loopHFRavg=((_loopIteration-1)*_loopHFRavg + _solver.HFRavg)/_loopIteration;
    NumberProperty* prop = _propertyStore.getNumber("Control","root","values");
    prop->setNumber("loopHFRavg",_loopHFRavg);
    prop->setNumber("imgHFR",_solver.HFRavg);
    _propertyStore.update(prop);
    emit propertyUpdated(prop,&_modulename);

    qDebug() << "Loop    " << _loopIteration << "/" << _loopIterations << " = " <<  _solver.HFRavg;


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

