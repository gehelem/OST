#include "guider.h"
//#include "polynomialfit.h"
#define PI 3.14159265

GuiderModule *initialize(QString name,QString label,QString profile,QVariantMap availableModuleLibs)
{
    GuiderModule *basemodule = new GuiderModule(name,label,profile,availableModuleLibs);
    return basemodule;
}
GuiderModule::GuiderModule(QString name,QString label,QString profile,QVariantMap availableModuleLibs)
    : IndiModule(name,label,profile,availableModuleLibs)
{
    _moduletype="guider";

    loadPropertiesFromFile(":guider.json");

    setOstProperty("moduleDescription","Guider",true);
    setOstProperty("moduleVersion",0.1,true);
    setOstProperty("moduleType",_moduletype,true);

    createOstElement("devices","camera","Camera",true);
    createOstElement("devices","mount","Mount",true);
    setOstElement("devices","camera",   _camera,false);
    setOstElement("devices","mount",    _mount,true);


//    _grid = new GridProperty(_modulename,"Control","root","grid","Grid property label",0,0,"PXY","Set","DX","DY","","");
//    emit propertyCreated(_grid,&_modulename);
//    _propertyStore.add(_grid);
//
//    _gridguide = new GridProperty(_modulename,"Control","root","gridguide","Grid property label",0,0,"PHD","Time","RA","DE","CRA","CDE");
//    emit propertyCreated(_gridguide,&_modulename);
//    _propertyStore.add(_gridguide);
//
//    _states = new LightProperty(_modulename,"Control","root","states","State",0,0);
//    _states->addLight(new LightValue("idle"  ,"Idle","hint",1));
//    _states->addLight(new LightValue("cal"   ,"Calibrating","hint",0));
//    _states->addLight(new LightValue("guide" ,"Guiding","hint",0));
//    _states->addLight(new LightValue("error" ,"Error","hint",0));
//    emit propertyCreated(_states,&_modulename);
//    _propertyStore.add(_states);

    buildInitStateMachines();
    buildCalStateMachines();
    buildGuideStateMachines();



}

