#include "allsky.h"

Allsky *initialize(QString name,QString label,QString profile,QVariantMap availableModuleLibs)
{
    Allsky *basemodule = new Allsky(name,label,profile,availableModuleLibs);
    return basemodule;
}

Allsky::Allsky(QString name, QString label, QString profile,QVariantMap availableModuleLibs)
    : IndiModule(name,label,profile,availableModuleLibs)
{

    //Q_INIT_RESOURCE(dummy);
    _moduletype="allsky";

    loadPropertiesFromFile(":allsky.json");

    setOstProperty("moduleDescription","Simple allsky camera module",true);
    setOstProperty("moduleVersion",0.1,true);
    setOstProperty("moduleType",_moduletype,true);

    createOstElement("devices","camera","Camera",true);
    setOstElement("devices","camera",   _camera,false);

    //saveAttributesToFile("allsky.json");
    _camera=getOstElementValue("devices","camera").toString();

    _process = new QProcess(this);
    connect(_process,&QProcess::readyReadStandardOutput,this,&Allsky::processOutput);
    connect(_process,&QProcess::readyReadStandardError, this,&Allsky::processError);
    connect(_process,static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,&Allsky::processFinished);

}

Allsky::~Allsky()
{
    //Q_CLEANUP_RESOURCE(dummy);
}

void Allsky::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
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
                    if (keyelt=="batch") {
                        if (setOstElement(keyprop,keyelt,true,true)) {
                            startBatch();
                        }
                    }
                    if (keyelt=="loop") {
                        if (setOstElement(keyprop,keyelt,true,false)) {
                            setOstPropertyAttribute("actions","status",IPS_BUSY,true);
                            startLoop();
                        }
                    }
                    if (keyelt=="abort") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            _isLooping=false;
                            setOstElement(keyprop,"loop",false,false);
                            setOstPropertyAttribute("actions","status",IPS_OK,true);

                        }
                    }
                }
            }
        }
    }
}
void Allsky::startLoop()
{
    _isLooping=true;
    _index=0;

    QDir dir0(_webroot+"/"+getName()+"/batch/", {"*"});
    for(const QString & filename: dir0.entryList()){
        dir0.remove(filename);
    }

    QDir dir;
    dir.mkdir(_webroot+"/"+getName());
    dir.mkdir(_webroot+"/"+getName()+"/batch/");

    connectIndi();
    connectDevice(_camera);
    setBLOBMode(B_ALSO,_camera.toStdString().c_str(),nullptr);
    sendModNewNumber(_camera,"SIMULATOR_SETTINGS","SIM_TIME_FACTOR",0.01 );

    if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", getOstElementValue("parameters","exposure").toDouble())) {
        setOstPropertyAttribute("actions","status",IPS_ALERT,true);
    }

}
void Allsky::startBatch()
{
    qDebug() << "PROCESS Start Batch";
    if (_process->state()!=0) {
        qDebug() << "can't start process";
    } else {
        QString program = "ffmpeg";
        QStringList arguments;
        arguments << "-framerate" << "30";
        arguments << "-pattern_type" << "glob";
        arguments << "-i" << _webroot+"/"+getName()+"/batch/*.jpeg";
        arguments << "-c:v" << "libx264";
        arguments << "-pix_fmt" << "yuv420p";
        arguments << "-framerate" << "30";
        arguments << "-y";
        arguments << _webroot+"/"+getName()+"/batch/"+getName()+".mp4";
        qDebug() << "PROCESS ARGS " << arguments;
        _process->start(program,arguments);

    }

}
void Allsky::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    sendMessage("PROCESS FINISHED (" + QString::number(exitCode) + ")");
}
void Allsky::processOutput()
{
    QString output = _process->readAllStandardOutput();
    qDebug() << "PROCESS LOG   " << output;
}
void Allsky::processError()
{
    QString output = _process->readAllStandardError();
    sendMessage("Process log : "+output);
}
void Allsky::newBLOB(IBLOB *bp)
{

    if (
            (QString(bp->bvp->device) == _camera)
       )
    {
        setOstPropertyAttribute("actions","status",IPS_OK,true);
        delete _image;
        _image=new fileio();
        _image->loadBlob(bp);

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
        _index++;
        QImage rawImage = _image->getRawQImage();
        rawImage.save(_webroot+"/"+getName()+QString(bp->bvp->device)+".jpeg","JPG",100);
        setOstPropertyAttribute("image","URL",getName()+QString(bp->bvp->device)+".jpeg",true);
        QString _n = QStringLiteral("%1").arg(_index, 10, 10, QLatin1Char('0'));
        rawImage.save(_webroot+"/"+getName()+"/batch/"+_n+".jpeg","JPG",100);

        setOstPropertyAttribute("actions","status",IPS_BUSY,true);
        if (_isLooping)
        {
            if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", getOstElementValue("parameters","exposure").toDouble())) {
                setOstPropertyAttribute("actions","status",IPS_ALERT,true);
                _isLooping=false;
            }
        }

    }

}
