#include "inspector.h"
#include <QPainter>

InspectorModule *initialize(QString name,QString label,QString profile,QVariantMap availableModuleLibs)
{
    InspectorModule *basemodule = new InspectorModule(name,label,profile,availableModuleLibs);
    return basemodule;
}

InspectorModule::InspectorModule(QString name, QString label, QString profile,QVariantMap availableModuleLibs)
    : IndiModule(name,label,profile,availableModuleLibs)

{
    _moduletype="inspector";

    loadPropertiesFromFile(":inspector.json");

    setOstProperty("moduleDescription","Inspector module",true);
    setOstProperty("moduleVersion",0.1,true);
    setOstProperty("moduleType",_moduletype,true);

    createOstElement("devices","camera","Camera",true);
    setOstElement("devices","camera",   _camera,false);

    //saveAttributesToFile("inspector.json");
    _camera=getOstElementValue("devices","camera").toString();
    _exposure=getOstElementValue("parameters","exposure").toFloat();

    /*_moduledescription="Inspector module";
    _devices = new TextProperty(_modulename,"Options","root","devices","Devices",2,0);
    _devices->addText(new TextValue("camera","Camera","hint",_camera));
    emit propertyCreated(_devices,&_modulename);
    _propertyStore.add(_devices);

    _values = new NumberProperty(_modulename,"Control","root","values","Values",0,0);
    //_values->addNumber(new NumberValue("loopHFRavg","Average HFR","hint",0,"",0,99,0));
    _values->addNumber(new NumberValue("imgHFR","Last imgage HFR","hint",0,"",0,99,0));
    //_values->addNumber(new NumberValue("iteration","Iteration","hint",0,"",0,99,0));
    emit propertyCreated(_values,&_modulename);
    _propertyStore.add(_values);


    _parameters = new NumberProperty(_modulename,"Control","root","parameters","Parameters",2,0);
    //_parameters->addNumber(new NumberValue("steps"         ,"Steps gap","hint",_steps,"",0,2000,100));
    //_parameters->addNumber(new NumberValue("iterations"    ,"Iterations","hint",_iterations,"",0,99,1));
    //_parameters->addNumber(new NumberValue("loopIterations","Average over","hint",_loopIterations,"",0,99,1));
    _parameters->addNumber(new NumberValue("exposure"      ,"Exposure","hint",_exposure,"",0,120,1));
    emit propertyCreated(_parameters,&_modulename);
    _propertyStore.add(_parameters);

    _img = new ImageProperty(_modulename,"Control","root","viewer","Image property label",0,0,0);
    emit propertyCreated(_img,&_modulename);
    _propertyStore.add(_img);

    _grid = new GridProperty(_modulename,"Control","root","grid","Grid property label",0,0,"SXY","Set","Pos","HFR","","");
    emit propertyCreated(_grid,&_modulename);
    _propertyStore.add(_grid);*/

    SMBuild();
    SMBuildLoop();
    //setBlobMode();

}