GuiderModule::~GuiderModule()
{

}
void GuiderModule::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{
        //BOOST_LOG_TRIVIAL(debug) << "OnMyExternalEvent - recv : " << getName().toStdString() << "-" << eventType.toStdString() << "-" << eventKey.toStdString();
    if (getName()==eventModule) {
        foreach(const QString& keyprop, eventData.keys()) {
            foreach(const QString& keyelt, eventData[keyprop].toMap()["elements"].toMap().keys()) {
                BOOST_LOG_TRIVIAL(debug) << "OnMyExternalEvent - recv : " << getName().toStdString() << "-" << eventType.toStdString() << "-" << keyprop.toStdString() << "-" << keyelt.toStdString();
                QVariant val=eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                if (keyprop=="parameters") {
                    if (keyelt=="startpos") {
                        if (setOstElement(keyprop,keyelt,val,true)) {
                            //_startpos=val.toInt();
                        }
                    }
//                    if (keyelt=="steps") {
//                        if (setOstElement(keyprop,keyelt,val,true)) {
//                            _steps=val.toInt();
//                        }
//                    }
//                    if (keyelt=="iterations") {
//                        if (setOstElement(keyprop,keyelt,val,true)) {
//                            _iterations=val.toInt();
//                        }
//                    }
//                    if (keyelt=="loopIterations") {
//                        if (setOstElement(keyprop,keyelt,val,true)) {
//                            _loopIterations=val.toInt();
//                        }
//                    }
//                    if (keyelt=="exposure") {
//                        if (setOstElement(keyprop,keyelt,val,true)) {
//                            _exposure=val.toInt();
//                        }
//                    }
//                    if (keyelt=="backlash") {
//                        if (setOstElement(keyprop,keyelt,val,true)) {
//                            _backlash=val.toInt();
//                        }
//                    }
//
                }
//                if (keyprop=="actions") {
//                    if (keyelt=="coarse") {
//                        if (setOstElement(keyprop,keyelt,false,false)) {
//                            setOstPropertyAttribute(keyprop,"status",IPS_BUSY,true);
//                            startCoarse();
//                        }
//                    }
//                    if (keyelt=="abort") {
//                        if (setOstElement(keyprop,keyelt,false,false)) {
//                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
//                            emit abort();
//                        }
//                    }
//                    if (keyelt=="loop") {
//                        if (setOstElement(keyprop,keyelt,false,false)) {
//                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
//                        }
//                    }
//
//                }
//                if (keyprop=="devices") {
//                    if (keyelt=="camera") {
//                        if (setOstElement(keyprop,keyelt,val,false)) {
//                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
//                            _camera=val.toString();
//                        }
//                    }
//                    if (keyelt=="focuser") {
//                        if (setOstElement(keyprop,keyelt,val,false)) {
//                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
//                            _focuser=val.toString();
//                        }
//                    }
//                }
            }
        }
    }
}
//void GuiderModule::OnSetPropertyNumber(NumberProperty* prop)
//{
//    if (!(prop->getModuleName()==_modulename)) return;
//
//    QList<NumberValue*> numbers=prop->getNumbers();
//    for (int j = 0; j < numbers.size(); ++j) {
//        if (numbers[j]->name()=="exposure")        _exposure=numbers[j]->getValue();
//        if (numbers[j]->name()=="pulse")           _pulse=numbers[j]->getValue();
//        if (numbers[j]->name()=="calsteps")        _calSteps=numbers[j]->getValue();
//        if (numbers[j]->name()=="pulsemax")        _pulseMax=numbers[j]->getValue();
//        if (numbers[j]->name()=="pulsemin")        _pulseMin=numbers[j]->getValue();
//        if (numbers[j]->name()=="raAgr")        _raAgr=numbers[j]->getValue();
//        if (numbers[j]->name()=="deAgr")        _deAgr=numbers[j]->getValue();
//        prop->setState(1);
//        emit propertyUpdated(prop,&_modulename);
//        _propertyStore.add(prop);
//        //BOOST_LOG_TRIVIAL(debug) << "Focus number property item modified " << prop->getName().toStdString();
//    }
//
//}
//void GuiderModule::OnSetPropertyText(TextProperty* prop)
//{
//    if (!(prop->getModuleName()==_modulename)) return;
//}
//
//void GuiderModule::OnSetPropertySwitch(SwitchProperty* prop)
//{
//    if (!(prop->getModuleName()==_modulename)) return;
//
//    SwitchProperty* wprop = _propertyStore.getSwitch(prop->getDeviceName(),prop->getGroupName(),prop->getName());
//    QList<SwitchValue*> switchs=prop->getSwitches();
//    for (int j = 0; j < switchs.size(); ++j) {
//        if (switchs[j]->name()=="calibration") {
//            disconnect(&_SMInit,        &QStateMachine::finished,nullptr, nullptr);
//            disconnect(&_SMCalibration, &QStateMachine::finished,nullptr, nullptr);
//            connect(&_SMInit,           &QStateMachine::finished,&_SMCalibration,&QStateMachine::start) ;
//            _SMInit.start();
//            wprop->setSwitch(switchs[j]->name(),true);
//        }
//        if (switchs[j]->name()=="init") {
//            disconnect(&_SMInit,        &QStateMachine::finished,nullptr, nullptr);
//            disconnect(&_SMCalibration, &QStateMachine::finished,nullptr, nullptr);
//            _SMInit.start();
//            wprop->setSwitch(switchs[j]->name(),true);
//        }
//        if (switchs[j]->name()=="guide") {
//            disconnect(&_SMInit,        &QStateMachine::finished,nullptr, nullptr);
//            disconnect(&_SMCalibration, &QStateMachine::finished,nullptr, nullptr);
//            connect(&_SMInit,           &QStateMachine::finished,&_SMGuide,&QStateMachine::start) ;
//            _SMInit.start();
//            wprop->setSwitch(switchs[j]->name(),true);
//        }
//        if (switchs[j]->name()=="calguide") {
//            disconnect(&_SMInit,        &QStateMachine::finished,nullptr, nullptr);
//            disconnect(&_SMCalibration, &QStateMachine::finished,nullptr, nullptr);
//            connect(&_SMInit,           &QStateMachine::finished,&_SMCalibration,&QStateMachine::start) ;
//            connect(&_SMCalibration,    &QStateMachine::finished,&_SMGuide,      &QStateMachine::start) ;
//            _SMInit.start();
//            wprop->setSwitch(switchs[j]->name(),true);
//        }
//        if (switchs[j]->name()=="loadconfs") {
//            wprop->setSwitch(switchs[j]->name(),true);
//            loadDevicesConfs();
//        }
//        if (switchs[j]->name()=="abort")  {
//            wprop->setSwitch(switchs[j]->name(),true);
//            emit Abort();
//        }
//        if (switchs[j]->name()=="condev") {
//            wprop->setSwitch(switchs[j]->name(),true);
//            connectAllDevices();
//        }
//        //prop->setSwitches(switchs);
//        _propertyStore.update(wprop);
//        emit propertyUpdated(wprop,&_modulename);
//    }
//
//}

void GuiderModule::newNumber(INumberVectorProperty *nvp)
{
    if (
            (QString(nvp->device) == _mount) &&
            (QString(nvp->name)   == "TELESCOPE_TIMED_GUIDE_NS") &&
            (nvp->s   == IPS_IDLE)

       )
    {
        _pulseDECfinished=true;
    }

    if (
            (QString(nvp->device) == _mount) &&
            (QString(nvp->name)   == "TELESCOPE_TIMED_GUIDE_WE") &&
            (nvp->s   == IPS_IDLE)

       )
    {
        _pulseRAfinished=true;
    }

    if (
            (QString(nvp->device) == _mount) &&
            ( (QString(nvp->name)   == "TELESCOPE_TIMED_GUIDE_WE") ||
              (QString(nvp->name)   == "TELESCOPE_TIMED_GUIDE_NS") ) &&
            (nvp->s   == IPS_IDLE)

       )
    {
        if (_pulseRAfinished && _pulseDECfinished) emit PulsesDone();
    }

}

