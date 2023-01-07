#include "polar.h"
#include <libastro.h>
#include <libnova/julian_day.h>

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
    double t = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    //double t = ln_get_julian_from_sys();

    if (_itt==0) {
        _t0= t;
    }
    if (_itt==1) {
        _t1= t;
    }
    if (_itt==2) {
        _t2= t;
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
        newRA=oldRA-15*24/360;
        if (newRA < 0) newRA=newRA+24;
    } else {
        newRA=oldRA+15*24/360;
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

    BOOST_LOG_TRIVIAL(debug) << "*******  SMCompute ******** ";
    BOOST_LOG_TRIVIAL(debug) << "****** SSolver ready solve";
    BOOST_LOG_TRIVIAL(debug) << "SMCompute STEP     = " <<     _itt;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve RA = " <<     _solver.stellarSolver->getSolution().ra;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve DE = " <<     _solver.stellarSolver->getSolution().dec;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve WI = " <<     _solver.stellarSolver->getSolution().fieldWidth;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve HE = " <<     _solver.stellarSolver->getSolution().fieldHeight;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve OR = " <<     _solver.stellarSolver->getSolution().orientation;
    BOOST_LOG_TRIVIAL(debug) << "SMCompute solve SC = " <<     _solver.stellarSolver->getSolution().pixscale;
    INDI::IEquatorialCoordinates coord2000,coordNow;
    coordNow.rightascension=_solver.stellarSolver->getSolution().ra*24/360;
    coordNow.declination=_solver.stellarSolver->getSolution().dec;

    //coord2000.rightascension=_solver.stellarSolver->getSolution().ra;
    //coord2000.declination=_solver.stellarSolver->getSolution().dec;

    if (_itt==0) {
        //INDI::ObservedToJ2000(&coordNow,_t0,&coord2000);
        //_ra0=coord2000.rightascension;
        //_de0=coord2000.declination;
        _ra0=_solver.stellarSolver->getSolution().ra;
        _de0=_solver.stellarSolver->getSolution().dec;
    }
    if (_itt==1) {
        //INDI::ObservedToJ2000(&coordNow,_t1,&coord2000);
        //_ra1=coord2000.rightascension;
        //_de1=coord2000.declination;
        _ra1=_solver.stellarSolver->getSolution().ra;
        _de1=_solver.stellarSolver->getSolution().dec;
    }
    if (_itt==2) {
        //INDI::ObservedToJ2000(&coordNow,_t2,&coord2000);
        //_ra2=coord2000.rightascension;
        //_de2=coord2000.declination;
        _ra2=_solver.stellarSolver->getSolution().ra;
        _de2=_solver.stellarSolver->getSolution().dec;

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
       //_ra0=354.1265137671062;  _de0=0.2921369570805727;_t0=1643110224331;
       //_ra1=339.12724652172227; _de1=0.300002845425132; _t1=1643110229671;
       //_ra2=324.1279546867718;  _de2=0.315854040156525; _t2=1643110234681;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal RA 0 = " <<     _ra0;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DE 0 = " <<     _de0;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal t 0  = " <<     _t0;

       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal RA 1 = " <<     _ra1;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DE 1 = " <<     _de1;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal t 1  = " <<     _t1;

       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal RA 2 = " <<     _ra2;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DE 2 = " <<     _de2;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal t 2  = " <<     _t2;
       /*double dra1=360*(_t1-_t0)/(1000*3600*24);
       double dra2=360*(_t2-_t0)/(1000*3600*24);

       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DRA1-0 = " << dra1;
       BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DRA2-0 = " << dra2;*/

       Rotations::V3 p0(Rotations::azAlt2xyz(QPointF(_ra0, _de0)));
       Rotations::V3 p1(Rotations::azAlt2xyz(QPointF(_ra1, _de1)));
       Rotations::V3 p2(Rotations::azAlt2xyz(QPointF(_ra2, _de2)));
       //Rotations::V3 p0(Rotations::haDec2xyz(QPointF(_ra0, _de0),0));
       //Rotations::V3 p1(Rotations::haDec2xyz(QPointF(_ra1, _de1),0));
       //Rotations::V3 p2(Rotations::haDec2xyz(QPointF(_ra2, _de2),0));
       Rotations::V3 axis = Rotations::getAxis(p0, p1, p2);

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
       //azimuthCenter = azAlt.x();
       //altitudeCenter = azAlt.y();
       _erraz=azAlt.x();
       _erralt=90-azAlt.y();
       //_erralt=azAlt.y();
       _errtot=sqrt(square(_erraz) + square(_erralt));
       BOOST_LOG_TRIVIAL(debug) << "azimuthCenter "  << _erraz;
       BOOST_LOG_TRIVIAL(debug) << "altitudeCenter " << _erralt;
       BOOST_LOG_TRIVIAL(debug) << "PA error °"       << _errtot;

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
    //params.minwidth=0.1*_ccdFov/3600;
    //params.maxwidth=1.1*_ccdFov/3600;
    //params.search_radius=2;
    //BOOST_LOG_TRIVIAL(debug) << "minwidth " << params.minwidth;
    //BOOST_LOG_TRIVIAL(debug) << "maxidth " << params.maxwidth;
    //_solver.setSearchScale(0.1*_ccdFov/3600,1.1*_ccdFov/3600,ScaleUnits::DEG_WIDTH);
    //_solver.setSearchPositionInDegrees(_mountRA*360/24,_mountDEC);
    _solver.stellarSolver->setSearchPositionInDegrees(_mountRA*360/24,_mountDEC);
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
