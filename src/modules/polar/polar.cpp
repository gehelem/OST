#include "polar.h"
#include <libastro.h>
#include <libnova/libnova.h>

#define PI 3.14159265
#define J2000          2451545.0    //Julian Date for noon on Jan 1, 2000 (epoch J2000)

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
    _actions->addSwitch(new SwitchValue("test","Test","hint",0));
    emit propertyCreated(_actions,&_modulename);
    _propertyStore.add(_actions);

    _commonParams = new NumberProperty(_modulename,"Control","root","commonParams","Parameters",2,0);
    _commonParams->addNumber(new NumberValue("exposure"      ,"Exposure s","hint",_exposure,"",0,5,1));
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

    _errors = new NumberProperty(_modulename,"Control","root","errors","Polar error",0,0);
    _errors->addNumber(new NumberValue("erraz","Azimuth error °"  ,"hint",_erraz,"",0,10000,0));
    _errors->addNumber(new NumberValue("erralt","Altitude error °"  ,"hint",_erralt,"",0,10000,0));
    _errors->addNumber(new NumberValue("errtot","Total error °"  ,"hint",_errtot,"",0,10000,0));
    emit propertyCreated(_errors,&_modulename);
    _propertyStore.add(_errors);

    _img = new ImageProperty(_modulename,"Control","root","viewer","Image property label",0,0,0);
    emit propertyCreated(_img,&_modulename);
    _propertyStore.add(_img);

    _states = new LightProperty(_modulename,"Control","root","states","State",0,0);
    _states->addLight(new LightValue("idle"  ,"Idle","hint",1));
    _states->addLight(new LightValue("Moving"  ,"Moving","hint",0));
    _states->addLight(new LightValue("Shooting"  ,"Shooting","hint",0));
    _states->addLight(new LightValue("Solving"  ,"Solving","hint",0));
    _states->addLight(new LightValue("Compute"  ,"Compute","hint",0));
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
        if (switchs[j]->name()=="test") {
            wprop->setSwitch(switchs[j]->name(),true);
            _ra0=330.69107; _de0=1.26547; _t0=0;
            _ra1=341.20879; _de1=1.25555; _t1=0;
            _ra2=351.72867; _de2=1.25023; _t2=0;
            SMComputeFinal();
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
        //_mountRA=nvp->np[0].value; // RA
        //_mountDEC=nvp->np[1].value; // DEC
        emit MoveDone();
    }

}
void PolarModule::newBLOB(IBLOB *bp)
{
    if (
            (QString(bp->bvp->device) == _camera) && (_machine.isRunning())
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
    connect(FinalCompute        ,&QState::entered, this, &PolarModule::SMComputeFinal);
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
    FinalCompute->        addTransition(this,&PolarModule::ComputeFinalDone     ,End);

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

   _erraz=0;_erralt=0;_errtot=0;
   _errors = new NumberProperty(_modulename,"Control","root","errors","Polar error",0,0);
   _errors->addNumber(new NumberValue("erraz","Azimuth error °"  ,"hint",_erraz,"",0,10000,0));
   _errors->addNumber(new NumberValue("erralt","Altitude error °"  ,"hint",_erralt,"",0,10000,0));
   _errors->addNumber(new NumberValue("errtot","Total error °"  ,"hint",_errtot,"",0,10000,0));
   emit propertyCreated(_errors,&_modulename);
   _propertyStore.add(_errors);

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
    _states = new LightProperty(_modulename,"Control","root","states","State",0,0);
    _states->addLight(new LightValue("idle"  ,"Idle","hint",0));
    _states->addLight(new LightValue("Moving"  ,"Moving","hint",0));
    _states->addLight(new LightValue("Shooting"  ,"Shooting","hint",2));
    _states->addLight(new LightValue("Solving"  ,"Solving","hint",0));
    _states->addLight(new LightValue("Compute"  ,"Compute","hint",0));
    emit propertyCreated(_states,&_modulename);
    _propertyStore.add(_states);

    BOOST_LOG_TRIVIAL(debug) << "SMRequestExposure";
    //double t = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    _t = ln_get_julian_from_sys();

    if (_itt==0) {
        _t0= _t;
    }
    if (_itt==1) {
        _t1= _t;
    }
    if (_itt==2) {
        _t2= _t;
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
    _states = new LightProperty(_modulename,"Control","root","states","State",0,0);
    _states->addLight(new LightValue("idle"  ,"Idle","hint",0));
    _states->addLight(new LightValue("Moving"  ,"Moving","hint",2));
    _states->addLight(new LightValue("Shooting"  ,"Shooting","hint",0));
    _states->addLight(new LightValue("Solving"  ,"Solving","hint",0));
    _states->addLight(new LightValue("Compute"  ,"Compute","hint",0));
    emit propertyCreated(_states,&_modulename);
    _propertyStore.add(_states);

    BOOST_LOG_TRIVIAL(debug) << "SMRequestMove";
    sendMessage("SMRequestMove");

    double oldRA,newRA;

    /* get mount RA */
    if (!getModNumber(_mount,"EQUATORIAL_EOD_COORD","RA",oldRA)) {
        BOOST_LOG_TRIVIAL(debug) << "SMRequestMove error 1";
        emit Abort();
        return;
    }


    if (_mountPointingWest) {
        newRA=oldRA-0.3333333;
        if (newRA < 0) newRA=newRA+24;
    } else {
        newRA=oldRA+0.3333333;
        if (newRA >= 24) newRA=newRA-24;
    }
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

    _states = new LightProperty(_modulename,"Control","root","states","State",0,0);
    _states->addLight(new LightValue("idle"  ,"Idle","hint",0));
    _states->addLight(new LightValue("Moving"  ,"Moving","hint",0));
    _states->addLight(new LightValue("Shooting"  ,"Shooting","hint",0));
    _states->addLight(new LightValue("Solving"  ,"Solving","hint",0));
    _states->addLight(new LightValue("Compute"  ,"Compute","hint",2));
    emit propertyCreated(_states,&_modulename);
    _propertyStore.add(_states);
    _ra=_solver.stellarSolver->getSolution().ra;
    _de=_solver.stellarSolver->getSolution().dec;
    BOOST_LOG_TRIVIAL(debug) << "*******  SMCompute ******** ";
    BOOST_LOG_TRIVIAL(debug) << "****** SSolver ready solve";
    BOOST_LOG_TRIVIAL(debug) << "SMCompute STEP     = " <<     _itt;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve RA = " <<     _ra;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve DE = " <<     _de;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve WI = " <<     _solver.stellarSolver->getSolution().fieldWidth;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve HE = " <<     _solver.stellarSolver->getSolution().fieldHeight;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve OR = " <<     _solver.stellarSolver->getSolution().orientation;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve SC = " <<     _solver.stellarSolver->getSolution().pixscale;
    //BOOST_LOG_TRIVIAL(debug) << "SMCompute solve WCS RA = " <<     _solver.stellarSolver->getWCSCoord()->ra;
    //BOOST_LOG_TRIVIAL(debug) << "SMCompute solve WCS DE = " <<     _solver.stellarSolver->getWCSCoord()->dec;
    //INDI::IEquatorialCoordinates coord2000,coordNow;
    //coordNow.rightascension=_ra*24/360;
    //coordNow.declination=_de;
    //INDI::ObservedToJ2000(&coordNow,_t,&coord2000);
    //_ra_conv=coord2000.rightascension*360/24;
    //_de_conv=coord2000.declination;
    struct ln_equ_posn j2000,jnow;
    j2000.ra  = _ra;
    j2000.dec = _de;
    ln_get_equ_prec2(&j2000, JD2000, _t, &jnow);
    _ra_conv=jnow.ra;
    _de_conv=jnow.dec;

    if (_itt==0) {
        _ra0=_ra;
        _de0=_de;
        _ra0_conv=_ra_conv;
        _de0_conv=_de_conv;
    }
    if (_itt==1) {
        _ra1=_ra;
        _de1=_de;
        _ra1_conv=_ra_conv;
        _de1_conv=_de_conv;
    }
    if (_itt==2) {
        _ra2=_ra;
        _de2=_de;
        _ra2_conv=_ra_conv;
        _de2_conv=_de_conv;
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


       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal RA 0 = " <<     _ra0_conv;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DE 0 = " <<     _de0_conv;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal t 0  = " <<     _t0;

       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal RA 1 = " <<     _ra1_conv;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DE 1 = " <<     _de1_conv;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal t 1  = " <<     _t1;

       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal RA 2 = " <<     _ra2_conv;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DE 2 = " <<     _de2_conv;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal t 2  = " <<     _t2;
       /*double dra1=360*(_t1-_t0)/(1000*3600*24);
       double dra2=360*(_t2-_t0)/(1000*3600*24);

       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DRA1-0 = " << dra1;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DRA2-0 = " << dra2;*/

       Rotations::V3 p0(Rotations::azAlt2xyz(QPointF(_ra0_conv, _de0_conv)));
       Rotations::V3 p1(Rotations::azAlt2xyz(QPointF(_ra1_conv, _de1_conv)));
       Rotations::V3 p2(Rotations::azAlt2xyz(QPointF(_ra2_conv, _de2_conv)));
       //Rotations::V3 p0(Rotations::haDec2xyz(QPointF(_ra0, _de0),0));
       //Rotations::V3 p1(Rotations::haDec2xyz(QPointF(_ra1, _de1),0));
       //Rotations::V3 p2(Rotations::haDec2xyz(QPointF(_ra2, _de2),0));
       //Rotations::V3 axis = Rotations::getAxis(p0, p1, p2);
       Rotations::V3 axis = Rotations::getAxis(p0, p1);

       if (axis.length() < 0.9)
       {
           // It failed to normalize the vector, something's wrong.
           BOOST_LOG_TRIVIAL(debug) << "Normal vector too short. findAxis failed.";
           emit Abort();
           return;
       }

       // Need to make sure we're pointing to the right pole.
       //if ((northernHemisphere() && (axis.x() < 0)) || (!northernHemisphere() && axis.x() > 0))
       if (axis.x() < 0)
       {
           axis = Rotations::V3(-axis.x(), -axis.y(), -axis.z());
       }

       BOOST_LOG_TRIVIAL(debug) << "axis.x() " << axis.x();
       BOOST_LOG_TRIVIAL(debug) << "axis.y() " << axis.y();
       BOOST_LOG_TRIVIAL(debug) << "axis.z() " << axis.z();
       QPointF azAlt = Rotations::xyz2azAlt(axis);
       //QPointF haDec = Rotations::xyz2haDec(axis,0);
       //BOOST_LOG_TRIVIAL(debug) << "haDec x "  << haDec.x();
       //BOOST_LOG_TRIVIAL(debug) << "haDec y " << haDec.y();
       BOOST_LOG_TRIVIAL(debug) << "axex "  << (180*(asin(axis.x()))/PI);
       BOOST_LOG_TRIVIAL(debug) << "axey "  << (180*(asin(axis.y()))/PI);
       _erraz=azAlt.x();
       //_erralt=90-azAlt.y();
       _erralt=azAlt.y();
       _errtot=sqrt(square(_erraz) + square(_erralt));
       BOOST_LOG_TRIVIAL(debug) << "az  error " << _erraz;
       BOOST_LOG_TRIVIAL(debug) << "alt error " << _erralt;
       BOOST_LOG_TRIVIAL(debug) << "tot error " << 180*(Rotations::getAngle(axis,Rotations::V3(0.0, 0.0 , 1)))/PI;
       BOOST_LOG_TRIVIAL(debug) << "tot error " << 180*(Rotations::getAngle(axis,Rotations::V3(0.0, 0.0 ,-1)))/PI;

       struct ln_equ_posn j2000,jnow;
       jnow.ra  = azAlt.x();
       jnow.dec = azAlt.y();
       ln_get_equ_prec2(&jnow, _t, JD2000, &j2000);
       BOOST_LOG_TRIVIAL(debug) << "j2000 RA " << j2000.ra;
       BOOST_LOG_TRIVIAL(debug) << "j2000 DE " << j2000.dec;




       _errors = new NumberProperty(_modulename,"Control","root","errors","Polar error",0,0);
       _errors->addNumber(new NumberValue("erraz","Azimuth error °"  ,"hint",_erraz,"",0,10000,0));
       _errors->addNumber(new NumberValue("erralt","Altitude error °"  ,"hint",_erralt,"",0,10000,0));
       _errors->addNumber(new NumberValue("errtot","Total error °"  ,"hint",_errtot,"",0,10000,0));
       emit propertyCreated(_errors,&_modulename);
       _propertyStore.add(_errors);

       _states = new LightProperty(_modulename,"Control","root","states","State",0,0);
       _states->addLight(new LightValue("idle"  ,"Idle","hint",0));
       _states->addLight(new LightValue("Moving"  ,"Moving","hint",0));
       _states->addLight(new LightValue("Shooting"  ,"Shooting","hint",0));
       _states->addLight(new LightValue("Solving"  ,"Solving","hint",0));
       _states->addLight(new LightValue("Compute"  ,"Compute","hint",1));
       emit propertyCreated(_states,&_modulename);
       _propertyStore.add(_states);


       emit ComputeFinalDone();
       return;

}
void PolarModule::SMFindStars()
{
    BOOST_LOG_TRIVIAL(debug) << "SMFindStars";

    _states = new LightProperty(_modulename,"Control","root","states","State",0,0);
    _states->addLight(new LightValue("idle"  ,"Idle","hint",0));
    _states->addLight(new LightValue("Moving"  ,"Moving","hint",0));
    _states->addLight(new LightValue("Shooting"  ,"Shooting","hint",0));
    _states->addLight(new LightValue("Solving"  ,"Solving","hint",2));
    _states->addLight(new LightValue("Compute"  ,"Compute","hint",0));
    emit propertyCreated(_states,&_modulename);
    _propertyStore.add(_states);


    sendMessage("SMFindStars");

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



    _solver.ResetSolver(image->stats,image->m_ImageBuffer);
    connect(&_solver,&Solver::successSolve,this,&PolarModule::OnSucessSEP);
    _solver.stars.clear();
    SSolver::Parameters params=_solver.stellarSolverProfiles[0];
    //_solver.stellarSolver->
    //params.minwidth=0.1*_ccdFov/3600;
    //params.maxwidth=1.1*_ccdFov/3600;
    //params.search_radius=2;
    //BOOST_LOG_TRIVIAL(debug) << "minwidth " << params.minwidth;
    //BOOST_LOG_TRIVIAL(debug) << "maxidth " << params.maxwidth;
    _solver.stellarSolver->setSearchScale(0.1*_ccdFov/3600,1.1*_ccdFov/3600,ScaleUnits::DEG_WIDTH);
    _solver.stellarSolver->setSearchPositionInDegrees(_mountRA*360/24,_mountDEC);
    //params.
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
    emit AbortDone();
    _machine.stop();
    _states->addLight(new LightValue("idle"  ,"Idle","hint",1));
    emit propertyUpdated(_states,&_modulename);
    _propertyStore.update(_states);
}