void GuiderModule::newBLOB(IBLOB *bp)
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
        //image->saveStretchedToJpeg(_webroot+"/"+QString(bp->bvp->device)+".jpeg",100);
        image->saveToJpeg(_webroot+"/"+QString(bp->bvp->device)+".jpeg",100);

        _img->setURL(QString(bp->bvp->device)+".jpeg");
        emit propertyUpdated(_img,&_modulename);
        _propertyStore.add(_img);
        BOOST_LOG_TRIVIAL(debug) << "Emit Exposure done";
        emit ExposureDone();
    }

}

void GuiderModule::newSwitch(ISwitchVectorProperty *svp)
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
void GuiderModule::buildInitStateMachines(void)
{
    /* Initialization statemachine = SMInit */

    auto *Abort = new QState();
    auto *Init  = new QState();
    auto *End   = new QFinalState();

    auto *InitInit             = new QState(Init);
    auto *RequestFrameReset    = new QState(Init);
    auto *WaitFrameReset       = new QState(Init);
    auto *RequestFirstExposure = new QState(Init);
    auto *WaitFirstExposure    = new QState(Init);
    auto *FindStarsFirst       = new QState(Init);
    auto *ComputeFirst         = new QState(Init);

    connect(InitInit            ,&QState::entered, this, &GuiderModule::SMInitInit);
    connect(RequestFrameReset   ,&QState::entered, this, &GuiderModule::SMRequestFrameReset);
    connect(RequestFirstExposure,&QState::entered, this, &GuiderModule::SMRequestExposure);
    connect(FindStarsFirst      ,&QState::entered, this, &GuiderModule::SMFindStars);
    connect(ComputeFirst        ,&QState::entered, this, &GuiderModule::SMComputeFirst);
    connect(Abort,               &QState::entered, this, &GuiderModule::SMAbort);

    Init->                addTransition(this,&GuiderModule::Abort                ,Abort);
    Abort->               addTransition(this,&GuiderModule::AbortDone            ,End);
    InitInit->            addTransition(this,&GuiderModule::InitDone             ,RequestFrameReset);
    RequestFrameReset->   addTransition(this,&GuiderModule::RequestFrameResetDone,WaitFrameReset);
    WaitFrameReset->      addTransition(this,&GuiderModule::FrameResetDone       ,RequestFirstExposure);
    RequestFirstExposure->addTransition(this,&GuiderModule::RequestExposureDone  ,WaitFirstExposure);
    WaitFirstExposure->   addTransition(this,&GuiderModule::ExposureDone         ,FindStarsFirst);
    FindStarsFirst->      addTransition(this,&GuiderModule::FindStarsDone        ,ComputeFirst);
    ComputeFirst->        addTransition(this,&GuiderModule::ComputeFirstDone     ,End);

    Init->setInitialState(InitInit);

    _SMInit.addState(Init);
    _SMInit.addState(Abort);
    _SMInit.addState(End);
    _SMInit.setInitialState(Init);


}
void GuiderModule::buildCalStateMachines(void)
{

    auto *Abort = new QState();
    auto *Cal  = new QState();
    auto *End   = new QFinalState();

    auto *InitCal             = new QState(Cal);
    auto *RequestCalPulses    = new QState(Cal);
    auto *WaitCalPulses       = new QState(Cal);
    auto *RequestCalExposure  = new QState(Cal);
    auto *WaitCalExposure     = new QState(Cal);
    auto *FindStarsCal        = new QState(Cal);
    auto *ComputeCal          = new QState(Cal);

    connect(InitCal             ,&QState::entered, this, &GuiderModule::SMInitCal);
    connect(RequestCalExposure  ,&QState::entered, this, &GuiderModule::SMRequestExposure);
    connect(FindStarsCal        ,&QState::entered, this, &GuiderModule::SMFindStars);
    connect(ComputeCal          ,&QState::entered, this, &GuiderModule::SMComputeCal);
    connect(RequestCalPulses,    &QState::entered, this, &GuiderModule::SMRequestPulses);
    connect(Abort,               &QState::entered, this, &GuiderModule::SMAbort);

    Cal->                 addTransition(this,&GuiderModule::Abort               ,Abort);
    Abort->               addTransition(this,&GuiderModule::AbortDone           ,End);
    InitCal->             addTransition(this,&GuiderModule::InitCalDone         ,RequestCalPulses);

    RequestCalPulses->    addTransition(this,&GuiderModule::RequestPulsesDone   ,WaitCalPulses);
    WaitCalPulses->       addTransition(this,&GuiderModule::PulsesDone          ,RequestCalExposure);
    RequestCalExposure->  addTransition(this,&GuiderModule::RequestExposureDone ,WaitCalExposure);
    WaitCalExposure->     addTransition(this,&GuiderModule::ExposureDone        ,FindStarsCal);
    FindStarsCal->        addTransition(this,&GuiderModule::FindStarsDone       ,ComputeCal);
    ComputeCal->          addTransition(this,&GuiderModule::ComputeCalDone      ,RequestCalPulses);
    ComputeCal->          addTransition(this,&GuiderModule::CalibrationDone     ,End);


    Cal->setInitialState(InitCal);

    _SMCalibration.addState(Cal);
    _SMCalibration.addState(Abort);
    _SMCalibration.addState(End);
    _SMCalibration.setInitialState(Cal);


}
void GuiderModule::buildGuideStateMachines(void)
{

    auto *Abort = new QState();
    auto *Guide  = new QState();
    auto *End   = new QFinalState();

    auto *InitGuide           = new QState(Guide);
    auto *RequestGuideExposure= new QState(Guide);
    auto *WaitGuideExposure   = new QState(Guide);
    auto *FindStarsGuide      = new QState(Guide);
    auto *ComputeGuide        = new QState(Guide);
    auto *RequestGuidePulses  = new QState(Guide);
    auto *WaitGuidePulses     = new QState(Guide);

    connect(InitGuide           ,&QState::entered, this, &GuiderModule::SMInitGuide);
    connect(RequestGuideExposure,&QState::entered, this, &GuiderModule::SMRequestExposure);
    connect(FindStarsGuide      ,&QState::entered, this, &GuiderModule::SMFindStars);
    connect(ComputeGuide        ,&QState::entered, this, &GuiderModule::SMComputeGuide);
    connect(RequestGuidePulses,  &QState::entered, this, &GuiderModule::SMRequestPulses);
    connect(Abort,               &QState::entered, this, &GuiderModule::SMAbort);

    Guide->               addTransition(this,&GuiderModule::Abort               ,Abort);
    Abort->               addTransition(this,&GuiderModule::AbortDone           ,End);
    InitGuide->           addTransition(this,&GuiderModule::InitGuideDone       ,RequestGuideExposure);

    RequestGuideExposure->  addTransition(this,&GuiderModule::RequestExposureDone ,WaitGuideExposure);
    WaitGuideExposure->     addTransition(this,&GuiderModule::ExposureDone        ,FindStarsGuide);
    FindStarsGuide->        addTransition(this,&GuiderModule::FindStarsDone       ,ComputeGuide);
    ComputeGuide->          addTransition(this,&GuiderModule::ComputeGuideDone    ,RequestGuidePulses);
    RequestGuidePulses->    addTransition(this,&GuiderModule::RequestPulsesDone   ,WaitGuidePulses);
    RequestGuidePulses->    addTransition(this,&GuiderModule::PulsesDone          ,RequestGuideExposure);
    WaitGuidePulses->       addTransition(this,&GuiderModule::PulsesDone          ,RequestGuideExposure);
    //ComputeGuide->          addTransition(this,&GuiderModule::GuideDone           ,End); // useless ??


    Guide->setInitialState(InitGuide);

    _SMGuide.addState(Guide);
    _SMGuide.addState(Abort);
    _SMGuide.addState(End);
    _SMGuide.setInitialState(Guide);


}
void GuiderModule::SMInitInit()
{
    BOOST_LOG_TRIVIAL(debug) << "SMInitInit";
    sendMessage("SMInitInit");
    setBlobMode();
    /* get mount DEC */
    if (!getModNumber(_mount,"EQUATORIAL_EOD_COORD","DEC",_mountDEC)) {
        emit Abort();
        return;
    }
     /* get mount RA */
    if (!getModNumber(_mount,"EQUATORIAL_EOD_COORD","RA",_mountRA)) {
        emit Abort();
        return;
    }
    /* get mount Pier position  */
   if (!getModSwitch(_mount,"TELESCOPE_PIER_SIDE","PIER_WEST",_mountPointingWest)) {
       emit Abort();
       return;
   }
   _gridguide->clear();
   _propertyStore.update(_gridguide);
   emit propertyUpdated(_gridguide,&_modulename);

   _grid->clear();
   _propertyStore.update(_grid);
   emit propertyUpdated(_grid,&_modulename);
    BOOST_LOG_TRIVIAL(debug) << "SMInitInitDone";
    emit InitDone();
}
void GuiderModule::SMInitCal()
{
    BOOST_LOG_TRIVIAL(debug) << "SMInitCal";
    sendMessage("SMInitCal");
    BOOST_LOG_TRIVIAL(debug) << "Guider module - Start calibration";
    _states->addLight(new LightValue("idle"  ,"Idle","hint",0));
    _states->addLight(new LightValue("cal"   ,"Calibrating","hint",2));
    _states->addLight(new LightValue("guide" ,"Guiding","hint",0));
    _states->addLight(new LightValue("error" ,"Error","hint",0));
    emit propertyUpdated(_states,&_modulename);
    _propertyStore.update(_states);

    _calState=0;
    _calStep=0;
    _pulseN = 0;
    _pulseS = 0;
    _pulseE = 0;
    _pulseW = _pulse;
    _trigCurrent.clear();
    _trigPrev=_trigFirst;
    _dxvector.clear();
    _dyvector.clear();
    _coefficients.clear();
    _itt=0;
    _pulseDECfinished = true;
    _pulseRAfinished = true;

    BOOST_LOG_TRIVIAL(debug) << "SMInitCalDone";
    emit InitCalDone();
}
void GuiderModule::SMInitGuide()
{
    BOOST_LOG_TRIVIAL(debug) << "SMInitGuide";
    sendMessage("SMInitGuide");

    BOOST_LOG_TRIVIAL(debug) << "************************************************************";
    BOOST_LOG_TRIVIAL(debug) << "************************************************************";
    BOOST_LOG_TRIVIAL(debug) << "Guider module - Start guide with fllowing calibration data : ";
    BOOST_LOG_TRIVIAL(debug) << "*********************** cal CCD Orientation "<< _calCcdOrientation*180/PI;
    BOOST_LOG_TRIVIAL(debug) << "*********************** cal moutn pointing west  " << _calMountPointingWest;
    BOOST_LOG_TRIVIAL(debug) << "*********************** cal W "<< _calPulseW;
    BOOST_LOG_TRIVIAL(debug) << "*********************** cal E "<< _calPulseE;
    BOOST_LOG_TRIVIAL(debug) << "*********************** cal N "<< _calPulseN;
    BOOST_LOG_TRIVIAL(debug) << "*********************** cal S "<< _calPulseS;
    BOOST_LOG_TRIVIAL(debug) << "************************************************************";
    BOOST_LOG_TRIVIAL(debug) << "************************************************************";
    _states->addLight(new LightValue("idle"  ,"Idle","hint",0));
    _states->addLight(new LightValue("cal"   ,"Calibrating","hint",0));
    _states->addLight(new LightValue("guide" ,"Guiding","hint",2));
    _states->addLight(new LightValue("error" ,"Error","hint",0));
    emit propertyUpdated(_states,&_modulename);
    _propertyStore.update(_states);

    _gridguide->clear();
    _propertyStore.update(_gridguide);
    emit propertyUpdated(_gridguide,&_modulename);

    _grid->clear();
    _propertyStore.update(_grid);
    emit propertyUpdated(_grid,&_modulename);

    BOOST_LOG_TRIVIAL(debug) << "SMInitGuideDone";
    emit InitGuideDone();
}
void GuiderModule::SMRequestFrameReset()
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