InspectorModule::~InspectorModule()
{

}
void InspectorModule::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{
        //BOOST_LOG_TRIVIAL(debug) << "OnMyExternalEvent - recv : " << getName().toStdString() << "-" << eventType.toStdString() << "-" << eventKey.toStdString();
    if (getName()==eventModule)
    {
        foreach(const QString& keyprop, eventData.keys()) {
            foreach(const QString& keyelt, eventData[keyprop].toMap()["elements"].toMap().keys()) {
                setOstElement(keyprop,keyelt,eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"],true);
                if (keyprop=="devices") {
                    if (keyelt=="camera") {
                        if (setOstElement(keyprop,keyelt,eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"],false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
                            _camera=getOstElementValue("devices","camera").toString();
                        }
                    }
                }

                if (keyprop=="parameters") {
                    if (keyelt=="exposure") {
                        if (setOstElement(keyprop,keyelt,eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"],false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
                            _exposure=getOstElementValue("parameters","exposure").toFloat();
                        }
                    }
                }
                if (keyprop=="actions") {
                    if (keyelt=="shoot") {
                        if (setOstElement(keyprop,keyelt,true,true)) {
                            _machine.start();
                        }
                    }
                    if (keyelt=="loop") {
                        if (setOstElement(keyprop,keyelt,true,true)) {
                            _machineLoop.start();
                        }
                    }
                    if (keyelt=="abort") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            emit Abort();
                            _machine.stop();
                            _machineLoop.stop();
                        }
                    }
                }
            }
        }
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
        setOstPropertyAttribute("actions","status",IPS_OK,true);
        delete _image;
        _image=new fileio();
        _image->loadBlob(bp);
        stats=_image->getStats();
        //_image->computeHistogram();
        emit ExposureDone();
        setOstElement("imagevalues","width",_image->getStats().width,false);
        setOstElement("imagevalues","height",_image->getStats().height,false);
        setOstElement("imagevalues","min",_image->getStats().min[0],false);
        setOstElement("imagevalues","max",_image->getStats().max[0],false);
        setOstElement("imagevalues","mean",_image->getStats().mean[0],false);
        setOstElement("imagevalues","median",_image->getStats().median[0],false);
        setOstElement("imagevalues","stddev",_image->getStats().stddev[0],false);
        setOstElement("imagevalues","snr",_image->getStats().SNR,true);
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
        emit FrameResetDone();
    }


}

void InspectorModule::SMBuild()
{
    auto *Abort = new QState();
    auto *Init  = new QState();
    auto *End   = new QFinalState();

    auto *InitInit             = new QState(Init);
    auto *RequestFrameReset    = new QState(Init);
    auto *WaitFrameReset       = new QState(Init);
    auto *RequestExposure      = new QState(Init);
    auto *WaitExposure         = new QState(Init);
    auto *FindStars            = new QState(Init);
    auto *Compute              = new QState(Init);

    connect(InitInit            ,&QState::entered, this, &InspectorModule::SMInit);
    connect(RequestFrameReset   ,&QState::entered, this, &InspectorModule::SMRequestFrameReset);
    connect(RequestExposure     ,&QState::entered, this, &InspectorModule::SMRequestExposure);
    connect(FindStars           ,&QState::entered, this, &InspectorModule::SMFindStars);
    connect(Compute             ,&QState::entered, this, &InspectorModule::SMCompute);
    connect(Abort,               &QState::entered, this, &InspectorModule::SMAbort);

    Init->                addTransition(this,&InspectorModule::Abort                ,Abort);
    Abort->               addTransition(this,&InspectorModule::AbortDone            ,End);
    InitInit->            addTransition(this,&InspectorModule::InitDone             ,RequestFrameReset);
    RequestFrameReset->   addTransition(this,&InspectorModule::RequestFrameResetDone,WaitFrameReset);
    WaitFrameReset->      addTransition(this,&InspectorModule::FrameResetDone       ,RequestExposure);
    RequestExposure->     addTransition(this,&InspectorModule::RequestExposureDone  ,WaitExposure);
    WaitExposure->        addTransition(this,&InspectorModule::ExposureDone         ,FindStars);
    FindStars->           addTransition(this,&InspectorModule::FindStarsDone        ,Compute);
    Compute->             addTransition(this,&InspectorModule::ComputeDone          ,End);

    Init->setInitialState(InitInit);

    _machine.addState(Init);
    _machine.addState(Abort);
    _machine.addState(End);
    _machine.setInitialState(Init);

}
void InspectorModule::SMBuildLoop()
{
    auto *Abort = new QState();
    auto *Init  = new QState();
    auto *End   = new QFinalState();

    auto *InitInit             = new QState(Init);
    auto *RequestFrameReset    = new QState(Init);
    auto *WaitFrameReset       = new QState(Init);
    auto *RequestExposure      = new QState(Init);
    auto *WaitExposure         = new QState(Init);
    auto *FindStars            = new QState(Init);
    auto *Compute              = new QState(Init);

    connect(InitInit            ,&QState::entered, this, &InspectorModule::SMInit);
    connect(RequestFrameReset   ,&QState::entered, this, &InspectorModule::SMRequestFrameReset);
    connect(RequestExposure     ,&QState::entered, this, &InspectorModule::SMRequestExposure);
    connect(FindStars           ,&QState::entered, this, &InspectorModule::SMFindStars);
    connect(Compute             ,&QState::entered, this, &InspectorModule::SMCompute);
    connect(Abort,               &QState::entered, this, &InspectorModule::SMAbort);

    Init->                addTransition(this,&InspectorModule::Abort                ,Abort);
    Abort->               addTransition(this,&InspectorModule::AbortDone            ,End);
    InitInit->            addTransition(this,&InspectorModule::InitDone             ,RequestFrameReset);
    RequestFrameReset->   addTransition(this,&InspectorModule::RequestFrameResetDone,WaitFrameReset);
    WaitFrameReset->      addTransition(this,&InspectorModule::FrameResetDone       ,RequestExposure);
    RequestExposure->     addTransition(this,&InspectorModule::RequestExposureDone  ,WaitExposure);
    WaitExposure->        addTransition(this,&InspectorModule::ExposureDone         ,FindStars);
    FindStars->           addTransition(this,&InspectorModule::FindStarsDone        ,Compute);
    Compute->             addTransition(this,&InspectorModule::ComputeDone          ,RequestExposure);

    Init->setInitialState(InitInit);

    _machineLoop.addState(Init);
    _machineLoop.addState(Abort);
    _machineLoop.addState(End);
    _machineLoop.setInitialState(Init);

}
void InspectorModule::SMInit()
{
    connectIndi();
    connectDevice(_camera);
    setBLOBMode(B_ALSO,_camera.toStdString().c_str(),nullptr);
    sendModNewNumber(_camera,"SIMULATOR_SETTINGS","SIM_TIME_FACTOR",0.01 );


    emit InitDone();
}

void InspectorModule::SMAbort()
{

    _machine.stop();
    _machineLoop.stop();
    emit AbortDone();
    BOOST_LOG_TRIVIAL(debug) << "SMAbort";
    sendMessage("machine stopped");
}


void InspectorModule::SMRequestFrameReset()
{
    sendMessage("SMRequestFrameReset");

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
    stats=_image->getStats();
    _solver.ResetSolver(stats,_image->getImageBuffer());
    connect(&_solver,&Solver::successSEP,this,&InspectorModule::OnSucessSEP);
    _solver.FindStars(_solver.stellarSolverProfiles[0]);
}

void InspectorModule::OnSucessSEP()
{
    setOstPropertyAttribute("actions","status",IPS_OK,true);
    setOstElement("imagevalues","imgHFR",_solver.HFRavg,false);
    setOstElement("imagevalues","starscount",_solver.stars.size(),true);


    //image->saveMapToJpeg(_webroot+"/"+_modulename+".jpeg",100,_solver.stars);
    QList<fileio::Record> rec=_image->getRecords();
    QImage rawImage = _image->getRawQImage();
    QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
    im.setColorTable(rawImage.colorTable());
    QImage immap = rawImage.convertToFormat(QImage::Format_RGB32);
    immap.setColorTable(rawImage.colorTable());

    im.save(_webroot+"/"+getName()+".jpeg","JPG",100);
    setOstPropertyAttribute("image","URL",getName()+".jpeg",true);

    //QRect r;
    //r.setRect(0,0,im.width(),im.height());

    QPainter p;
    p.begin(&immap);
    p.setPen(QPen(Qt::red));
    //p.setFont(QFont("Times", 15, QFont::Normal));
    //p.drawText(r, Qt::AlignLeft, QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss zzz") );
    p.setPen(QPen(Qt::green));
    foreach( FITSImage::Star s, _solver.stars )
    {
        qDebug() << "draw " << s.x << "/" << s.y;
        int x=s.x;
        int y=s.y;
        int a=s.a;
        int b=s.b;
        p.drawEllipse(QPoint(x,y),a*5,b*5);
    }
    p.end();
    immap.save(_webroot+"/"+getName()+"map.jpeg","JPG",100);
    setOstPropertyAttribute("imagemap","URL",getName()+"map.jpeg",true);

    emit FindStarsDone();
}

void InspectorModule::SMCompute()
{
    sendMessage("SMCompute");




    emit ComputeDone();
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

    setOstElement("imagevalues","width",_image->getStats().width,false);
    setOstElement("imagevalues","height",_image->getStats().height,false);
    setOstElement("imagevalues","min",_image->getStats().min[0],false);
    setOstElement("imagevalues","max",_image->getStats().max[0],false);
    setOstElement("imagevalues","mean",_image->getStats().mean[0],false);
    setOstElement("imagevalues","median",_image->getStats().median[0],false);
    setOstElement("imagevalues","stddev",_image->getStats().stddev[0],false);
    setOstElement("imagevalues","snr",_image->getStats().SNR,false);
    setOstElement("imagevalues","imgHFR",_solver.HFRavg,true);
    emit ComputeResultDone();
    // what should i do here ?

}




void InspectorModule::SMInitLoopFrame()
{
    sendMessage("SMInitLoopFrame");
    _loopIteration=0;
    _loopHFRavg=99;
    setOstElement("imagevalues","imgHFR",_loopHFRavg,false);
    emit InitLoopFrameDone();
}

void InspectorModule::SMComputeLoopFrame()
{
    sendMessage("SMComputeLoopFrame");
    _loopIteration++;
    _loopHFRavg=((_loopIteration-1)*_loopHFRavg + _solver.HFRavg)/_loopIteration;
    setOstElement("imagevalues","imgHFR",_loopHFRavg,false);

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

