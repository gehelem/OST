#include "guider.h"
#include "polynomialfit.h"
#define PI 3.14159265

GuiderModule *initialize(QString name,QString label)
{
    GuiderModule *basemodule = new GuiderModule(name,label);
    return basemodule;
}

GuiderModule::GuiderModule(QString name,QString label)
    : Basemodule(name,label)
{
    _moduledescription="Guider module";

    _actions = new SwitchProperty(_modulename,"Control","root","actions","Actions",2,0,1);
    _actions->addSwitch(new SwitchValue("condev","Connect devices","hint",0));
    _actions->addSwitch(new SwitchValue("loadconfs","Load devices conf","hint",0));
    _actions->addSwitch(new SwitchValue("abort","Abort","hint",0));
    _actions->addSwitch(new SwitchValue("calibration","Calibration","hint",0));
    emit propertyCreated(_actions,&_modulename);
    _propertyStore.add(_actions);

    _parameters = new NumberProperty(_modulename,"Control","root","parameters","Parameters",2,0);
    _parameters->addNumber(new NumberValue("exposure"      ,"Exposure","hint",_exposure,"",0,5,1));
    _parameters->addNumber(new NumberValue("pulse"      ,"Calibration pulse","hint",_pulse,"",0,5000,1));
    emit propertyCreated(_parameters,&_modulename);
    _propertyStore.add(_parameters);

    _values = new NumberProperty(_modulename,"Control","root","values","Values",0,0);
    //_values->addNumber(new NumberValue("xxx","xxx","hint",0,"",0,99,0));
    emit propertyCreated(_values,&_modulename);
    _propertyStore.add(_values);

    _img = new ImageProperty(_modulename,"Control","root","viewer","Image property label",0,0,0);
    emit propertyCreated(_img,&_modulename);
    _propertyStore.add(_img);

    _grid = new GridProperty(_modulename,"Control","root","grid","Grid property label",0,0,"PXY","Set","DX","DY","");
    emit propertyCreated(_grid,&_modulename);
    _propertyStore.add(_grid);


}

GuiderModule::~GuiderModule()
{

}
void GuiderModule::OnSetPropertyNumber(NumberProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
    _propertyStore.add(prop);

    QList<NumberValue*> numbers=prop->getNumbers();
    for (int j = 0; j < numbers.size(); ++j) {
        if (numbers[j]->name()=="exposure")        _exposure=numbers[j]->getValue();
        if (numbers[j]->name()=="pulse")           _pulse=numbers[j]->getValue();

        emit propertyUpdated(prop,&_modulename);
        BOOST_LOG_TRIVIAL(debug) << "Focus number property item modified " << prop->getName().toStdString();
    }

}
void GuiderModule::OnSetPropertyText(TextProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
}

void GuiderModule::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;

    SwitchProperty* wprop = _propertyStore.getSwitch(prop->getDeviceName(),prop->getGroupName(),prop->getName());
    QList<SwitchValue*> switchs=prop->getSwitches();
    for (int j = 0; j < switchs.size(); ++j) {
        if (switchs[j]->name()=="calibration") {
            wprop->setSwitch(switchs[j]->name(),true);
            startCalibration();
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
    }

}

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
        if (_machine.isRunning()) {
            if (_pulseRAfinished && _pulseDECfinished) emit PulsesDone();
        }
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
        if (_machine.isRunning()) {
            BOOST_LOG_TRIVIAL(debug) << "Emit Exposure done";
            emit ExposureDone();
        }
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
        if (_machine.isRunning()) emit FrameResetDone();
    }

}

