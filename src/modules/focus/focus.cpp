#include "focus.h"
#include "polynomialfit.h"

FocusModule *initialize(QString name,QString label,QString profile,QVariantMap availableModuleLibs)
{
    FocusModule *basemodule = new FocusModule(name,label,profile,availableModuleLibs);
    return basemodule;
}

FocusModule::FocusModule(QString name,QString label,QString profile,QVariantMap availableModuleLibs)
    : Basemodule(name,label,profile,availableModuleLibs)

{
    Q_INIT_RESOURCE(focus);
    _moduletype="focus";

    loadPropertiesFromFile(":focus.json");

    setOstProperty("moduleDescription","Focus module with statemachines",true);
    setOstProperty("moduleLabel","Focus module",true);
    setOstProperty("moduleVersion",0.1,true);

    _startpos=          getOstElementValue("parameters","startpos").toInt();
    _steps=             getOstElementValue("parameters","steps").toInt();
    _iterations=        getOstElementValue("parameters","iterations").toInt();
    _loopIterations=    getOstElementValue("parameters","loopIterations").toInt();
    _exposure=          getOstElementValue("parameters","exposure").toInt();
    _backlash=          getOstElementValue("parameters","backlash").toInt();



    /*_img = new ImageProperty(_modulename,"Control","root","viewer","Image property label",0,0,0);
    emit propertyCreated(_img,&_modulename);
    _propertyStore.add(_img);

    _grid = new GridProperty(_modulename,"Control","root","grid","Grid property label",0,0,"SXY","Set","Pos","HFR","","");
    emit propertyCreated(_grid,&_modulename);
    _propertyStore.add(_grid);*/

}

FocusModule::~FocusModule()
{

}
void FocusModule::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{
        //BOOST_LOG_TRIVIAL(debug) << "OnMyExternalEvent - recv : " << getName().toStdString() << "-" << eventType.toStdString() << "-" << eventKey.toStdString();
        foreach(const QString& keyprop, eventData.keys()) {
            foreach(const QString& keyelt, eventData[keyprop].toMap()["elements"].toMap().keys()) {
                BOOST_LOG_TRIVIAL(debug) << "OnMyExternalEvent - recv : " << getName().toStdString() << "-" << eventType.toStdString() << "-" << keyprop.toStdString() << "-" << keyelt.toStdString();
                QVariant val=eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                if (keyprop=="parameters") {
                    if (keyelt=="startpos") {
                        if (setOstElement(keyprop,keyelt,val,true)) {
                            _startpos=val.toInt();
                        }
                    }
                    if (keyelt=="steps") {
                        if (setOstElement(keyprop,keyelt,val,true)) {
                            _steps=val.toInt();
                        }
                    }
                    if (keyelt=="iterations") {
                        if (setOstElement(keyprop,keyelt,val,true)) {
                            _iterations=val.toInt();
                        }
                    }
                    if (keyelt=="loopIterations") {
                        if (setOstElement(keyprop,keyelt,val,true)) {
                            _loopIterations=val.toInt();
                        }
                    }
                    if (keyelt=="exposure") {
                        if (setOstElement(keyprop,keyelt,val,true)) {
                            _exposure=val.toInt();
                        }
                    }
                    if (keyelt=="backlash") {
                        if (setOstElement(keyprop,keyelt,val,true)) {
                            _backlash=val.toInt();
                        }
                    }

                }
                if (keyprop=="actions") {
                    if (keyelt=="condev") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
                            connectDevice(_camera);
                            connectDevice(_focuser);
                        }
                    }
                    if (keyelt=="discondev") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
                            disconnectDevice(_camera);
                            disconnectDevice(_focuser);
                        }
                    }
                    if (keyelt=="loadconfs") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
                            loadDevicesConfs();
                        }
                    }
                    if (keyelt=="coarse") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_BUSY,true);
                            startCoarse();
                        }
                    }
                    if (keyelt=="abort") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
                            emit abort();
                        }
                    }
                    if (keyelt=="loop") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
                        }
                    }

                }
                if (keyprop=="devices") {
                    if (keyelt=="camera") {
                        if (setOstElement(keyprop,keyelt,val,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
                            _camera=val.toString();
                        }
                    }
                    if (keyelt=="focuser") {
                        if (setOstElement(keyprop,keyelt,val,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
                            _focuser=val.toString();
                        }
                    }
                }
            }



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
        setOstElement("values","focpos",nvp->np[0].value,true);

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
        _image.loadBlob(bp);
        setOstPropertyAttribute("image","status",IPS_OK,true);

        QImage rawImage = _image.getRawQImage();
        rawImage.save(_webroot+"/"+QString(bp->bvp->device)+".jpeg","JPG",50);
        setOstPropertyAttribute("image","URL",QString(bp->bvp->device)+".jpeg",true);

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
    /*_grid->clear();
    _propertyStore.update(_grid);
    emit propertyUpdated(_grid,&_modulename);*/


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


    setBlobMode();

    /*qDebug() << "conf count" << _machine.configuration().count();
    QSet<QAbstractState *>::iterator i;
    for (i=_machine.configuration().begin();i !=_machine.configuration().end();i++)
    {
        qDebug() << (*i)->objectName();
    }*/

    if (!frameReset(_camera))
    {
        RequestFrameResetDone();
        usleep(1000);
        emit FrameResetDone();
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
    stats=_image.getStats();
    _solver.ResetSolver(stats,_image.getImageBuffer());
    connect(&_solver,&Solver::successSEP,this,&FocusModule::OnSucessSEP);
    _solver.FindStars(_solver.stellarSolverProfiles[0]);
}

void FocusModule::OnSucessSEP()
{
    setOstElement("values","imgHFR",_solver.HFRavg,true);
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

    setOstElement("values","loopHFRavg",_loopHFRavg,false);
    setOstElement("values","bestpos",   _bestpos,false);
    setOstElement("values","bestposfit",_bestposfit,false);
    setOstElement("values","focpos",    _startpos + _iteration*_steps,false);
    setOstElement("values","iteration", _iteration,true);

    /*_grid->append(_startpos + _iteration*_steps,_loopHFRavg);
    _propertyStore.update(_grid);
    emit propertyAppended(_grid,&_modulename,0,_startpos + _iteration*_steps,_loopHFRavg,0,0);*/

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
    setOstElement("values","imgHFR",_solver.HFRavg,true);
    // what should i do here ?
    emit ComputeResultDone();
}




void FocusModule::SMInitLoopFrame()
{
    sendMessage("SMInitLoopFrame");
    _loopIteration=0;
    _loopHFRavg=99;
    setOstElement("values","loopHFRavg",_loopHFRavg,true);
    emit InitLoopFrameDone();
}

void FocusModule::SMComputeLoopFrame()
{
    sendMessage("SMComputeLoopFrame");
    _loopIteration++;
    _loopHFRavg=((_loopIteration-1)*_loopHFRavg + _solver.HFRavg)/_loopIteration;
    setOstElement("values","loopHFRavg",_loopHFRavg,false);
    setOstElement("values","imgHFR",_solver.HFRavg,true);

    //qDebug() << "Loop    " << _loopIteration << "/" << _loopIterations << " = " <<  _solver.HFRavg;


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