void GuiderModule::SMRequestExposure()
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
void GuiderModule::SMComputeFirst()
{
    BOOST_LOG_TRIVIAL(debug) << "SMComputeFirst";
    _trigFirst.clear();
    buildIndexes(_solver,_trigFirst);
    BOOST_LOG_TRIVIAL(debug) << "******************************************************************** ";
    BOOST_LOG_TRIVIAL(debug) << "******************************************************************** ";
    BOOST_LOG_TRIVIAL(debug) << "************ Guider module - initialization : ";
    BOOST_LOG_TRIVIAL(debug) << "*********************** mount pointing west true/false  " << _mountPointingWest;
    BOOST_LOG_TRIVIAL(debug) << "*********************** actual RA  "<< _mountRA;
    BOOST_LOG_TRIVIAL(debug) << "*********************** actual DEC "<< _mountDEC;
    BOOST_LOG_TRIVIAL(debug) << "******************************************************************** ";
    BOOST_LOG_TRIVIAL(debug) << "******************************************************************** ";


    emit ComputeFirstDone();
}
void GuiderModule::SMComputeCal()
{
    BOOST_LOG_TRIVIAL(debug) << "SMComputeCal";
    buildIndexes(_solver,_trigCurrent);
    _ccdOrientation=0;

    double coeff[2];
    if (_trigCurrent.size()>0) {
        matchIndexes(_trigPrev,_trigCurrent,_matchedCurPrev,_dxPrev,_dyPrev);
        matchIndexes(_trigFirst,_trigCurrent,_matchedCurFirst,_dxFirst,_dyFirst);
        _grid->append(_dxFirst,_dyFirst);
        _propertyStore.update(_grid);
        emit propertyAppended(_grid,&_modulename,0,_dxFirst,_dyFirst,0,0);
        BOOST_LOG_TRIVIAL(debug) << "DX DY // first =  " << _dxFirst << "-" << _dyFirst;
        _dxvector.push_back(_dxPrev);
        _dyvector.push_back(_dyPrev);
        /*if (_dxvector.size() > 1)
        {
            polynomialfit(_dxvector.size(), 2, _dxvector.data(), _dyvector.data(), coeff);
            BOOST_LOG_TRIVIAL(debug) << "Coeffs " << coeff[0] << "-" <<  coeff[1] << " CCD Orientation = " << atan(coeff[1])*180/PI;
        }*/


    } else {
      BOOST_LOG_TRIVIAL(debug) << "houston, we have a problem";
    }
    BOOST_LOG_TRIVIAL(debug) << "Drifts // prev " << sqrt(square(_dxPrev)+square(_dyPrev));
    _trigPrev=_trigCurrent;

    /*if (_calState==0) {
        BOOST_LOG_TRIVIAL(debug) << "RA drift " << sqrt(square(_avdx)+square(_avdy)) << " drift / ms = " << 1000*sqrt(square(_avdx)+square(_avdy))/_pulseWTot;
    }
    if (_calState==2) {
        BOOST_LOG_TRIVIAL(debug) << "DEC drift " << sqrt(square(_avdx)+square(_avdy)) << " drift / ms = " << 1000*sqrt(square(_avdx)+square(_avdy))/_pulseNTot;
    }*/
    _pulseN=0;
    _pulseS=0;
    _pulseE=0;
    _pulseW=0;
    _calStep++;
    if (_calStep >= _calSteps) {
        double ddx=0;
        double ddy=0;
        for (int i=1;i<_dxvector.size();i++) {
            ddx=ddx+_dxvector[i];
            ddy=ddy+_dyvector[i];
        }
        ddx=ddx/(_dxvector.size());
        ddy=ddy/(_dyvector.size());
        double a=atan(ddy/ddx);
        BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" DX drift " <<  ddx;
        BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" DY drift " <<  ddy;
        BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" Total drift " <<  sqrt(square(ddy)+ square(ddy));
        if (_calState==0) {
            _calPulseW=_pulse/ sqrt(square(ddy)+ square(ddy));
            _ccdOrientation=a;
            _calMountPointingWest=_mountPointingWest;
            _calCcdOrientation=_ccdOrientation;

            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" Drift orientation =  " << a*180/PI;
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" W drift (px) " <<  sqrt(square(ddy)+ square(ddy));
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" W drift ms/px " << _calPulseW;
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" W drift ms/'' " << (_pulse)/ (sqrt(square(ddy)+ square(ddy))*_ccdSampling);
        }
        if (_calState==1) {
            _calPulseE=_pulse/ sqrt(square(ddy)+ square(ddy));
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" Drift orientation =  " << a*180/PI;
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" E drift (px) " <<  sqrt(square(ddy)+ square(ddy));
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" E drift ms/px " << _calPulseE;
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" E drift ms/'' " << (_pulse)/ (sqrt(square(ddy)+ square(ddy))*_ccdSampling);
        }
        if (_calState==2) {
            _calPulseN=_pulse/ sqrt(square(ddy)+ square(ddy));
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" Drift orientation =  " << a*180/PI;
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" N drift (px) " <<  sqrt(square(ddy)+ square(ddy));
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" N drift ms/px " << _calPulseN;
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" N drift ms/'' " << (_pulse)/ (sqrt(square(ddy)+ square(ddy))*_ccdSampling);
        }
        if (_calState==3) {
            _calPulseS=_pulse/ sqrt(square(ddy)+ square(ddy));
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" Drift orientation =  " << a*180/PI;
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" S drift (px) " <<  sqrt(square(ddy)+ square(ddy));
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" S drift ms/px " << _calPulseS;
            BOOST_LOG_TRIVIAL(debug) << "*********************** step "<< _calState <<" S drift ms/'' " << (_pulse)/ (sqrt(square(ddy)+ square(ddy))*_ccdSampling);
        }

        _calStep=0;
        _calState++;
        //if (_calState==2) {
            _dxvector.clear();
            _dyvector.clear();
            _coefficients.clear();
        //}
        if (_calState>=4) {
            BOOST_LOG_TRIVIAL(debug) << "*********************** cal W "<< _calPulseW;
            BOOST_LOG_TRIVIAL(debug) << "*********************** cal E "<< _calPulseE;
            BOOST_LOG_TRIVIAL(debug) << "*********************** cal N "<< _calPulseN;
            BOOST_LOG_TRIVIAL(debug) << "*********************** cal S "<< _calPulseS;

            //emit abort(); return;
            emit CalibrationDone();
            _trigFirst=_trigCurrent;
            return;
        }
    }
    if (_calState==0) {_pulseW=_pulse;}
    if (_calState==1) {_pulseE=_pulse;}
    if (_calState==2) {_pulseN=_pulse;}
    if (_calState==3) {_pulseS=_pulse;}

    emit ComputeCalDone();
}
void GuiderModule::SMComputeGuide()
{
    _states->addLight(new LightValue("idle"  ,"Idle","hint",0));
    _states->addLight(new LightValue("cal"   ,"Calibrating","hint",1));
    _states->addLight(new LightValue("guide" ,"Guiding","hint",2));
    _states->addLight(new LightValue("error" ,"Error","hint",0));
    emit propertyUpdated(_states,&_modulename);
    _propertyStore.update(_states);

    BOOST_LOG_TRIVIAL(debug) << "SMComputeGuide " << _solver.stars.size();
    _pulseW = 0;
    _pulseE = 0;
    _pulseN = 0;
    _pulseS = 0;
    buildIndexes(_solver,_trigCurrent);

    BOOST_LOG_TRIVIAL(debug) << "Trig current size " << _trigCurrent.size();
    if (_trigCurrent.size()>0) {
        matchIndexes(_trigFirst,_trigCurrent,_matchedCurFirst,_dxFirst,_dyFirst);
        _grid->append(_dxFirst,_dyFirst);
        _propertyStore.update(_grid);
        emit propertyAppended(_grid,&_modulename,0,_dxFirst,_dyFirst,0,0);
    }
    double _driftRA=  _dxFirst*cos(_calCcdOrientation)+_dyFirst*sin(_calCcdOrientation);
    double _driftDE= -_dxFirst*sin(_calCcdOrientation)+_dyFirst*cos(_calCcdOrientation);
    BOOST_LOG_TRIVIAL(debug) << "*********************** guide  RA drift (px) " << _driftRA;
    BOOST_LOG_TRIVIAL(debug) << "*********************** guide  DE drift (px) " << _driftDE;
    if (_driftRA > 0 ) {
        _pulseW = _raAgr*_driftRA*_calPulseW;
        if (_pulseW > _pulseMax) _pulseW=_pulseMax;
        if (_pulseW < _pulseMin) _pulseW=0;
    } else _pulseW=0;
    if (_pulseW>0) BOOST_LOG_TRIVIAL(debug) << "*********************** guide  W pulse " << _pulseW;

    if (_driftRA < 0 ) {
        _pulseE = -_raAgr*_driftRA*_calPulseE;
        if (_pulseE > _pulseMax) _pulseE=_pulseMax;
        if (_pulseE < _pulseMin) _pulseE=0;
    } else _pulseE=0;
    if (_pulseE>0) BOOST_LOG_TRIVIAL(debug) << "*********************** guide  E pulse " << _pulseE;

    if (_driftDE > 0 ) {
        _pulseS = _deAgr*_driftDE*_calPulseS;
        if (_pulseS > _pulseMax) _pulseS=_pulseMax;
        if (_pulseS < _pulseMin) _pulseS=0;
    } else _pulseS=0;
    if (_pulseS>0) BOOST_LOG_TRIVIAL(debug) << "*********************** guide  S pulse " << _pulseS;

    if (_driftDE < 0 ) {
        _pulseN = -_deAgr*_driftDE*_calPulseN;
        if (_pulseN > _pulseMax) _pulseN=_pulseMax;
        if (_pulseN < _pulseMin) _pulseN=0;
    } else _pulseN=0;
    if (_pulseN>0) BOOST_LOG_TRIVIAL(debug) << "*********************** guide  N pulse " << _pulseN;

    _gridguide->append(_itt,_driftRA,_driftDE,_pulseW-_pulseE,_pulseN-_pulseS);
    _propertyStore.update(_gridguide);
    emit propertyAppended(_gridguide,&_modulename,_itt,_driftRA,_driftDE,_pulseW-_pulseE,_pulseN-_pulseS);
    _itt++;

    _values->addNumber(new NumberValue("pulseN","Pulse N","hint",_pulseN,"",0,10000,0));
    _values->addNumber(new NumberValue("pulseS","Pulse S","hint",_pulseS,"",0,10000,0));
    _values->addNumber(new NumberValue("pulseE","Pulse E","hint",_pulseE,"",0,10000,0));
    _values->addNumber(new NumberValue("pulseW","Pulse W","hint",_pulseW,"",0,10000,0));
    emit propertyCreated(_values,&_modulename);
    _propertyStore.add(_values);

    emit ComputeGuideDone();
}
void GuiderModule::SMRequestPulses()
{
    BOOST_LOG_TRIVIAL(debug) << "SMRequestPulses";

    sendMessage("SMRequestPulses");

    if (_pulseN>0) {
        BOOST_LOG_TRIVIAL(debug) << "********* Pulse  N " << _pulseN;
        _pulseDECfinished = false;
        if (!sendModNewNumber(_mount,"TELESCOPE_TIMED_GUIDE_NS","TIMED_GUIDE_N", _pulseN))
        {        emit abort();        return;    }
    }

    if (_pulseS>0) {
        _pulseDECfinished = false;
        BOOST_LOG_TRIVIAL(debug) << "********* Pulse  S " << _pulseS;
        if (!sendModNewNumber(_mount,"TELESCOPE_TIMED_GUIDE_NS","TIMED_GUIDE_S", _pulseS))
        {        emit abort();        return;    }
    }

    if (_pulseE>0) {
        _pulseRAfinished = false;
        BOOST_LOG_TRIVIAL(debug) << "********* Pulse  E " << _pulseE;
        if (!sendModNewNumber(_mount,"TELESCOPE_TIMED_GUIDE_WE","TIMED_GUIDE_E", _pulseE))
        {        emit abort();        return;    }
    }

    if (_pulseW>0) {
        _pulseRAfinished = false;
        BOOST_LOG_TRIVIAL(debug) << "********* Pulse  W " << _pulseW;
        if (!sendModNewNumber(_mount,"TELESCOPE_TIMED_GUIDE_WE","TIMED_GUIDE_W", _pulseW))
        {        emit abort();        return;    }
    }

    BOOST_LOG_TRIVIAL(debug) << "SMRequestPulses before";
    emit RequestPulsesDone();
    BOOST_LOG_TRIVIAL(debug) << "SMRequestPulses after";

    if ((_pulseN==0)&&(_pulseS==0)&&(_pulseE==0)&&(_pulseW==0)) {
        BOOST_LOG_TRIVIAL(debug) << "SMRequestPulses zéro";
        emit PulsesDone();
    }

}

