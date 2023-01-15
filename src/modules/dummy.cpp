#include "dummy.h"

Dummy *initialize(QString name,QString label,QString profile,QVariantMap availableModuleLibs)
{
    Dummy *basemodule = new Dummy(name,label,profile,availableModuleLibs);
    return basemodule;
}

Dummy::Dummy(QString name, QString label, QString profile,QVariantMap availableModuleLibs)
    : IndiModule(name,label,profile,availableModuleLibs)
{

    //Q_INIT_RESOURCE(dummy);
    _moduletype="dummy";

    loadPropertiesFromFile(":dummy.json");

    setOstProperty("moduleDescription","Dummy module to show what we can do",true);
    setOstProperty("moduleVersion",0.1,true);
    setOstProperty("moduleType",_moduletype,true);

    setOstProperty("message","Dummy module init finished",true);
    setOstElement("extextRO","extext1","Texte read only 1",false);
    setOstElement("extextRO","extext2","Texte read only 2",false);
    setOstElement("extextRO","extext3","Texte read only 3",false);
    setOstElement("extextRO","extext4","Texte read only 4",true);
    setOstElement("boolsRW0","b1",true,false);
    setOstElement("boolsRW0","b2",false,false);
    setOstElement("boolsRW0","b3",false,false);
    setOstElement("boolsRW0","b4",false,true);
    setOstElement("boolsRW1","b1",false,false);
    setOstElement("boolsRW1","b2",false,false);
    setOstElement("boolsRW1","b3",false,false);
    setOstElement("boolsRW1","b4",false,true);
    setOstElement("boolsRW2","b1",true,false);
    setOstElement("boolsRW2","b2",true,false);
    setOstElement("boolsRW2","b3",true,false);
    setOstElement("boolsRW2","b4",false,true);
    setOstElement("numbersRW","n1",0,false);
    setOstElement("numbersRW","n2",-1000,false);
    setOstElement("numbersRW","n3",3.14,false);
    setOstElement("numbersRW","n4",-20.23,true);
    setOstElementAttribute("numbersRW","n4","step",100,true);
    setOstElementAttribute("numbersRW","n4","min",-10000,true);
    setOstElementAttribute("numbersRW","n4","max",10000,true);

    setOstElement("mixedRW","b1",false,false);
    setOstElement("mixedRW","b2",false,false);
    setOstElement("mixedRW","b3",true,false);
    setOstElement("mixedRW","n1",10,false);
    setOstElement("mixedRW","n2",11,false);
    setOstElement("mixedRW","t1","Mixed text value",false);
    //saveAttributesToFile("dummy.json");
    _camera=getOstElementValue("devices","camera").toString();

    foreach(QString key,getAvailableModuleLibs().keys()) {
        QVariantMap info = getAvailableModuleLibs()[key].toMap();
        QString mess;
        if (createOstProperty("mod"+key,"mod"+key,0,"Modules","root",mess)) {
                //BOOST_LOG_TRIVIAL(debug) << "createOstProperty OK : " << mess.toStdString();
        } else {
            BOOST_LOG_TRIVIAL(debug) << "createOstProperty KO : " << mess.toStdString();
        }
    }
    setBLOBMode(B_ALSO,_camera.toStdString().c_str(),nullptr);

}

Dummy::~Dummy()
{
    //Q_CLEANUP_RESOURCE(dummy);
}