void GuiderModule::startCalibration()
{
    BOOST_LOG_TRIVIAL(debug) << "Guider module - Start calibration ";

    _grid->clear();
    _propertyStore.update(_grid);
    emit propertyUpdated(_grid,&_modulename);

    auto *init  = new QState();
    auto *calibrate = new QState();
    auto *guide = new QState();
    auto *abort = new QState();

    auto *RequestFrameReset    = new QState(init);
    auto *WaitFrameReset       = new QState(init);
    auto *RequestFirstExposure = new QState(init);
    auto *WaitFirstExposure    = new QState(init);
    auto *FindStarsRef         = new QState(init);
    auto *ComputeRef           = new QState(init);

    auto *RequestCalPulses     = new QState(calibrate);
    auto *WaitCalPulses        = new QState(calibrate);
    auto *RequestCalExposure   = new QState(calibrate);
    auto *WaitCalExposure      = new QState(calibrate);
    auto *FindStarsCal         = new QState(calibrate);
    auto *ComputeCal           = new QState(calibrate);

    auto *RequestGuidePulses     = new QState(guide);
    auto *WaitGuidePulses        = new QState(guide);
    auto *RequestGuideExposure   = new QState(guide);
    auto *WaitGuideExposure      = new QState(guide);
    auto *FindStarsGuide         = new QState(guide);
    auto *ComputeGuide           = new QState(guide);

    init->setInitialState(RequestFrameReset);
    calibrate->setInitialState(RequestCalPulses);
    guide->setInitialState(RequestGuidePulses);

    _machine.addState(init);
    _machine.addState(calibrate);
    _machine.addState(guide);
    _machine.addState(abort);
    _machine.setInitialState(init);

    connect(RequestFrameReset,   &QState::entered, this, &GuiderModule::SMRequestFrameReset);
    connect(RequestFirstExposure,&QState::entered, this, &GuiderModule::SMRequestExposure);
    connect(FindStarsRef        ,&QState::entered, this, &GuiderModule::SMFindStars);
    connect(ComputeRef,          &QState::entered, this, &GuiderModule::SMComputeRef);
    connect(RequestCalExposure,  &QState::entered, this, &GuiderModule::SMRequestExposure);
    connect(RequestCalPulses,    &QState::entered, this, &GuiderModule::SMRequestPulses);
    connect(FindStarsCal,        &QState::entered, this, &GuiderModule::SMFindStars);
    connect(ComputeCal,          &QState::entered, this, &GuiderModule::SMComputeCal);
    connect(RequestGuideExposure,&QState::entered, this, &GuiderModule::SMRequestExposure);
    connect(RequestGuidePulses,  &QState::entered, this, &GuiderModule::SMRequestPulses);
    connect(FindStarsGuide,      &QState::entered, this, &GuiderModule::SMFindStars);
    connect(ComputeGuide,        &QState::entered, this, &GuiderModule::SMComputeGuide);
    connect(abort,               &QState::entered, this, &GuiderModule::SMAbort);

    init->     addTransition(this,&GuiderModule::abort,                abort);
    calibrate->addTransition(this,&GuiderModule::abort,                abort);
    guide->    addTransition(this,&GuiderModule::abort,                abort);

    RequestFrameReset->   addTransition(this,&GuiderModule::RequestFrameResetDone,WaitFrameReset);
    WaitFrameReset->      addTransition(this,&GuiderModule::FrameResetDone       ,RequestFirstExposure);
    RequestFirstExposure->addTransition(this,&GuiderModule::RequestExposureDone  ,WaitFirstExposure);
    WaitFirstExposure->   addTransition(this,&GuiderModule::ExposureDone ,        FindStarsRef);
    FindStarsRef->        addTransition(this,&GuiderModule::FindStarsDone ,       ComputeRef);
    ComputeRef->          addTransition(this,&GuiderModule::ComputeRefDone ,      calibrate);

    RequestCalPulses     ->addTransition(this,&GuiderModule::RequestPulsesDone,WaitCalPulses);
    WaitCalPulses        ->addTransition(this,&GuiderModule::PulsesDone,RequestCalExposure);
    RequestCalExposure   ->addTransition(this,&GuiderModule::RequestExposureDone,WaitCalExposure);
    WaitCalExposure      ->addTransition(this,&GuiderModule::ExposureDone,FindStarsCal);
    FindStarsCal         ->addTransition(this,&GuiderModule::FindStarsDone,ComputeCal);
    ComputeCal           ->addTransition(this,&GuiderModule::ComputeCalDone,RequestCalPulses);
    ComputeCal           ->addTransition(this,&GuiderModule::StartGuiding,guide);

    RequestGuidePulses     ->addTransition(this,&GuiderModule::RequestPulsesDone,WaitGuidePulses);
    WaitGuidePulses        ->addTransition(this,&GuiderModule::PulsesDone,RequestGuideExposure);
    RequestGuideExposure   ->addTransition(this,&GuiderModule::RequestExposureDone,WaitGuideExposure);
    WaitGuideExposure      ->addTransition(this,&GuiderModule::ExposureDone,FindStarsGuide);
    FindStarsGuide         ->addTransition(this,&GuiderModule::FindStarsDone,ComputeGuide);
    ComputeGuide           ->addTransition(this,&GuiderModule::ComputeGuideDone,RequestGuidePulses);

    _calState=0;
    _calStep=0;
    _calSteps=5;
    _pulseN = _pulse;
    _pulseS = _pulse;
    _pulseE = _pulse;
    _pulseW = _pulse;
    _trigRef.clear();
    _trigCurrent.clear();
    _dxvector.clear();
    _dyvector.clear();
    _coefficients.clear();


    _machine.start();
    sendMessage("machine started");
}
void GuiderModule::SMRequestFrameReset()
{
    BOOST_LOG_TRIVIAL(debug) << "SMRequestFrameReset";
    sendMessage("SMRequestFrameReset");
    if (!frameReset(_camera))
    {
            emit abort();
            return;
    }
    emit RequestFrameResetDone();
}


