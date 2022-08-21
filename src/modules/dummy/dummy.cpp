#include "dummy.h"

Dummy *initialize(QString name,QString label,QString profile)
{
    Dummy *basemodule = new Dummy(name,label,profile);
    return basemodule;
}

Dummy::Dummy(QString name, QString label, QString profile)
    : Basemodule(name,label,profile)
{

    Q_INIT_RESOURCE(dummy);
    _moduletype="dummy";

    loadPropertiesFromFile(":dummy.json");

    setOstProperty("moduleDescription","Dummy module to show what we can do",true);

    //createOstProperty("version","Version",0);
    setOstProperty("version",0.1,true);


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

}

Dummy::~Dummy()
{
    Q_CLEANUP_RESOURCE(dummy);
}

void Dummy::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{
        //BOOST_LOG_TRIVIAL(debug) << "OnMyExternalEvent - recv : " << getName().toStdString() << "-" << eventType.toStdString() << "-" << eventKey.toStdString();
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
                    if (keyelt=="shoot") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_BUSY,true);
                            connectDevice(_camera);
                            setBlobMode();
                            if (!sendModNewNumber(_camera,"CCD_EXPOSURE","CCD_EXPOSURE_VALUE", getOstElementValue("parameters","exposure").toDouble())) {
                                setOstPropertyAttribute(keyprop,"status",IPS_ALERT,true);
                            }
                        }
                    }
                    if (keyelt=="extract") {
                        if (setOstElement(keyprop,keyelt,false,false)) {
                            setOstPropertyAttribute(keyprop,"status",IPS_BUSY,true);
                            _solver.ResetSolver(_image->stats,_image->m_ImageBuffer);
                            connect(&_solver,&Solver::successSEP,this,&Dummy::OnSucessSEP);
                            _solver.FindStars(_solver.stellarSolverProfiles[0]);
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
        _image = new Image();
        _image->LoadFromBlob2(bp);
        /*_image->CalcStats();
        _image->computeHistogram();
        _image->saveStretchedToJpeg(_webroot+"/"+QString(bp->bvp->device)+".jpeg",100);*/

        setOstPropertyAttribute("actions","status",IPS_OK,true);
        setOstElement("imagevalues","width",_image->stats.width,false);
        setOstElement("imagevalues","height",_image->stats.height,true);

    }

}
void Dummy::OnSucessSEP()
{
    setOstPropertyAttribute("actions","status",IPS_OK,true);
    setOstElement("imagevalues","hfravg",_solver.HFRavg,false);
    setOstElement("imagevalues","starscount",_solver.stars.size(),true);

}
