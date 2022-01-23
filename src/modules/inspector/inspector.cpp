#include "inspector.h"

InspectorModule *initialize(QString name,QString label)
{
    InspectorModule *basemodule = new InspectorModule(name,label);
    return basemodule;
}

InspectorModule::InspectorModule(QString name,QString label)
    : Basemodule(name,label)

{
    _moduledescription="Inspector module";
    _devices = new TextProperty(_modulename,"Options","root","devices","Devices",2,0);
    _devices->addText(new TextValue("camera","Camera","hint",_camera));
    emit propertyCreated(_devices,&_modulename);
    _propertyStore.add(_devices);

    _values = new NumberProperty(_modulename,"Control","root","values","Values",0,0);
    _values->addNumber(new NumberValue("loopHFRavg","Average HFR","hint",0,"",0,99,0));
    _values->addNumber(new NumberValue("imgHFR","Last imgage HFR","hint",0,"",0,99,0));
    _values->addNumber(new NumberValue("iteration","Iteration","hint",0,"",0,99,0));
    emit propertyCreated(_values,&_modulename);
    _propertyStore.add(_values);

    _actions = new SwitchProperty(_modulename,"Control","root","actions","Actions",2,0,1);
    _actions->addSwitch(new SwitchValue("condev","Connect devices","hint",0));
    _actions->addSwitch(new SwitchValue("shoot","Shoot and analyse","hint",0));
    _actions->addSwitch(new SwitchValue("loop","Infinite loop","hint",0));
    _actions->addSwitch(new SwitchValue("abort","Abort","hint",0));
    _actions->addSwitch(new SwitchValue("loadconfs","Load devices conf","hint",0));
    emit propertyCreated(_actions,&_modulename);
    _propertyStore.add(_actions);

    _parameters = new NumberProperty(_modulename,"Control","root","parameters","Parameters",2,0);
    _parameters->addNumber(new NumberValue("steps"         ,"Steps gap","hint",_steps,"",0,2000,100));
    _parameters->addNumber(new NumberValue("iterations"    ,"Iterations","hint",_iterations,"",0,99,1));
    _parameters->addNumber(new NumberValue("loopIterations","Average over","hint",_loopIterations,"",0,99,1));
    _parameters->addNumber(new NumberValue("exposure"      ,"Exposure","hint",_exposure,"",0,120,1));
    emit propertyCreated(_parameters,&_modulename);
    _propertyStore.add(_parameters);

    _img = new ImageProperty(_modulename,"Control","root","viewer","Image property label",0,0,0);
    emit propertyCreated(_img,&_modulename);
    _propertyStore.add(_img);

    _grid = new GridProperty(_modulename,"Control","root","grid","Grid property label",0,0,"SXY","Set","Pos","HFR","","");
    emit propertyCreated(_grid,&_modulename);
    _propertyStore.add(_grid);

}

InspectorModule::~InspectorModule()
{

}
void InspectorModule::OnSetPropertyNumber(NumberProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
    _propertyStore.add(prop);

    QList<NumberValue*> numbers=prop->getNumbers();
    for (int j = 0; j < numbers.size(); ++j) {
        if (numbers[j]->name()=="iterations")      _iterations=numbers[j]->getValue();
        if (numbers[j]->name()=="loopIterations")  _loopIterations=numbers[j]->getValue();
        if (numbers[j]->name()=="exposure")        _exposure=numbers[j]->getValue();
        emit propertyUpdated(prop,&_modulename);
    }
}
void InspectorModule::OnSetPropertyText(TextProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
}

void InspectorModule::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
    SwitchProperty* wprop = _propertyStore.getSwitch(prop->getDeviceName(),prop->getGroupName(),prop->getName());
    QList<SwitchValue*> switchs=prop->getSwitches();
    for (int j = 0; j < switchs.size(); ++j) {
        if (switchs[j]->name()=="shoot") {
            wprop->setSwitch(switchs[j]->name(),true);

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

void InspectorModule::newNumber(INumberVectorProperty *nvp)
{
    if (
            (QString(nvp->device) == _camera )
        &&  (nvp->s==IPS_ALERT)
       )
    {
        sendMessage("cameraAlert");
        emit cameraAlert();
    }
}

void InspectorModule::newBLOB(IBLOB *bp)
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
        image->saveStretchedToJpeg(_webroot+"/"+QString(bp->bvp->device)+".jpeg",100);

        _img->setURL(QString(bp->bvp->device)+".jpeg");
        emit propertyUpdated(_img,&_modulename);
        _propertyStore.add(_img);
        if (_machine.isRunning()) {
            emit ExposureDone();
            emit ExposureBestDone();
        }
    }

}

void InspectorModule::newSwitch(ISwitchVectorProperty *svp)
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

void InspectorModule::SMAbort()
{

    _machine.stop();
    sendMessage("machine stopped");
}


void InspectorModule::SMRequestFrameReset()
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



void InspectorModule::SMRequestExposure()
{
    sendMessage("SMRequestExposure");
    if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure))
    {
        emit abort();
        return;
    }
    emit RequestExposureDone();
}

void InspectorModule::SMFindStars()
{
    sendMessage("SMFindStars");
    _solver.ResetSolver(image->stats,image->m_ImageBuffer);
    connect(&_solver,&Solver::successSEP,this,&InspectorModule::OnSucessSEP);
    _solver.FindStars(_solver.stellarSolverProfiles[0]);
}

void InspectorModule::OnSucessSEP()
{
    _values->setNumber("imgHFR",_solver.HFRavg);
    _propertyStore.update(_values);
    emit propertyUpdated(_values,&_modulename);
    emit FindStarsDone();
}

void InspectorModule::SMCompute()
{
    sendMessage("SMCompute");


    _values->setNumber("loopHFRavg",_loopHFRavg);
    _values->setNumber("iteration",_iteration);
    _propertyStore.update(_values);
    emit propertyUpdated(_values,&_modulename);


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



void InspectorModule::SMRequestExposureBest()
{
    sendMessage("SMRequestExposureBest");
    if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure))
    {
        emit abort();
        return;
    }
    emit RequestExposureBestDone();
}

void InspectorModule::SMComputeResult()
{
    sendMessage("SMComputeResult");
    _values->setNumber("imgHFR",_solver.HFRavg);
    _propertyStore.update(_values);
    emit propertyUpdated(_values,&_modulename);
    // what should i do here ?
    emit ComputeResultDone();
}




void InspectorModule::SMInitLoopFrame()
{
    sendMessage("SMInitLoopFrame");
    _loopIteration=0;
    _loopHFRavg=99;
    _values->setNumber("loopHFRavg",_loopHFRavg);
    _propertyStore.update(_values);
    emit propertyUpdated(_values,&_modulename);
    emit InitLoopFrameDone();
}

void InspectorModule::SMComputeLoopFrame()
{
    sendMessage("SMComputeLoopFrame");
    _loopIteration++;
    _loopHFRavg=((_loopIteration-1)*_loopHFRavg + _solver.HFRavg)/_loopIteration;
    _values->setNumber("loopHFRavg",_loopHFRavg);
    _values->setNumber("imgHFR",_solver.HFRavg);
    _propertyStore.update(_values);
    emit propertyUpdated(_values,&_modulename);

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

void InspectorModule::SMAlert()
{
    sendMessage("SMAlert");
    emit abort();
}

