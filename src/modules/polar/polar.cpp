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

    _commonParams = new NumberProperty(_modulename,"Control","root","commonParams","Parameters",2,0);
    _commonParams->addNumber(new NumberValue("exposure"      ,"Exposure (s)","hint",_exposure,"",0,5,1));
    emit propertyCreated(_commonParams,&_modulename);
    _propertyStore.add(_commonParams);


    _values = new NumberProperty(_modulename,"Control","root","values","Values",0,0);
    _values->addNumber(new NumberValue("ra0","RA 0"  ,"hint",_ra0,"",0,10000,0));
    _values->addNumber(new NumberValue("de0","DE 0"  ,"hint",_de0,"",0,10000,0));
    _values->addNumber(new NumberValue("t0", "Time 0","hint",_t0 ,"",0,10000,0));
    _values->addNumber(new NumberValue("ra1","RA 1"  ,"hint",_ra1,"",0,10000,0));
    _values->addNumber(new NumberValue("de1","DE 1"  ,"hint",_de1,"",0,10000,0));
    _values->addNumber(new NumberValue("t1", "+Time 1","hint",_t1 ,"",0,10000,0));
    _values->addNumber(new NumberValue("ra2","RA 2"  ,"hint",_ra2,"",0,10000,0));
    _values->addNumber(new NumberValue("de2","DE 2"  ,"hint",_de2,"",0,10000,0));
    _values->addNumber(new NumberValue("t2", "+Time 2","hint",_t2 ,"",0,10000,0));
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
            (QString(nvp->name)   == "EQUATORIAL_EOD_COORD") &&
            (nvp->s   == IPS_OK)

       )
    {
        emit MoveDone();
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
    auto *FinalCompute         = new QFinalState(Polar);

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
    Compute->             addTransition(this,&PolarModule::PolarDone            ,FinalCompute);
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
    BOOST_LOG_TRIVIAL(debug) << "Mount RA " << _mountRA;

    /* get focal length */
    if (!getModNumber(_mount,"TELESCOPE_INFO","TELESCOPE_FOCAL_LENGTH",_focalLength)) {
        emit Abort();
        return;
    }
    BOOST_LOG_TRIVIAL(debug) << "focal lenth " << _focalLength;

    /* get pixel size */
    if (!getModNumber(_camera,"CCD_INFO","CCD_PIXEL_SIZE",_pixelSize)) {
        emit Abort();
        return;
    }
    BOOST_LOG_TRIVIAL(debug) << "Pixel size " << _pixelSize;

    /* get ccd width */
    if (!getModNumber(_camera,"CCD_INFO","CCD_MAX_X",_ccdX)) {
        emit Abort();
        return;
    }
    BOOST_LOG_TRIVIAL(debug) << "CCD width " << _ccdX;

    /* get ccd height */
    if (!getModNumber(_camera,"CCD_INFO","CCD_MAX_Y",_ccdY)) {
        emit Abort();
        return;
    }
    BOOST_LOG_TRIVIAL(debug) << "CCD height " << _ccdY;


    _ccdSampling=206*_pixelSize/_focalLength;
    BOOST_LOG_TRIVIAL(debug) << "CCD sampling (arcs/px) " << _ccdSampling;
    _ccdFov=sqrt(square(_ccdX)+square(_ccdY))*_ccdSampling;
    BOOST_LOG_TRIVIAL(debug) << "CCD FOV (arcs) " << _ccdFov;
    BOOST_LOG_TRIVIAL(debug) << "CCD FOV (arcm) " << _ccdFov/60;
    BOOST_LOG_TRIVIAL(debug) << "CCD FOV (deg)  " << _ccdFov/3600;


    /* get mount Pier position  */
   if (!getModSwitch(_mount,"TELESCOPE_PIER_SIDE","PIER_WEST",_mountPointingWest)) {
       emit Abort();
       return;
   }
   BOOST_LOG_TRIVIAL(debug) << "PIER_WEST " << _mountPointingWest;

   _itt=0;
   _ra0=0;_de0=0;_t0=0;
   _ra1=0;_de1=0;_t1=0;
   _ra2=0;_de2=0;_t2=0;

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
    if (_itt==0) {
        _t0=QDateTime::currentDateTime().currentMSecsSinceEpoch();
    }
    if (_itt==1) {
        _t1=QDateTime::currentDateTime().currentMSecsSinceEpoch();
    }
    if (_itt==2) {
        _t2=QDateTime::currentDateTime().currentMSecsSinceEpoch();
    }

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

    double oldRA,newRA;

    /* get mount RA */
    if (!getModNumber(_mount,"EQUATORIAL_EOD_COORD","RA",oldRA)) {
        BOOST_LOG_TRIVIAL(debug) << "SMRequestMove error 1";
        emit Abort();
        return;
    }


    if (_mountPointingWest) newRA=oldRA-15*24/360; else newRA=oldRA+15*24/360;
    BOOST_LOG_TRIVIAL(debug) << "SMRequestMove old RA = " << oldRA << " new RA= " << newRA << " pointing west " << _mountPointingWest;

    if (!sendModNewNumber(_mount,"EQUATORIAL_EOD_COORD","RA", newRA))
    {
        BOOST_LOG_TRIVIAL(debug) << "SMRequestMove error 2";
        emit Abort();
        return;
    }
    BOOST_LOG_TRIVIAL(debug) << "SMRequestMove done";
    emit RequestMoveDone();

}

