#include "dummy.h"
#include "version.cc"

Dummy *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Dummy *basemodule = new Dummy(name, label, profile, availableModuleLibs);
    return basemodule;
}

Dummy::Dummy(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{

    //Q_INIT_RESOURCE(dummy);
    loadOstPropertiesFromFile(":dummy.json");

    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Dummy module to show what we can do and not");
    setMetadata("thisversion", "0.1");

    getEltString("extextRO", "extext1")->setValue("Text read only 1");
    getEltString("extextRO", "extext2")->setValue("Text read only 2");
    getEltString("extextRO", "extext3")->setValue("Text read only 3");
    getEltString("extextRO", "extext4")->setValue("Text read only 4", true);


    getEltBool("boolsRW0", "b1")->setValue(true, true);
    getEltBool("boolsRW1", "b1")->setValue(false);
    getEltBool("boolsRW1", "b2")->setValue(false);
    getEltBool("boolsRW1", "b3")->setValue(false);
    getEltBool("boolsRW1", "b4")->setValue(false, true);
    getEltBool("boolsRW2", "b1")->setValue(true);
    getEltBool("boolsRW2", "b2")->setValue(true);
    getEltBool("boolsRW2", "b3")->setValue(true);
    getEltBool("boolsRW2", "b4")->setValue(false, true);
    getEltInt("numbersRW", "n1")->setValue(0);
    getEltInt("numbersRW", "n2")->setValue(-1000);
    getEltFloat("numbersRW", "n3")->setValue(3.14);
    getEltInt("numbersRW", "n4")->setValue(-20.23);
    getEltBool("mixedRW", "b3")->setValue(true, true);
    getEltInt("mixedRW", "n1")->setValue(10);
    getEltInt("mixedRW", "n2")->setValue(11);
    getEltString("mixedRW", "t1")->setValue("Mixed text value", true);

    getEltString("extextRW", "extext4")->lovUpdate("i1", "i1 label modified");
    //addOstElementLov("extextRW", "extext4", "i1", "i1 label modified"); // should give a warning
    getEltString("extextRW", "extext4")->lovAdd("i3", "i3 label");
    getEltString("extextRW", "extext4")->lovAdd("i4", "i4 label");
    getEltString("extextRW", "extext4")->lovDel("i4");
    getEltString("extextRW", "extext4")->lovDel("i4444");

    OST::PropertyMulti *n = getProperty("numbersRW");
    n->setState(OST::State::Error, true);
    OST::ElementFloat *numbersRWn3 = getEltFloat("numbersRW", "n3");
    numbersRWn3->setValue(999666, true);

    //getText("extextRW", "extext1")->setValue("Value modified");
    //OST::ElementJsonDumper d;
    //getText("extextRW", "extext1")->accept(&d);
    //qDebug() << d.getResult();
    OST::PropertyMulti *p = getProperty("extextRW");
    p->setState(OST::State::Busy, true);
    static_cast<OST::ElementString*>(p->getElt("extext1"))->setValue("Value modified2", false);
    static_cast<OST::ElementString*>(p->getElt("extext4"))->lovUpdate("i3", "another label");

    dynprop = new OST::PropertyMulti("dynprop", "Dynamic", OST::Permission::ReadWrite, "Examples",
                                     "Dynamically instanciated", "", true, false);
    dynlight = new OST::ElementLight("dynlight", "Dyn light", "", "");
    dynlight->setValue(OST::State::Busy, true);
    dynprop->addElt( dynlight);
    dyntext = new OST::ElementString("dyntext", "Dyn text", "", "");
    dynprop->addElt(dyntext);
    createProperty(dynprop);
    dynprop->setState(OST::State::Busy, true);
    dyntext->setValue("Okydoky", false);
    dynlight->setValue(OST::State::Ok, true);

    dynbool = new OST::ElementBool("dynbool", "Dyn bool", "", "");
    dynprop->addElt(dynbool);
    dynbool->setValue(false, false);

    dyntext2 = getEltString("extextRW", "extext1");
    dyntext2->setValue("torototo", true);

    giveMeADevice("camera", "Multipurpose camera", INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("mount", "Multipurpose mount", INDI::BaseDevice::TELESCOPE_INTERFACE);
    defineMeAsFocuser();
    defineMeAsGuider();
    defineMeAsSequencer();
    defineMeAsNavigator();

    getProperty("timesRWgrid")->push();
    getProperty("timesRWgrid")->push();
    getProperty("datesRWgrid")->push();
    getProperty("datesRWgrid")->push();

    connect(this, &Dummy::newImage, this, &Dummy::OnNewImage);


    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Dummy::onTimer);
    timer->start(5000);

}

void Dummy::onTimer()
{

    //    zz = 0,    /*!< no dump       */
    //        aa,        /*!< dump all data */
    //        ee,        /*!< set one element value */
    //        ea,        /*!< set all elements values (prop level) */
    //        ev,        /*!< set one element value/min/max/format */
    //        ps,        /*!< only property state */
    //        gc,        /*!< grid new line */
    //        gu,        /*!< grid update line  */
    //        gd,        /*!< grid delete line  */
    //        gr,        /*!< grid reset */
    //        lc,        /*!< lov create */
    //        lu,        /*!< lov udpate */
    //        ld,        /*!< lov delete */
    //        dp,        /*!< delete/remove property */
    //        de,        /*!< delete/remove element  */
    //        dm,        /*!< delete/remove module   */
    //        am,        /*!< add module   */
    //

    //logDebug("ee :");
    //usleep(50);
    //getEltInt("numbersRW", "n1")->setValue(12, true);

    //logDebug("ea :");
    //usleep(50);
    //QVariantMap j;
    //j["n1"] = 1;
    //j["n2"] = 2;
    //j["n3"] = 3;
    //j["n4"] = 4;
    //j["n5"] = 5;
    //getProperty("numbersRW")->setAll(j);

    //logDebug("ev :");
    //usleep(50);
    //getEltInt("numbersRW", "n1")->setMinMax(0, 100, true);

    //logDebug("ps x3 :");
    //usleep(50);
    //getProperty("numbersRW")->disable();
    //getProperty("numbersRW")->setState(OST::State::Ok, true);
    //getProperty("numbersRW")->enable();

    //logDebug("gc x2:");
    //usleep(50);
    //getProperty("numbersRW")->newLine(j);
    //getProperty("numbersRW")->newLine(j);

    //logDebug("gu :");
    //usleep(50);
    //getProperty("numbersRW")->updateLine(1, j);

    //logDebug("gd :");
    //usleep(50);
    //getProperty("numbersRW")->deleteLine(1);

    //logDebug("fetch ?? :");
    //usleep(50);
    //getProperty("numbersRW")->fetchLine(3);

    //logDebug("updateline ?? :");
    //usleep(50);
    //getProperty("numbersRW")->updateLine(3);



    //logInfo("Test log %1", {100});
    //logDebug("Available indi drivers");
    //logDebug("Debug");
    //logInfo("Info");
    //logWarning("Warning");
    //logError("Error");
    //logCritical("Critical");
    //getEltInt("totoRW", "toto")->setValue(12);

    //getEltInt("numbersRW", "n1")->setValue(12);
    ////getEltInt("numbersRW", "n1")->setValue(200);
    //getProperty("timesRWgrid")->push();
    //getProperty("timesRWgrid")->deleteLine(0);

    //getProperty("dynprop")->disable();
    //getProperty("dynprop")->enable();


    //logDebug("createprop :");
    //dynprop = new OST::PropertyMulti("dynprop2", "Dynamic", OST::Permission::ReadWrite, "Examples",
    //                                 "Dynamically instanciated", "", true, false);
    //dynlight = new OST::ElementLight("dynlight", "Dyn light", "", "");
    //dynlight->setValue(OST::State::Busy, false);
    //dynprop->addElt( dynlight);
    //dyntext = new OST::ElementString("dyntext", "Dyn text", "", "");
    //dynprop->addElt(dyntext);
    //createProperty(dynprop);

    //logDebug("deleteprop :");
    //deleteOstProperty("dynprop2");

    //logDebug("profile ");

    //QJsonDocument d(this->getProfile().toObject());
    //QByteArray ba = d.toJson(QJsonDocument::Compact);
    //logDebug(QString(ba));



    //dynprop->setState(OST::State::Busy, true);
    //dyntext->setValue("Okydoky", false);
    //dynlight->setValue(OST::State::Ok, true);

    //dynbool = new OST::ElementBool("dynbool", "Dyn bool", "", "");
    //dynprop->addElt(dynbool);
    //dynbool->setValue(false, true);

    //deleteOstProperty("dynprop2");




}

Dummy::~Dummy()
{
    //Q_CLEANUP_RESOURCE(dummy);
}

/**
 * @brief Dummy module custom event handler (Hook 3/3)
 *
 * Override of Basemodule's hook for Dummy-specific events.
 * Called automatically after onExternalEventBase() and onExternalEventIndi().
 *
 * NO need to call IndiModule::onExternalEvent() - it's empty!
 * Orchestration is automatic via Basemodule::onExternalEventRoot() final.
 */
void Dummy::onExternalEvent(OST::ExtEvent event)
{
    //qDebug() << "Dummy::onExternalEvent" << event.data;
    //if (getModuleName() != e.module ) return;
    //foreach(const QString &keyprop, e.data.keys())
    //{
    //    if ((e.type == "Fpropposticon1") && (keyprop == "modulesstatus"))
    //    {
    //        getProperty("modulesstatus")->clearGrid();
    //        emit moduleStatusRequest();
    //    }
    //    foreach(const QString &keyelt, e.data[keyprop].toMap()["elements"].toMap().keys())
    //    {
    //        if (keyprop == "dynprop")
    //        {
    //            if (keyelt == "dyntext")
    //            {

    //                dyntext->setValue(e.data[keyprop].toMap()["elements"].toMap()[keyelt].toString(), true);
    //            }
    //            if (keyelt == "dynbool")
    //            {
    //                bool val = e.data["dynprop"].toMap()["elements"].toMap()["dynbool"].toBool();
    //                if (val)
    //                {

    //                    dyntext->setValue("Changed from dynamic switch", true);
    //                }

    //            }
    //        }
    //        if (keyprop == "boolsRW2")
    //        {
    //            if (keyelt == "b1")
    //            {
    //                // test max gridlimit
    //                getProperty("secondtestgrid")->clearGrid();
    //                getProperty("secondtestgrid")->setGridLimit(1010);
    //                //for ( int i = 0; i < 1010; i++)getProperty("secondtestgrid")->push();
    //            }
    //        }
    //        if (keyprop == "devices")
    //        {
    //            if (keyelt == "camera")
    //            {
    //                if (getEltString(keyprop, keyelt)->setValue(
    //                            e.data[keyprop].toMap()["elements"].toMap()[keyelt].toString(), false))
    //                {
    //                    getProperty(keyprop)->setState(OST::Ok, true);
    //                    _camera = getString("devices", "camera");
    //                }
    //            }
    //        }
    //        if (keyprop == "actions2")
    //        {
    //            if (keyelt == "blob")
    //            {
    //                if (getEltBool(keyprop, keyelt)->setValue(false, false))
    //                {
    //                    connectIndi();
    //                    getProperty(keyprop)->setState(OST::Ok, true);
    //                    _camera = getString("devices", "camera");
    //                    connectDevice(_camera);
    //                    setBLOBMode(B_ALSO, _camera.toStdString().c_str(), nullptr);
    //                    enableDirectBlobAccess(_camera.toStdString().c_str(), nullptr);
    //                }
    //            }
    //            if (keyelt == "shoot")
    //            {

    //                if (getEltBool(keyprop, keyelt)->setValue(false, false))
    //                {
    //                    getProperty(keyprop)->setState(OST::Busy, true);
    //                    _camera = getString("devices", "camera");
    //                    double d = getPixelSize(_camera);
    //                    if (_camera == "CCD Simulator")
    //                    {
    //                        sendModNewNumber(_camera, "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 0.02 );
    //                    }
    //                    setFocalLengthAndDiameter();
    //                    if (!requestCapture(_camera, getFloat("parms", "exposure"), getInt("parms", "gain"), getInt("parms", "offset")))
    //                    {
    //                        getProperty(keyprop)->setState(OST::Error, true);
    //                    }
    //                }
    //            }
    //            if (keyelt == "extract")
    //            {
    //                if (getEltBool(keyprop, keyelt)->setValue(false, false))
    //                {
    //                    getProperty(keyprop)->setState(OST::Busy, true);
    //                    stats = _image->getStats();
    //                    _solver.ResetSolver(stats, _image->getImageBuffer());
    //                    connect(&_solver, &Solver::successSEP, this, &Dummy::OnSucessSEP);
    //                    connect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);
    //                    _solver.FindStars(_solver.stellarSolverProfiles[0]);
    //                }
    //            }
    //            if (keyelt == "solve")
    //            {
    //                if (getEltBool(keyprop, keyelt)->setValue(false, false))
    //                {
    //                    getProperty(keyprop)->setState(OST::Busy, true);
    //                    double ra, dec;
    //                    if (
    //                        !getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "DEC", dec)
    //                        || !getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "RA", ra)
    //                    )
    //                    {
    //                        getProperty(keyprop)->setState(OST::Error, true);
    //                        logWarning("Can't find mount device %1 solve aborted", {getString("devices", "mount")});
    //                    }
    //                    else
    //                    {
    //                        stats = _image->getStats();
    //                        _solver.ResetSolver(stats, _image->getImageBuffer());
    //                        QStringList folders;
    //                        folders.append(getString("parameters", "indexfolderpath"));
    //                        _solver.stellarSolver.setIndexFolderPaths(folders);
    //                        connect(&_solver, &Solver::successSolve, this, &Dummy::OnSucessSolve);
    //                        connect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);
    //                        _solver.stellarSolver.setSearchPositionInDegrees(ra * 360 / 24, dec);
    //                        _solver.SolveStars(_solver.stellarSolverProfiles[0]);
    //                    }
    //                }
    //            }
    //        }
    //        if (keyprop == "search")
    //        {
    //            if (keyelt == "searchbtn")
    //            {
    //                if (getEltBool(keyprop, keyelt)->setValue(true, true))
    //                {
    //                    updateSearchList();

    //                }
    //            }
    //        }
    //        if (keyprop == "lovevents")
    //        {
    //            if (keyelt == "btn")
    //            {
    //                getEltString("lovevents", "code")->lovClear();
    //                getEltString("lovevents", "code")->lovAdd("V1", "Value1");
    //                getEltString("lovevents", "code")->lovAdd("V2", "Value2");
    //            }
    //        }


    //    }

    //    if (e.type == "Fldelete")
    //    {
    //        double line = e.data[keyprop].toMap()["line"].toDouble();
    //        getStore()[keyprop]->deleteLine(line);
    //    }
    //    if (e.type == "Flcreate")
    //    {
    //        getStore()[keyprop]->newLine(e.data[keyprop].toMap()["elements"].toMap());
    //    }
    //    if (e.type == "Flupdate")
    //    {
    //        double line = e.data[keyprop].toMap()["line"].toDouble();
    //        getStore()[keyprop]->updateLine(line, e.data[keyprop].toMap()["elements"].toMap());
    //    }
    //    if (e.type == "Flselect" && keyprop == "results")
    //    {
    //        double line = e.data[keyprop].toMap()["line"].toDouble();
    //        QString code = getString("results", "code", line);
    //        float ra = getFloat("results", "RA", line);
    //        float dec = getFloat("results", "DEC", line);
    //        QString ns = getString("results", "NS", line);
    //        getEltString("selection", "code")->setValue(code);
    //        getEltFloat("selection", "RA")->setValue(ra);
    //        getEltFloat("selection", "DEC")->setValue(dec);
    //        getEltString("selection", "NS")->setValue(ns, true);
    //    }

    //}
}

void Dummy::newBLOB(INDI::PropertyBlob pblob)
{

    if (
        (QString(pblob.getDeviceName()) == _camera)
    )
    {


        delete _image;
        _image = new fileio();
        _image->loadBlob(pblob, 0);

        getProperty("actions2")->setState(OST::Ok, true);
        QList<fileio::Record> rec = _image->getRecords();
        stats = _image->getStats();
        _image->saveAsFITS(getWebroot() + "/" + getModuleName() + QString(pblob.getDeviceName()) + ".FITS", stats,
                           _image->getImageBuffer(),
                           FITSImage::Solution(), rec, false);

        QImage rawImage = _image->getRawQImage();
        rawImage.save(getWebroot() + "/" + getModuleName() + QString(pblob.getDeviceName()) + ".jpeg", "JPG", 100);
        OST::ImgData dta = _image->ImgStats();
        dta.mUrlJpeg = getModuleName() + QString(pblob.getDeviceName()) + ".jpeg";
        dta.mUrlFits = getModuleName() + QString(pblob.getDeviceName()) + ".FITS";
        dta.mAlternates.clear();
        dta.mAlternates.append(getModuleName() + QString(pblob.getDeviceName()) + ".jpeg");
        dta.mAlternates.append(getModuleName() + QString(pblob.getDeviceName()) + ".jpeg");
        dta.isSolved = false;
        getEltImg("testimage", "image1")->setValue(dta, true);

        emit newImage();
    }
    getProperty("actions2")->setState(OST::Ok, true);
    getProperty("testimage")->push();


}
void Dummy::updateProperty(INDI::Property property)
{
    if (strcmp(property.getName(), "CCD Simulator") == 0)
    {
        //qDebug() << "updateProperty " << property.getName();
    }
    if (strcmp(property.getName(), "CCD1") == 0)
    {
        //qDebug() << "updateProperty " << property.getName();
        newBLOB(property);
    }
}

void Dummy::OnSucessSEP()
{
    getProperty("actions2")->setState(OST::Ok, true);
    OST::ImgData dta = getEltImg("testimage", "image1")->value();
    dta.HFRavg = _solver.HFRavg;
    dta.starsCount = _solver.stars.size();
    getEltImg("testimage", "image1")->setValue(dta, true);
    disconnect(&_solver, &Solver::successSEP, this, &Dummy::OnSucessSEP);
    disconnect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);

}
void Dummy::OnNewImage()
{
    logDebug("New image");

    double ra, dec;
    if (
        !getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "DEC", dec)
        || !getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "RA", ra)
    )
    {
        logWarning("Can't find mount device %1 solve aborted", {getString("devices", "mount")});
    }
    else
    {

        //stats = _image->getStats();
        //_solver.ResetSolver(stats, _image->getImageBuffer());
        //QStringList folders;
        //folders.append(getString("parameters", "indexfolderpath"));
        //_solver.stellarSolver.setIndexFolderPaths(folders);
        //connect(&_solver, &Solver::successSolve, this, &Dummy::OnSucessSolve);
        //connect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);
        //_solver.stellarSolver.setSearchPositionInDegrees(ra * 360 / 24, dec);
        //_solver.SolveStars(_solver.stellarSolverProfiles[0]);
    }
}