void GuiderModule::SMRequestExposure()
{
    BOOST_LOG_TRIVIAL(debug) << "SMRequestExposure";
    sendMessage("SMRequestExposure");
    if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure))
    {
        emit abort();
        return;
    }
    emit RequestExposureDone();
}
void GuiderModule::SMComputeRef()
{
    BOOST_LOG_TRIVIAL(debug) << "SMComputeRef";
    int nb = _solver.stars.size();
    if (nb > 10) nb = 10;
    _trigCurrent.clear();
    _trigRef.clear();
        for (int i=0;i<nb;i++)
        {
            for (int j=i+1;j<nb;j++)
            {
                for (int k=j+1;k<nb;k++)
                {
                    double dij,dik,djk,p,s;
                    dij=sqrt(square(_solver.stars[i].x-_solver.stars[j].x)+square(_solver.stars[i].y-_solver.stars[j].y));
                    dik=sqrt(square(_solver.stars[i].x-_solver.stars[k].x)+square(_solver.stars[i].y-_solver.stars[k].y));
                    djk=sqrt(square(_solver.stars[j].x-_solver.stars[k].x)+square(_solver.stars[j].y-_solver.stars[k].y));
                    p=dij+dik+djk;
                    s=sqrt(p*(p-dij)*(p-dik)*(p-djk));
                    //BOOST_LOG_TRIVIAL(debug) << "Trig REF " << " - " << i << " - " << j << " - " << k << " - p=  " << p << " - s=" << s << " - s/p=" << s/p;
                    _trigRef.append({
                                     _solver.stars[i].x,
                                     _solver.stars[i].y,
                                     _solver.stars[j].x,
                                     _solver.stars[j].y,
                                     _solver.stars[k].x,
                                     _solver.stars[k].y,
                                     dij,dik,djk,
                                     p,s,s/p
                                    });
                }
            }
        }

    _calState =0;
    _calStep = 0;
    _pulseW = _pulse;
    _pulseE = 0;
    _pulseN = 0;
    _pulseS = 0;
    _pulseWTot = _pulse;
    _pulseETot = 0;
    _pulseNTot = 0;
    _pulseSTot = 0;

    emit ComputeRefDone();
}
void GuiderModule::SMComputeCal()
{
    BOOST_LOG_TRIVIAL(debug) << "SMComputeCal";
    int nb = _solver.stars.size();
    if (nb > 10) nb = 10;
    _trigCurrent.clear();

    for (int i=0;i<nb;i++)
    {
        for (int j=i+1;j<nb;j++)
        {
            for (int k=j+1;k<nb;k++)
            {
                double dij,dik,djk,p,s;
                dij=sqrt(square(_solver.stars[i].x-_solver.stars[j].x)+square(_solver.stars[i].y-_solver.stars[j].y));
                dik=sqrt(square(_solver.stars[i].x-_solver.stars[k].x)+square(_solver.stars[i].y-_solver.stars[k].y));
                djk=sqrt(square(_solver.stars[j].x-_solver.stars[k].x)+square(_solver.stars[j].y-_solver.stars[k].y));
                p=dij+dik+djk;
                s=sqrt(p*(p-dij)*(p-dik)*(p-djk));
                //BOOST_LOG_TRIVIAL(debug) << "Trig CURRENT" << " - " << i << " - " << j << " - " << k << " - p=  " << p << " - s=" << s << " - s/p=" << s/p;
                _trigCurrent.append({
                                 _solver.stars[i].x,
                                 _solver.stars[i].y,
                                 _solver.stars[j].x,
                                 _solver.stars[j].y,
                                 _solver.stars[k].x,
                                 _solver.stars[k].y,
                                 dij,dik,djk,
                                 p,s,s/p
                                });

            }
        }

    }
    if (_trigCurrent.size()>0) {
        matchTrig(_trigRef,_trigCurrent);
        _dxvector.push_back(_avdx);
        _dyvector.push_back(_avdy);
        if (_dxvector.size() > 1)
        {
            double coeff[2];
            polynomialfit(_dxvector.size(), 2, _dxvector.data(), _dyvector.data(), coeff);
            BOOST_LOG_TRIVIAL(debug) << "Coeffs " << coeff[0] << "-" <<  coeff[1] << " CCD Orientation = " << atan(coeff[1])*180/PI;
        }


    } else {
      BOOST_LOG_TRIVIAL(debug) << "houston, we have a problem";
    }
    _pulseN=0;
    _pulseS=0;
    _pulseE=0;
    _pulseW=0;
    _calStep++;
    if (_calStep >= _calSteps) {
        _calStep=0;
        _calState++;
        if (_calState==2) {
            _dxvector.clear();
            _dyvector.clear();
            _coefficients.clear();
        }
        if (_calState>=4) {
            emit StartGuiding();
            return;
        }
    }
    if (_calState==0) {_pulseW=_pulse;_pulseWTot=_pulseWTot+_pulse;}
    if (_calState==1) {_pulseE=_pulse;_pulseETot=_pulseETot+_pulse;}
    if (_calState==2) {_pulseN=_pulse;_pulseNTot=_pulseNTot+_pulse;}
    if (_calState==3) {_pulseS=_pulse;_pulseSTot=_pulseSTot+_pulse;}

    emit ComputeCalDone();
}
void GuiderModule::SMComputeGuide()
{
    BOOST_LOG_TRIVIAL(debug) << "SMComputeGuide";

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

    emit RequestPulsesDone();
}

