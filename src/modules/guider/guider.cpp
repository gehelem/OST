#include "guider.h"


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
    _parameters->addNumber(new NumberValue("exposure"      ,"Exposure","hint",_exposure,"",0,120,1));
    emit propertyCreated(_parameters,&_modulename);
    _propertyStore.add(_parameters);

    _values = new NumberProperty(_modulename,"Control","root","values","Values",0,0);
    //_values->addNumber(new NumberValue("xxx","xxx","hint",0,"",0,99,0));
    emit propertyCreated(_values,&_modulename);
    _propertyStore.add(_values);

    _img = new ImageProperty(_modulename,"Control","root","viewer","Image property label",0,0,0);
    emit propertyCreated(_img,&_modulename);
    _propertyStore.add(_img);


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
 Q_UNUSED(nvp)
}

void GuiderModule::newBLOB(IBLOB *bp)
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

        _img->setURL(QString(bp->bvp->device)+".jpeg");
        emit propertyUpdated(_img,&_modulename);
        _propertyStore.add(_img);
        if (_machine.isRunning()) {
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
    auto *calibrate = new QState();
    auto *Abort       = new QState();

    auto *RequestFrameReset = new QState(calibrate);
    auto *WaitFrameReset    = new QState(calibrate);
    auto *RequestExposure   = new QState(calibrate);
    auto *WaitExposure      = new QState(calibrate);
    auto *FindStars         = new QState(calibrate);
    calibrate->setInitialState(RequestFrameReset);

    _machine.addState(calibrate);
    _machine.addState(Abort);
    _machine.setInitialState(calibrate);

    connect(RequestFrameReset,  &QState::entered, this, &GuiderModule::SMRequestFrameReset);
    connect(RequestExposure,    &QState::entered, this, &GuiderModule::SMRequestExposure);
    connect(FindStars,          &QState::entered, this, &GuiderModule::SMFindStars);
    connect(Abort,              &QState::entered, this, &GuiderModule::SMAbort);

    calibrate->         addTransition(this,&GuiderModule::abort,                Abort);
    RequestFrameReset-> addTransition(this,&GuiderModule::RequestFrameResetDone,WaitFrameReset);
    WaitFrameReset->    addTransition(this,&GuiderModule::FrameResetDone       ,RequestExposure);
    RequestExposure->   addTransition(this,&GuiderModule::RequestExposureDone , WaitExposure);
    WaitExposure->      addTransition(this,&GuiderModule::ExposureDone ,        FindStars);
    FindStars->         addTransition(this,&GuiderModule::FindStarsDone,        RequestExposure);



    connect(RequestFrameReset,  &QState::entered, this, &GuiderModule::SMRequestFrameReset);
    connect(RequestExposure,  &QState::entered, this, &GuiderModule::SMRequestExposure);
    connect(FindStars,  &QState::entered, this, &GuiderModule::SMFindStars);

    _machine.start();
    sendMessage("machine started");
}
void GuiderModule::SMRequestFrameReset()
{
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
    sendMessage("SMRequestExposure");
    if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure))
    {
        emit abort();
        return;
    }
    emit RequestExposureDone();
}

void GuiderModule::SMFindStars()
{
    sendMessage("SMFindStars");
    _solver.ResetSolver(image->stats,image->m_ImageBuffer);
    connect(&_solver,&Solver::successSEP,this,&GuiderModule::OnSucessSEP);
    _solver.FindStars(_solver.stellarSolverProfiles[6]);
}

void GuiderModule::OnSucessSEP()
{
    sendMessage("SEP finished");
    disconnect(&_solver,&Solver::successSEP,this,&GuiderModule::OnSucessSEP);
    BOOST_LOG_TRIVIAL(debug) << "********* SEP Finished";
    foreach (FITSImage::Star star, _solver.stars) {
        BOOST_LOG_TRIVIAL(debug) << "Star found " << " - " << star.x << " - " << star.y << " - " << star.mag << " - " << star.HFR  << " - " << _solver.stars.size();
    }
    emit FindStarsDone();
}

void GuiderModule::SMAbort()
{

    _machine.stop();
    sendMessage("machine stopped");
}
