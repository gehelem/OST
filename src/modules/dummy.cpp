#include "dummy.h"

Dummy *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Dummy *basemodule = new Dummy(name, label, profile, availableModuleLibs);
    return basemodule;
}

Dummy::Dummy(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{

    //Q_INIT_RESOURCE(dummy);
    setClassName(QString(metaObject()->className()).toLower());
    loadOstPropertiesFromFile(":dummy.json");

    setModuleDescription("Dummy module to show what we can do and not");
    setModuleVersion("0.1");

    setOstElementValue("extextRO", "extext1", "Texte read only 1", false);
    setOstElementValue("extextRO", "extext2", "Texte read only 2", false);
    setOstElementValue("extextRO", "extext3", "Texte read only 3", false);
    setOstElementValue("extextRO", "extext4", "Texte read only 4", true);
    setOstElementValue("boolsRW0", "b1", true, true);
    //setOstElementValue("boolsRW0", "b2", false, false);
    //setOstElementValue("boolsRW0", "b3", false, false);
    //setOstElementValue("boolsRW0", "b4", false, true);
    setOstElementValue("boolsRW1", "b1", false, false);
    setOstElementValue("boolsRW1", "b2", false, false);
    setOstElementValue("boolsRW1", "b3", false, false);
    setOstElementValue("boolsRW1", "b4", false, true);
    setOstElementValue("boolsRW2", "b1", true, false);
    setOstElementValue("boolsRW2", "b2", true, false);
    setOstElementValue("boolsRW2", "b3", true, false);
    setOstElementValue("boolsRW2", "b4", false, true);
    setOstElementValue("numbersRW", "n1", 0, false);
    setOstElementValue("numbersRW", "n2", -1000, false);
    setOstElementValue("numbersRW", "n3", 3.14, false);
    setOstElementValue("numbersRW", "n4", -20.23, true);
    getEltInt("numbersRW", "n4")->setStep(100);
    getEltInt("numbersRW", "n4")->setMin(-10000);
    getEltInt("numbersRW", "n4")->setMax(10000);

    //setOstElementValue("mixedRW", "b1", false, false);
    //setOstElementValue("mixedRW", "b2", false, false);
    setOstElementValue("mixedRW", "b3", true, false);
    setOstElementValue("mixedRW", "n1", 10, false);
    setOstElementValue("mixedRW", "n2", 11, false);
    setOstElementValue("mixedRW", "t1", "Mixed text value", true);
    //saveAttributesToFile("dummy.json");

    //foreach(QString key, getAvailableModuleLibs().keys())
    //{
    //    if (!createOstProperty("mod" + key, "mod" + key, 0, "Module", "Ava"))
    //    {
    //        sendMessage("createOstProperty KO : " + key);
    //    }
    //}
    //setBLOBMode(B_ALSO, _camera.toStdString().c_str(), nullptr);
    //enableDirectBlobAccess(_camera.toStdString().c_str(), nullptr);
    //setBlobMode();
    getEltString("extextRW", "extext4")->lovUpdate("i1", "i1 label modified");
    //addOstElementLov("extextRW", "extext4", "i1", "i1 label modified"); // should give a warning
    getEltString("extextRW", "extext4")->lovAdd("i3", "i3 label");
    getEltString("extextRW", "extext4")->lovAdd("i4", "i4 label");
    getEltString("extextRW", "extext4")->lovDel("i4");
    getEltString("extextRW", "extext4")->lovDel("i4444");
    //deleteOstElementLov("extextRW", "extext4", "i4"); // should give a warning
    //sendMessage(QString("lov element i3=") + getOstElementLov("extextRW", "extext4", "i3").toString());
    //sendMessage(QString("lov element inexistant") + getOstElementLov("extextRW", "extext4",
    //            "xxx").toString());// should give a warning
    //clearOstElementLov("extextRW", "extext4");
    OST::PropertyMulti *n = getProperty("numbersRW");
    n->setState(OST::State::Error);
    //OST::ElementInt *numbersRWn3 = static_cast<OST::ElementInt*>(n->getElt("n3"));
    OST::ElementInt *numbersRWn3 = static_cast<OST::ElementInt*>(n->getElt("n3"));
    numbersRWn3->setValue(999666, true);

    //getText("extextRW", "extext1")->setValue("Value modified");
    //OST::ElementJsonDumper d;
    //getText("extextRW", "extext1")->accept(&d);
    //qDebug() << d.getResult();
    OST::PropertyMulti *p = getProperty("extextRW");
    p->setState(OST::State::Busy);
    static_cast<OST::ElementString*>(p->getElt("extext1"))->setValue("Value modified2", false);
    static_cast<OST::ElementString*>(p->getElt("extext4"))->lovUpdate("i3", "another label");

    dynprop = new OST::PropertyMulti("dynprop", "Dynamic", OST::Permission::ReadWrite, "Examples",
                                     "Dynamically instanciated", "", true, false);
    dynlight = new OST::ElementLight("Dyn light", "", "");
    dynlight->setValue(OST::State::Busy, true);
    dynprop->addElt("dynlight", dynlight);
    dyntext = new OST::ElementString("Dyn text", "", "");
    dynprop->addElt("dyntext", dyntext);
    createProperty("dynprop", dynprop);
    dynprop->setState(OST::State::Busy);
    dyntext->setValue("Okydoky", false);
    dynlight->setValue(OST::State::Ok, true);

    dynbool = new OST::ElementBool("Dyn bool", "", "");
    dynprop->addElt("dynbool", dynbool);
    dynbool->setValue(false, false);

    dyntext2 = getEltString("extextRW", "extext1");
    dyntext2->setValue("torototo", true);

    giveMeADevice("camera", "Multipurpose camera", INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("mount", "Multipurpose mount", INDI::BaseDevice::TELESCOPE_INTERFACE);
    defineMeAsFocuser();
    defineMeAsGuider();
    defineMeAsSequencer();
    defineMeAsNavigator();

}

Dummy::~Dummy()
{
    //Q_CLEANUP_RESOURCE(dummy);
}

void Dummy::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                              const QVariantMap &eventData)
{
    if (getModuleName() == eventModule )
    {
        foreach(const QString &keyprop, eventData.keys())
        {
            foreach(const QString &keyelt, eventData[keyprop].toMap()["elements"].toMap().keys())
            {
                //setOstElementValue(keyprop, keyelt, eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"], true);
                if (keyprop == "dynprop")
                {
                    if (keyelt == "dyntext")
                    {

                        dyntext->setValue(eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"].toString(), true);
                    }
                    if (keyelt == "dynbool")
                    {
                        bool val = eventData["dynprop"].toMap()["elements"].toMap()["dynbool"].toMap()["value"].toBool();
                        if (val)
                        {

                            dyntext->setValue("Changed from dynamic switch", true);
                        }

                    }
                }
                if (keyprop == "boolsRW2")
                {
                    if (keyelt == "b1")
                    {
                        //refreshDeviceslovs();
                    }
                }
                if (keyprop == "devices")
                {
                    if (keyelt == "camera")
                    {
                        if (setOstElementValue(keyprop, keyelt, eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"], false))
                        {
                            getProperty(keyprop)->setState(OST::Ok);
                            _camera = getString("devices", "camera");
                        }
                    }
                }
                if (keyprop == "actions2")
                {
                    if (keyelt == "blob")
                    {
                        if (setOstElementValue(keyprop, keyelt, false, false))
                        {
                            connectIndi();
                            getProperty(keyprop)->setState(OST::Ok);
                            _camera = getString("devices", "camera");
                            connectDevice(_camera);
                            setBLOBMode(B_ALSO, _camera.toStdString().c_str(), nullptr);
                            enableDirectBlobAccess(_camera.toStdString().c_str(), nullptr);
                        }
                    }
                    if (keyelt == "shoot")
                    {
                        if (setOstElementValue(keyprop, keyelt, false, false))
                        {
                            getProperty(keyprop)->setState(OST::Busy);
                            _camera = getString("devices", "camera");
                            if (_camera == "CCD Simulator")
                            {
                                sendModNewNumber(_camera, "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 0.01 );
                            }
                            if (!requestCapture(_camera, getFloat("parms", "exposure"), getInt("parms", "gain"), getInt("parms", "offset")))
                            {
                                getProperty(keyprop)->setState(OST::Error);
                            }
                        }
                    }
                    if (keyelt == "extract")
                    {
                        if (setOstElementValue(keyprop, keyelt, false, false))
                        {
                            getProperty(keyprop)->setState(OST::Busy);
                            stats = _image->getStats();
                            _solver.ResetSolver(stats, _image->getImageBuffer());
                            connect(&_solver, &Solver::successSEP, this, &Dummy::OnSucessSEP);
                            connect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);
                            _solver.FindStars(_solver.stellarSolverProfiles[0]);
                        }
                    }
                    if (keyelt == "solve")
                    {
                        if (setOstElementValue(keyprop, keyelt, false, false))
                        {
                            getProperty(keyprop)->setState(OST::Busy);
                            double ra, dec;
                            if (
                                !getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "DEC", dec)
                                || !getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "RA", ra)
                            )
                            {
                                getProperty(keyprop)->setState(OST::Error);
                                sendMessage("Can't find mount device " + getString("devices", "mount") + " solve aborted");
                            }
                            else
                            {
                                stats = _image->getStats();
                                _solver.ResetSolver(stats, _image->getImageBuffer());
                                QStringList folders;
                                folders.append(getString("parameters", "indexfolderpath"));
                                _solver.stellarSolver.setIndexFolderPaths(folders);
                                connect(&_solver, &Solver::successSolve, this, &Dummy::OnSucessSolve);
                                connect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);
                                _solver.stellarSolver.setSearchPositionInDegrees(ra * 360 / 24, dec);
                                _solver.SolveStars(_solver.stellarSolverProfiles[0]);
                            }
                        }
                    }
                }
                if (keyprop == "search")
                {
                    if (keyelt == "searchbtn")
                    {
                        if (setOstElementValue(keyprop, keyelt, true, true))
                        {
                            updateSearchList();

                        }
                    }
                }
                if (keyprop == "lovevents")
                {
                    if (keyelt == "btn")
                    {
                        getEltString("lovevents", "code")->lovClear();
                        getEltString("lovevents", "code")->lovAdd("V1", "Value1");
                        getEltString("lovevents", "code")->lovAdd("V2", "Value2");
                    }
                }


            }

            if (eventType == "Fldelete")
            {
                double line = eventData[keyprop].toMap()["line"].toDouble();
                qDebug() << "dummy" << eventType << "-" << eventModule << "-" << eventKey << "-" << eventData << "line=" << line;
                getStore()[keyprop]->deleteLine(line);
            }
            if (eventType == "Flcreate")
            {
                qDebug() << "dummy" << eventType << "-" << eventModule << "-" << eventKey << "-" << eventData;
                getStore()[keyprop]->newLine(eventData[keyprop].toMap()["elements"].toMap());

            }
            if (eventType == "Flupdate")
            {
                double line = eventData[keyprop].toMap()["line"].toDouble();
                qDebug() << "dummy" << eventType << "-" << eventModule << "-" << eventKey << "-" << eventData;
                getStore()[keyprop]->updateLine(line, eventData[keyprop].toMap()["elements"].toMap());

            }
            if (eventType == "Flselect" && keyprop == "results")
            {
                double line = eventData[keyprop].toMap()["line"].toDouble();
                qDebug() << "dummy" << eventType << "-" << eventModule << "-" << eventKey << "-" << eventData << "line=" << line;
                QString code = getString("results", "code", line);
                float ra = getFloat("results", "RA", line);
                float dec = getFloat("results", "DEC", line);
                QString ns = getString("results", "NS", line);
                setOstElementValue("selection", "code", code, false);
                setOstElementValue("selection", "RA", ra, false);
                setOstElementValue("selection", "DEC", dec, false);
                setOstElementValue("selection", "NS", ns, true);

            }

        }
    }
}

void Dummy::newBLOB(INDI::PropertyBlob pblob)
{
    refreshDeviceslovs(pblob.getDeviceName());

    if (
        (QString(pblob.getDeviceName()) == _camera)
    )
    {


        delete _image;
        _image = new fileio();
        _image->loadBlob(pblob, 0);

        getProperty("actions2")->setState(OST::Ok);
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
        dta.isSolved = false;
        getEltImg("testimage", "image1")->setValue(dta, true);

    }
    getProperty("actions2")->setState(OST::Ok);
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
    getProperty("actions2")->setState(OST::Ok);
    OST::ImgData dta = getEltImg("testimage", "image1")->value();
    dta.HFRavg = _solver.HFRavg;
    dta.starsCount = _solver.stars.size();
    getEltImg("testimage", "image1")->setValue(dta, true);
    disconnect(&_solver, &Solver::successSEP, this, &Dummy::OnSucessSEP);
    disconnect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);

}
void Dummy::OnSucessSolve()
{
    if (_solver.stellarSolver.failed())
    {
        sendMessage("Solver failed");
        getProperty("actions2")->setState(OST::Error);
        OST::ImgData dta = getEltImg("testimage", "image1")->value();
        dta.isSolved = false;
        dta.solverRA = 0;
        dta.solverDE = 0;
        getEltImg("testimage", "image1")->setValue(dta, true);
    }
    else
    {
        getProperty("actions2")->setState(OST::Ok);
        OST::ImgData dta = getEltImg("testimage", "image1")->value();
        dta.isSolved = true;
        dta.solverRA = _solver.stellarSolver.getSolution().ra;
        dta.solverDE = _solver.stellarSolver.getSolution().dec;
        getEltImg("testimage", "image1")->setValue(dta, true);



    }
    disconnect(&_solver, &Solver::successSolve, this, &Dummy::OnSucessSolve);
    disconnect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);

}
void Dummy::OnSolverLog(QString &text)
{
    sendMessage(text);
}
void Dummy::updateSearchList(void)
{
    QString s = getString("search", "search");
    sendMessage("Searching " + s);

    getStore()["results"]->clearGrid();
    QList<catalogResult> results;

    searchCatalog(s, results);
    if (results.count() == 0)
    {
        sendWarning("Searching " + s + " gives no result");
        return;
    }

    int max = 20;
    if (max < results.count())
    {
        sendWarning("updateSearchList more than " + QString::number(max) + " objects found, limiting result to " + QString::number(
                        max));
    }
    else
    {
        max = results.count();
    }

    for (int i = 0; i < max; i++)
    {
        setOstElementValue("results", "catalog", results[i].catalog, false);
        setOstElementValue("results", "code", results[i].code, false);
        setOstElementValue("results", "RA", results[i].RA, false);
        setOstElementValue("results", "NS", results[i].NS, false);
        setOstElementValue("results", "DEC", results[i].DEC, false);
        setOstElementValue("results", "diam", results[i].diam, false);
        setOstElementValue("results", "mag", results[i].mag, false);
        setOstElementValue("results", "name", results[i].name, false);
        setOstElementValue("results", "alias", results[i].alias, false);
        getStore()["results"]->push();
    }

}
void Dummy::newDevice(INDI::BaseDevice bd)
{
    Q_UNUSED(bd);
}
