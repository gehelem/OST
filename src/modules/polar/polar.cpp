#include "polar.h"
#define PI 3.14159265

PolarModule *initialize(QString name,QString label)
{
    PolarModule *basemodule = new PolarModule(name,label);
    return basemodule;
}
PolarModule::PolarModule(QString name,QString label)
    : Basemodule(name,label)
{
    _moduledescription="Polar module";

    _actions = new SwitchProperty(_modulename,"Control","root","actions","Actions",2,0,1);
    _actions->addSwitch(new SwitchValue("condev","Connect devices","hint",0));
    _actions->addSwitch(new SwitchValue("loadconfs","Load devices conf","hint",0));
    _actions->addSwitch(new SwitchValue("abort","Abort","hint",0));
    _actions->addSwitch(new SwitchValue("start","Start","hint",0));
    emit propertyCreated(_actions,&_modulename);
    _propertyStore.add(_actions);

    _commonParams = new NumberProperty(_modulename,"Parameters","root","commonParams","Common Parameters",2,0);
    _commonParams->addNumber(new NumberValue("exposure"      ,"Exposure (s)","hint",_exposure,"",0,5,1));
    emit propertyCreated(_commonParams,&_modulename);
    _propertyStore.add(_commonParams);


    _values = new NumberProperty(_modulename,"Control","root","values","Values",0,0);
    //_values->addNumber(new NumberValue("pulseN","Pulse N","hint",_pulseN,"",0,10000,0));
    emit propertyCreated(_values,&_modulename);
    _propertyStore.add(_values);

    _img = new ImageProperty(_modulename,"Control","root","viewer","Image property label",0,0,0);
    emit propertyCreated(_img,&_modulename);
    _propertyStore.add(_img);

    _states = new LightProperty(_modulename,"Control","root","states","State",0,0);
    _states->addLight(new LightValue("idle"  ,"Idle","hint",1));
    emit propertyCreated(_states,&_modulename);
    _propertyStore.add(_states);

    buildStateMachine();

}
PolarModule::~PolarModule()
{

}
void PolarModule::OnSetPropertyNumber(NumberProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;

    QList<NumberValue*> numbers=prop->getNumbers();
    for (int j = 0; j < numbers.size(); ++j) {
        if (numbers[j]->name()=="exposure")        _exposure=numbers[j]->getValue();
        prop->setState(1);
        emit propertyUpdated(prop,&_modulename);
        _propertyStore.add(prop);
        //BOOST_LOG_TRIVIAL(debug) << "Focus number property item modified " << prop->getName().toStdString();
    }

}
void PolarModule::OnSetPropertyText(TextProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
}
void PolarModule::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;

    SwitchProperty* wprop = _propertyStore.getSwitch(prop->getDeviceName(),prop->getGroupName(),prop->getName());
    QList<SwitchValue*> switchs=prop->getSwitches();
    for (int j = 0; j < switchs.size(); ++j) {
        if (switchs[j]->name()=="start") {
            _machine.start();
            wprop->setSwitch(switchs[j]->name(),true);
        }
        if (switchs[j]->name()=="loadconfs") {
            wprop->setSwitch(switchs[j]->name(),true);
            loadDevicesConfs();
        }
        if (switchs[j]->name()=="abort")  {
            wprop->setSwitch(switchs[j]->name(),true);
            emit Abort();
        }
        if (switchs[j]->name()=="condev") {
            wprop->setSwitch(switchs[j]->name(),true);
            connectAllDevices();
        }
        //prop->setSwitches(switchs);
        _propertyStore.update(wprop);
        emit propertyUpdated(wprop,&_modulename);
    }

}
void PolarModule::newNumber(INumberVectorProperty *nvp)
{
    if (
            (QString(nvp->device) == _mount) &&
            (QString(nvp->name)   == "xx") &&
            (nvp->s   == IPS_IDLE)

       )
    {
        //xx
    }

}
void PolarModule::newBLOB(IBLOB *bp)
{
    if (
            (QString(bp->bvp->device) == _camera)
       )
    {
        delete image;
        image = new Image();
        image->LoadFromBlob(bp);
        image->CalcStats();
        image->computeHistogram();
        image->saveToJpeg(_webroot+"/"+QString(bp->bvp->device)+".jpeg",100);

        _img->setURL(QString(bp->bvp->device)+".jpeg");
        emit propertyUpdated(_img,&_modulename);
        _propertyStore.add(_img);
        BOOST_LOG_TRIVIAL(debug) << "Emit Exposure done";
        emit ExposureDone();
    }

}
void PolarModule::newSwitch(ISwitchVectorProperty *svp)
{
    if (
            (QString(svp->device) == _camera)
        &&  (QString(svp->name)   =="CCD_FRAME_RESET")
        &&  (svp->s==IPS_OK)
       )
    {
        sendMessage("FrameResetDone");
        emit FrameResetDone();
    }

}
void PolarModule::buildStateMachine(void)
{
    auto *Abort = new QState();
    auto *Polar = new QState();
    auto *End   = new QFinalState();

    auto *InitInit             = new QState(Polar);
    auto *RequestFrameReset    = new QState(Polar);
    auto *WaitFrameReset       = new QState(Polar);
    auto *RequestExposure      = new QState(Polar);
    auto *WaitExposure         = new QState(Polar);
    auto *FindStars            = new QState(Polar);
    auto *Compute              = new QState(Polar);
    auto *RequestMove          = new QState(Polar);
    auto *WaitMove             = new QState(Polar);
    auto *FinalCompute         = new QState(Polar);

    connect(InitInit            ,&QState::entered, this, &PolarModule::SMInit);
    connect(RequestFrameReset   ,&QState::entered, this, &PolarModule::SMRequestFrameReset);
    connect(RequestExposure     ,&QState::entered, this, &PolarModule::SMRequestExposure);
    connect(RequestMove         ,&QState::entered, this, &PolarModule::SMRequestMove);
    connect(FindStars           ,&QState::entered, this, &PolarModule::SMFindStars);
    connect(Compute             ,&QState::entered, this, &PolarModule::SMCompute);
    connect(Abort,               &QState::entered, this, &PolarModule::SMAbort);

    Polar->               addTransition(this,&PolarModule::Abort                ,Abort);
    Abort->               addTransition(this,&PolarModule::AbortDone            ,End);
    InitInit->            addTransition(this,&PolarModule::InitDone             ,RequestFrameReset);
    RequestFrameReset->   addTransition(this,&PolarModule::RequestFrameResetDone,WaitFrameReset);
    WaitFrameReset->      addTransition(this,&PolarModule::FrameResetDone       ,RequestExposure);
    RequestExposure->     addTransition(this,&PolarModule::RequestExposureDone  ,WaitExposure);
    WaitExposure->        addTransition(this,&PolarModule::ExposureDone         ,FindStars);
    FindStars->           addTransition(this,&PolarModule::FindStarsDone        ,Compute);
    Compute->             addTransition(this,&PolarModule::ComputeDone          ,RequestMove);
    Compute->             addTransition(this,&PolarModule::PolarDone            ,End);
    RequestMove->         addTransition(this,&PolarModule::RequestMoveDone      ,WaitMove);
    WaitMove->            addTransition(this,&PolarModule::MoveDone             ,RequestExposure);

    Polar->setInitialState(InitInit);

    _machine.addState(Polar);
    _machine.addState(Abort);
    _machine.addState(End);
    _machine.setInitialState(Polar);


}
void PolarModule::SMInit()
{
    BOOST_LOG_TRIVIAL(debug) << "SMInit";
    sendMessage("SMInit");
    setBlobMode();

    /* get mount DEC */
    if (!getModNumber(_mount,"EQUATORIAL_EOD_COORD","DEC",_mountDEC)) {
        emit Abort();
        return;
    }
    BOOST_LOG_TRIVIAL(debug) << "Mount DEC " << _mountDEC;
     /* get mount RA */
    if (!getModNumber(_mount,"EQUATORIAL_EOD_COORD","RA",_mountRA)) {
        emit Abort();
        return;
    }
    BOOST_LOG_TRIVIAL(debug) << "Mount DEC " << _mountRA;
    /* get mount Pier position  */
   if (!getModSwitch(_mount,"TELESCOPE_PIER_SIDE","PIER_WEST",_mountPointingWest)) {
       emit Abort();
       return;
   }
   BOOST_LOG_TRIVIAL(debug) << "PIER_WEST " << _mountPointingWest;

   _grid->clear();
   _propertyStore.update(_grid);
   emit propertyUpdated(_grid,&_modulename);
    BOOST_LOG_TRIVIAL(debug) << "SMInitDone";
    emit InitDone();
}
void PolarModule::SMRequestFrameReset()
{
    BOOST_LOG_TRIVIAL(debug) << "SMRequestFrameReset";
    sendMessage("SMRequestFrameReset");
    if (!frameReset(_camera))
    {
            emit Abort();
            return;
    }
    BOOST_LOG_TRIVIAL(debug) << "SMRequestFrameResetDone";
    emit RequestFrameResetDone();
}
void PolarModule::SMRequestExposure()
{
    BOOST_LOG_TRIVIAL(debug) << "SMRequestExposure";
    sendMessage("SMRequestExposure");
    if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure))
    {
        emit Abort();
        return;
    }
    emit RequestExposureDone();
}
void PolarModule::SMRequestMove()
{
    BOOST_LOG_TRIVIAL(debug) << "SMRequestMove";
    sendMessage("SMRequestMove");
    /*if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure))
    {
        emit Abort();
        return;
    }*/
    emit RequestMoveDone();
}

void PolarModule::SMCompute()
{
    BOOST_LOG_TRIVIAL(debug) << "SMCompute";

    emit ComputeDone();
}
void PolarModule::SMFindStars()
{
    BOOST_LOG_TRIVIAL(debug) << "SMFindStars";

    sendMessage("SMFindStars");
    _solver.ResetSolver(image->stats,image->m_ImageBuffer);
    connect(&_solver,&Solver::successSEP,this,&PolarModule::OnSucessSEP);
    _solver.stars.clear();
    _solver.FindStars(_solver.stellarSolverProfiles[0]);
}
void PolarModule::OnSucessSEP()
{
    BOOST_LOG_TRIVIAL(debug) << "OnSucessSEP";

    sendMessage("SEP finished");
    disconnect(&_solver,&Solver::successSEP,this,&PolarModule::OnSucessSEP);
    BOOST_LOG_TRIVIAL(debug) << "********* SEP Finished";
    emit FindStarsDone();
}
void PolarModule::SMAbort()
{

    _machine.stop();

    emit AbortDone();

    _states->addLight(new LightValue("idle"  ,"Idle","hint",1));
    emit propertyUpdated(_states,&_modulename);
    _propertyStore.update(_states);


}