void GuiderModule::SMFindStars()
{
    BOOST_LOG_TRIVIAL(debug) << "SMFindStars";

    sendMessage("SMFindStars");
    stats=_image->getStats();
    _solver.ResetSolver(stats,_image->getImageBuffer());
    connect(&_solver,&Solver::successSEP,this,&GuiderModule::OnSucessSEP);
    _solver.stars.clear();
    _solver.FindStars(_solver.stellarSolverProfiles[0]);
}

void GuiderModule::OnSucessSEP()
{
    BOOST_LOG_TRIVIAL(debug) << "OnSucessSEP";

    sendMessage("SEP finished");
    disconnect(&_solver,&Solver::successSEP,this,&GuiderModule::OnSucessSEP);
    BOOST_LOG_TRIVIAL(debug) << "********* SEP Finished";
    emit FindStarsDone();
}

void GuiderModule::SMAbort()
{

    disconnect(&_SMInit,        &QStateMachine::finished,nullptr, nullptr);
    disconnect(&_SMCalibration, &QStateMachine::finished,nullptr, nullptr);
    _SMInit.stop();
    _SMCalibration.stop();
    _SMGuide.stop();

    emit AbortDone();

    _states->addLight(new LightValue("idle"  ,"Idle","hint",1));
    _states->addLight(new LightValue("cal"   ,"Calibrating","hint",0));
    _states->addLight(new LightValue("guide" ,"Guiding","hint",0));
    _states->addLight(new LightValue("error" ,"Error","hint",0));
    emit propertyUpdated(_states,&_modulename);
    _propertyStore.update(_states);


}

