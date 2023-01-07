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
                            Shoot();
                        }
                    }
                    if (keyelt=="loop") {
                        if (setOstElement(keyprop,keyelt,true,true)) {
                            //
                        }
                    }
                    if (keyelt=="abort") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            emit Abort();
                            //
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
        setOstElement("imagevalues","width",_image->getStats().width,false);
        setOstElement("imagevalues","height",_image->getStats().height,false);
        setOstElement("imagevalues","min",_image->getStats().min[0],false);
        setOstElement("imagevalues","max",_image->getStats().max[0],false);
        setOstElement("imagevalues","mean",_image->getStats().mean[0],false);
        setOstElement("imagevalues","median",_image->getStats().median[0],false);
        setOstElement("imagevalues","stddev",_image->getStats().stddev[0],false);
        setOstElement("imagevalues","snr",_image->getStats().SNR,true);
        sendMessage("SMFindStars");
        _solver.ResetSolver(stats,_image->getImageBuffer());
        connect(&_solver,&Solver::successSEP,this,&InspectorModule::OnSucessSEP);
        _solver.FindStars(_solver.stellarSolverProfiles[0]);

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

void InspectorModule::Shoot()
{
    connectIndi();
    if (connectDevice(_camera)) {
        setBLOBMode(B_ALSO,_camera.toStdString().c_str(),nullptr);
        frameReset(_camera);
        sendModNewNumber(_camera,"SIMULATOR_SETTINGS","SIM_TIME_FACTOR",0.01 );
        sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", _exposure);
        setOstPropertyAttribute("actions","status",IPS_BUSY,true);
    } else {
        setOstPropertyAttribute("actions","status",IPS_ALERT,true);
    }
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
        qDebug() << "draw " << x << "/" << y;
        p.drawEllipse(QPoint(x,y),a*5,b*5);
    }
    p.end();

    resetOstElements("histogram");
    QVector<double> his = _image->getHistogramFrequency(0);
    for( int i=1;i<his.size();i++) {
        //qDebug() << "HIS " << i << "-"  << _image->getCumulativeFrequency(0)[i] << "-"  << _image->getHistogramIntensity(0)[i] << "-"  << _image->getHistogramFrequency(0)[i];
        setOstElement("histogram","i",i,false);
        setOstElement("histogram","n",his[i],false);
        pushOstElements("histogram");
    }

    immap.save(_webroot+"/"+getName()+"map.jpeg","JPG",100);
    setOstPropertyAttribute("imagemap","URL",getName()+"map.jpeg",true);

    emit FindStarsDone();
}