void Dummy::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
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

                if (keyprop=="actions") {
                    if (keyelt=="blob") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_OK,true);
                            connectDevice(_camera);
                            connectDevice(getOstElementValue("devices","mount").toString());
                            setBLOBMode(B_ALSO,_camera.toStdString().c_str(),nullptr);
                        }
                    }
                    if (keyelt=="shoot") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_BUSY,true);
                            sendModNewNumber(_camera,"SIMULATOR_SETTINGS","SIM_TIME_FACTOR",0.01 );
                            if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", getOstElementValue("parameters","exposure").toDouble())) {
                                setOstPropertyAttribute(keyprop,"status",IPS_ALERT,true);
                            }
                        }
                    }
                    if (keyelt=="extract") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_BUSY,true);
                            stats=_image->getStats();
                            _solver.ResetSolver(stats,_image->getImageBuffer());
                            connect(&_solver,&Solver::successSEP,this,&Dummy::OnSucessSEP);
                            connect(&_solver,&Solver::solverLog,this,&Dummy::OnSolverLog);
                            _solver.FindStars(_solver.stellarSolverProfiles[0]);
                        }
                    }
                    if (keyelt=="solve") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_BUSY,true);
                            double ra,dec;
                            if (
                                      !getModNumber(getOstElementValue("devices","mount").toString(),"EQUATORIAL_EOD_COORD","DEC",dec)
                                    ||!getModNumber(getOstElementValue("devices","mount").toString(),"EQUATORIAL_EOD_COORD","RA",ra)
                               )
                            {
                                setOstPropertyAttribute(keyprop,"status",IPS_ALERT,true);
                                sendMessage("Can't find mount device "+getOstElementValue("devices","mount").toString()+" solve aborted");
                            } else {
                                setOstElement("imagevalues","mountRA",ra*360/24,false);
                                setOstElement("imagevalues","mountDEC",dec,false);

                                stats=_image->getStats();
                                _solver.ResetSolver(stats,_image->getImageBuffer());
                                QStringList folders;
                                folders.append(getOstElementValue("parameters","indexfolderpath").toString());
                                _solver.stellarSolver->setIndexFolderPaths(folders);
                                connect(&_solver,&Solver::successSolve,this,&Dummy::OnSucessSolve);
                                connect(&_solver,&Solver::solverLog,this,&Dummy::OnSolverLog);
                                _solver.stellarSolver->setSearchPositionInDegrees(ra*360/24,dec);
                                _solver.SolveStars(_solver.stellarSolverProfiles[0]);
                            }
                        }
                    }
                }
            }
        }
    }
}

void Dummy::newBLOB(IBLOB *bp)
{

    if (
            (QString(bp->bvp->device) == _camera)
       )
    {
        delete _image;
        _image=new fileio();
        _image->loadBlob(bp);

        setOstPropertyAttribute("actions","status",IPS_OK,true);
        setOstElement("imagevalues","width",_image->getStats().width,false);
        setOstElement("imagevalues","height",_image->getStats().height,false);
        setOstElement("imagevalues","min",_image->getStats().min[0],false);
        setOstElement("imagevalues","max",_image->getStats().max[0],false);
        setOstElement("imagevalues","mean",_image->getStats().mean[0],false);
        setOstElement("imagevalues","median",_image->getStats().median[0],false);
        setOstElement("imagevalues","stddev",_image->getStats().stddev[0],false);
        setOstElement("imagevalues","snr",_image->getStats().SNR,true);
        QList<fileio::Record> rec=_image->getRecords();
        stats=_image->getStats();
        _image->saveAsFITS(_webroot+"/"+getName()+QString(bp->bvp->device)+".FITS",stats,_image->getImageBuffer(),FITSImage::Solution(),rec,false);

        QImage rawImage = _image->getRawQImage();
        rawImage.save(_webroot+"/"+getName()+QString(bp->bvp->device)+".jpeg","JPG",100);
        setOstPropertyAttribute("testimage","URL",getName()+QString(bp->bvp->device)+".jpeg",true);

    }
    setOstPropertyAttribute("actions","status",IPS_OK,true);


}
void Dummy::OnSucessSEP()
{
    setOstPropertyAttribute("actions","status",IPS_OK,true);
    setOstElement("imagevalues","hfravg",_solver.HFRavg,false);
    setOstElement("imagevalues","starscount",_solver.stars.size(),true);

}
void Dummy::OnSucessSolve()
{
    if (_solver.stellarSolver->failed()) {
        sendMessage("Solver failed");
        setOstPropertyAttribute("actions","status",IPS_ALERT,true);
        setOstPropertyAttribute("imagevalues","status",IPS_ALERT,true);
        setOstElement("imagevalues","solRA",0,false);
        setOstElement("imagevalues","solDEC",0,true);
    } else {
        setOstPropertyAttribute("actions","status",IPS_OK,true);
        setOstPropertyAttribute("imagevalues","status",IPS_OK,true);
        setOstElement("imagevalues","solRA",_solver.stellarSolver->getSolution().ra,false);
        setOstElement("imagevalues","solDEC",_solver.stellarSolver->getSolution().dec,true);
    }

}
void Dummy::OnSolverLog(QString &text)
{
    sendMessage(text);
}