void GuiderModule::SMFindStars()
{
    BOOST_LOG_TRIVIAL(debug) << "SMFindStars";

    sendMessage("SMFindStars");
    _solver.ResetSolver(image->stats,image->m_ImageBuffer);
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

    _machine.stop();
    sendMessage("machine stopped");
}

void GuiderModule::matchTrig(QVector<Trig> ref,QVector<Trig> act)
{
    _matchedPairs.clear();

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
                foreach (MatchedPair pair, _matchedPairs) {
                    if ( (pair.xr==r.x1)&&(pair.yr==r.y1) ) found=true;
                }
                if (!found) _matchedPairs.append({r.x1,r.y1,a.x1,a.y1,r.x1-a.x1,r.y1-a.y1});
                found= false;
                foreach (MatchedPair pair, _matchedPairs) {
                    if ( (pair.xr==r.x2)&&(pair.yr==r.y2) ) found=true;
                }
                if (!found) _matchedPairs.append({r.x2,r.y2,a.x2,a.y2,r.x2-a.x2,r.y2-a.y2});
                found= false;
                foreach (MatchedPair pair, _matchedPairs) {
                    if ( (pair.xr==r.x3)&&(pair.yr==r.y3) ) found=true;
                }
                if (!found) _matchedPairs.append({r.x3,r.y3,a.x3,a.y3,r.x3-a.x3,r.y3-a.y3});
            }
        }
    }
    _avdx=0;
    _avdy=0;
    for (int i=0 ; i <_matchedPairs.size();i++ ) {
        _avdx=_avdx+_matchedPairs[i].dx;
        _avdy=_avdy+_matchedPairs[i].dy;
    }
    _avdx=_avdx/_matchedPairs.size();
    _avdy=_avdy/_matchedPairs.size();

    _grid->append(_avdx,_avdy);
    _propertyStore.update(_grid);
    emit propertyAppended(_grid,&_modulename,0,_avdx,_avdy,0);
    BOOST_LOG_TRIVIAL(debug) << "AVDX AVDY =  " << _avdx << "-" << _avdy;

    /*foreach (MatchedPair pair, _matchedPairs) {
            BOOST_LOG_TRIVIAL(debug) << "Matched pair =  " << pair.dx << "-" << pair.dy;
    }*/

}