void Dummy::OnSolveFinished()
{
    logInfo("Solver finished");
}
void Dummy::OnSucessSolve()
{
    logInfo("Solver sucess");

    if (_solver.stellarSolver.failed())
    {
        logWarning("Solver failed");
        getProperty("actions2")->setState(OST::Error, true);
        OST::ImgData dta = getEltImg("testimage", "image1")->value();
        dta.isSolved = false;
        dta.solverRA = 0;
        dta.solverDE = 0;
        getEltImg("testimage", "image1")->setValue(dta, true);
    }
    else
    {
        getProperty("actions2")->setState(OST::Ok, true);
        OST::ImgData dta = getEltImg("testimage", "image1")->value();
        dta.isSolved = true;
        dta.solverRA = _solver.stellarSolver.getSolution().ra;
        dta.solverDE = _solver.stellarSolver.getSolution().dec;
        getEltImg("testimage", "image1")->setValue(dta, true);
    }
    disconnect(&_solver, &Solver::successSolve, this, &Dummy::OnSucessSolve);
    disconnect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);

}
void Dummy::OnSolverLog(QString text)
{
    //sendMessage(text);
    qDebug() << text;
}
void Dummy::updateSearchList(void)
{
    QString s = getString("search", "search");
    logInfo("Searching %1", {s});

    getStore()["results"]->clearGrid();
    QList<catalogResult> results;

    searchCatalog(s, results);
    if (results.count() == 0)
    {
        logWarning("Searching %1 gives no result", {s});
        return;
    }

    int max = 20;
    if (max < results.count())
    {
        logWarning("updateSearchList more than %1 objects found, limiting result to %2", {max, max});
    }
    else
    {
        max = results.count();
    }

    for (int i = 0; i < max; i++)
    {
        getEltString("results", "catalog")->setValue(results[i].catalog);
        getEltString("results", "code")->setValue(results[i].code);
        getEltFloat("results", "RA")->setValue(results[i].RA);
        getEltString("results", "NS")->setValue(results[i].NS);
        getEltFloat("results", "DEC")->setValue(results[i].DEC);
        getEltFloat("results", "diam")->setValue(results[i].diam);
        getEltFloat("results", "mag")->setValue(results[i].mag);
        getEltString("results", "name")->setValue(results[i].name);
        getEltString("results", "alias")->setValue(results[i].alias);
        getStore()["results"]->push();
    }

}
void Dummy::newDevice(INDI::BaseDevice bd)
{
    Q_UNUSED(bd);
}