void PolarModule::SMCompute()
{
    BOOST_LOG_TRIVIAL(debug) << "*******  SMCompute ******** ";
    BOOST_LOG_TRIVIAL(debug) << "****** SSolver ready solve";
    BOOST_LOG_TRIVIAL(debug) << "SMCompute STEP     = " <<     _itt;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve RA = " <<     _solver.stellarSolver->getSolution().ra;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve DE = " <<     _solver.stellarSolver->getSolution().dec;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve WI = " <<     _solver.stellarSolver->getSolution().fieldWidth;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve HE = " <<     _solver.stellarSolver->getSolution().fieldHeight;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve OR = " <<     _solver.stellarSolver->getSolution().orientation;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve SC = " <<     _solver.stellarSolver->getSolution().pixscale;
    if (_itt==0) {
        _ra0=_solver.stellarSolver->getSolution().ra;
        _de0=_solver.stellarSolver->getSolution().dec;
    }
    if (_itt==1) {
        _ra1=_solver.stellarSolver->getSolution().ra;
        _de1=_solver.stellarSolver->getSolution().dec;
        //_t0=
    }
    if (_itt==2) {
        _ra2=_solver.stellarSolver->getSolution().ra;
        _de2=_solver.stellarSolver->getSolution().dec;
        //_t0=
    }

    _values = new NumberProperty(_modulename,"Control","root","values","Values",0,0);
    _values->addNumber(new NumberValue("ra0","RA 0"  ,"hint",_ra0,"",0,10000,0));
    _values->addNumber(new NumberValue("de0","DE 0"  ,"hint",_de0,"",0,10000,0));
    _values->addNumber(new NumberValue("t0", "Time 0","hint",_t0 ,"",0,10000,0));
    _values->addNumber(new NumberValue("ra1","RA 1"  ,"hint",_ra1,"",0,10000,0));
    _values->addNumber(new NumberValue("de1","DE 1"  ,"hint",_de1,"",0,10000,0));
    _values->addNumber(new NumberValue("t1", "Time 1","hint",_t1 ,"",0,10000,0));
    _values->addNumber(new NumberValue("ra2","RA 2"  ,"hint",_ra2,"",0,10000,0));
    _values->addNumber(new NumberValue("de2","DE 2"  ,"hint",_de2,"",0,10000,0));
    _values->addNumber(new NumberValue("t2", "Time 2","hint",_t2 ,"",0,10000,0));
    emit propertyCreated(_values,&_modulename);
    _propertyStore.add(_values);

    _itt++;
    if (_itt < 3) emit ComputeDone(); else emit PolarDone();

}
void PolarModule::SMComputeFinal()
{
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal";
}

void PolarModule::SMFindStars()
{
    BOOST_LOG_TRIVIAL(debug) << "SMFindStars";

    sendMessage("SMFindStars");
    _solver.ResetSolver(image->stats,image->m_ImageBuffer);
    connect(&_solver,&Solver::successSolve,this,&PolarModule::OnSucessSEP);
    SSolver::Parameters params=_solver.stellarSolverProfiles[0];
    params.minwidth=0.1*_ccdFov/3600;
    params.maxwidth=1.1*_ccdFov/3600;
    _solver.stars.clear();
    _solver.SolveStars(params);
}
void PolarModule::OnSucessSEP()
{
    BOOST_LOG_TRIVIAL(debug) << "OnSucessSEP";

    sendMessage("SEP finished");
    disconnect(&_solver,&Solver::successSolve,this,&PolarModule::OnSucessSEP);
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