void GuiderModule::matchIndexes(QVector<Trig> ref, QVector<Trig> act, QVector<MatchedPair>& pairs, double& dx, double& dy)
{
    pairs.clear();

    foreach (Trig r, ref) {
        foreach (Trig a, act) {
            if (
                    (r.s< a.s*1.001 ) && (r.s> a.s*0.999 ) && (r.p< a.p*1.001 ) && (r.p> a.p*0.999 )
                 && (r.d12 < a.d12*1.001) && (r.d12 > a.d12*0.999)
                 && (r.d13 < a.d13*1.001) && (r.d13 > a.d13*0.999)
                 && (r.d23 < a.d23*1.001) && (r.d23 > a.d23*0.999)
               )
            {
                //BOOST_LOG_TRIVIAL(debug) << "Matching " << r.ratio <<  " / " << a.ratio << " xr-yr=" << r.x1 << "-" << r.y1 << " xa-ya=" << a.x1 << "-" << a.y1 << " / dx1 =" << r.x1-a.x1 << " / dy1 =" << r.y1-a.y1 << " / dx2 =" << r.x2-a.x2 << " / dy2 =" << r.y2-a.y2 << " / dx3 =" << r.x3-a.x3 << " / dy3 =" << r.y3-a.y3;
                //BOOST_LOG_TRIVIAL(debug) << "Matching " << r.x1 << " - " << r.d12 << " / " << a.d12 << " - " << r.d13 << " / " << a.d13 << " - " << r.d23 << " / " << a.d23;
                //BOOST_LOG_TRIVIAL(debug) << "Matching dxy1= " << r.x1-a.x1 << "/" << r.y1-a.y1 << " - dxy2="  << r.x2-a.x2 << "/" << r.y2-a.y2 << " - dxy3="  << r.x3-a.x3 << "/" << r.y3-a.y3;
                bool found;
                found= false;
                foreach (MatchedPair pair, pairs) {
                    if ( (pair.xr==r.x1)&&(pair.yr==r.y1) ) found=true;
                }
                if (!found) pairs.append({r.x1,r.y1,a.x1,a.y1,r.x1-a.x1,r.y1-a.y1});
                found= false;
                foreach (MatchedPair pair, pairs) {
                    if ( (pair.xr==r.x2)&&(pair.yr==r.y2) ) found=true;
                }
                if (!found) pairs.append({r.x2,r.y2,a.x2,a.y2,r.x2-a.x2,r.y2-a.y2});
                found= false;
                foreach (MatchedPair pair, pairs) {
                    if ( (pair.xr==r.x3)&&(pair.yr==r.y3) ) found=true;
                }
                if (!found) pairs.append({r.x3,r.y3,a.x3,a.y3,r.x3-a.x3,r.y3-a.y3});
            }
        }
    }
    dx=0;
    dy=0;
    for (int i=0 ; i <pairs.size();i++ ) {
        dx=dx+pairs[i].dx;
        dy=dy+pairs[i].dy;
    }
    dx=dx/pairs.size();
    dy=dy/pairs.size();



    /*foreach (MatchedPair pair, _matchedPairs) {
            BOOST_LOG_TRIVIAL(debug) << "Matched pair =  " << pair.dx << "-" << pair.dy;
    }*/

}
void GuiderModule::buildIndexes(Solver& solver, QVector<Trig>& trig)
{
    int nb = solver.stars.size();
    if (nb > 10) nb = 10;
    trig.clear();

    for (int i=0;i<nb;i++)
    {
        for (int j=i+1;j<nb;j++)
        {
            for (int k=j+1;k<nb;k++)
            {
                double dij,dik,djk,p,s;
                dij=sqrt(square(solver.stars[i].x-solver.stars[j].x)+square(solver.stars[i].y-solver.stars[j].y));
                dik=sqrt(square(solver.stars[i].x-solver.stars[k].x)+square(solver.stars[i].y-solver.stars[k].y));
                djk=sqrt(square(solver.stars[j].x-solver.stars[k].x)+square(solver.stars[j].y-solver.stars[k].y));
                p=dij+dik+djk;
                s=sqrt(p*(p-dij)*(p-dik)*(p-djk));
                //BOOST_LOG_TRIVIAL(debug) << "Trig CURRENT" << " - " << i << " - " << j << " - " << k << " - p=  " << p << " - s=" << s << " - s/p=" << s/p;
                trig.append({
                                 solver.stars[i].x,
                                 solver.stars[i].y,
                                 solver.stars[j].x,
                                 solver.stars[j].y,
                                 solver.stars[k].x,
                                 solver.stars[k].y,
                                 dij,dik,djk,
                                 p,s,s/p
                                });

            }
        }

    }

}
