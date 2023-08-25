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
    setOstElementValue("boolsRW0", "b1", true, false);
    setOstElementValue("boolsRW0", "b2", false, false);
    setOstElementValue("boolsRW0", "b3", false, false);
    setOstElementValue("boolsRW0", "b4", false, true);
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
    setOstElementAttribute("numbersRW", "n4", "step", 100, true);
    setOstElementAttribute("numbersRW", "n4", "min", -10000, true);
    setOstElementAttribute("numbersRW", "n4", "max", 10000, true);

    setOstElementValue("mixedRW", "b1", false, false);
    setOstElementValue("mixedRW", "b2", false, false);
    setOstElementValue("mixedRW", "b3", true, false);
    setOstElementValue("mixedRW", "n1", 10, false);
    setOstElementValue("mixedRW", "n2", 11, false);
    setOstElementValue("mixedRW", "t1", "Mixed text value", false);
    //saveAttributesToFile("dummy.json");
    _camera = getOstElementValue("devices", "camera").toString();

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
    setOstElementLov("extextRW", "extext4", "i1", "i1 label modified");
    //addOstElementLov("extextRW", "extext4", "i1", "i1 label modified"); // should give a warning
    addOstElementLov("extextRW", "extext4", "i3", "i3 label");
    addOstElementLov("extextRW", "extext4", "i4", "i4 label");
    deleteOstElementLov("extextRW", "extext4", "i4");
    //deleteOstElementLov("extextRW", "extext4", "i4"); // should give a warning
    //sendMessage(QString("lov element i3=") + getOstElementLov("extextRW", "extext4", "i3").toString());
    //sendMessage(QString("lov element inexistant") + getOstElementLov("extextRW", "extext4",
    //            "xxx").toString());// should give a warning
    //clearOstElementLov("extextRW", "extext4");
    OST::ValueInt *numbersRWn3 = static_cast<OST::ValueInt*>(getStore()["numbersRW"]->getValues()["n3"]);
    numbersRWn3->setState(OST::State::Error);
    numbersRWn3->setValue(999666);
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
            if (eventData[keyprop].toMap().contains("value"))
            {
                QVariant val = eventData[keyprop].toMap()["value"];
                //setOstPropertyValue(keyprop, val, true);
            }
            foreach(const QString &keyelt, eventData[keyprop].toMap()["elements"].toMap().keys())
            {
                setOstElementValue(keyprop, keyelt, eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"], true);
                if (keyprop == "devices")
                {
                    if (keyelt == "camera")
                    {
                        if (setOstElementValue(keyprop, keyelt, eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"], false))
                        {
                            setOstPropertyAttribute(keyprop, "status", IPS_OK, true);
                            _camera = getOstElementValue("devices", "camera").toString();
                        }
                    }
                }
                if (keyprop == "actions")
                {
                    if (keyelt == "blob")
                    {
                        if (setOstElementValue(keyprop, keyelt, false, false))
                        {
                            connectIndi();
                            setOstPropertyAttribute(keyprop, "status", IPS_OK, true);
                            connectDevice(_camera);
                            connectDevice(getOstElementValue("devices", "mount").toString());
                            setBLOBMode(B_ALSO, _camera.toStdString().c_str(), nullptr);
                            enableDirectBlobAccess(_camera.toStdString().c_str(), nullptr);
                        }
                    }
                    if (keyelt == "shoot")
                    {
                        if (setOstElementValue(keyprop, keyelt, false, false))
                        {
                            setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                            sendModNewNumber(_camera, "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 0.01 );
                            if (!sendModNewNumber(_camera, "CCD_EXPOSURE", "CCD_EXPOSURE_VALUE", getOstElementValue("parameters",
                                                  "exposure").toDouble()))
                            {
                                setOstPropertyAttribute(keyprop, "status", IPS_ALERT, true);
                            }
                        }
                    }
                    if (keyelt == "extract")
                    {
                        if (setOstElementValue(keyprop, keyelt, false, false))
                        {
                            setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
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
                            setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                            double ra, dec;
                            if (
                                !getModNumber(getOstElementValue("devices", "mount").toString(), "EQUATORIAL_EOD_COORD", "DEC", dec)
                                || !getModNumber(getOstElementValue("devices", "mount").toString(), "EQUATORIAL_EOD_COORD", "RA", ra)
                            )
                            {
                                setOstPropertyAttribute(keyprop, "status", IPS_ALERT, true);
                                sendMessage("Can't find mount device " + getOstElementValue("devices", "mount").toString() + " solve aborted");
                            }
                            else
                            {
                                setOstElementValue("imagevalues", "mountRA", ra * 360 / 24, false);
                                setOstElementValue("imagevalues", "mountDEC", dec, false);

                                stats = _image->getStats();
                                _solver.ResetSolver(stats, _image->getImageBuffer());
                                QStringList folders;
                                folders.append(getOstElementValue("parameters", "indexfolderpath").toString());
                                _solver.stellarSolver->setIndexFolderPaths(folders);
                                connect(&_solver, &Solver::successSolve, this, &Dummy::OnSucessSolve);
                                connect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);
                                _solver.stellarSolver->setSearchPositionInDegrees(ra * 360 / 24, dec);
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

            }

            if (eventType == "Fldelete")
            {
                double line = eventData[keyprop].toMap()["line"].toDouble();
                qDebug() << "dummy" << eventType << "-" << eventModule << "-" << eventKey << "-" << eventData << "line=" << line;
                deleteOstPropertyLine(keyprop, line);

            }
            if (eventType == "Flcreate")
            {
                qDebug() << "dummy" << eventType << "-" << eventModule << "-" << eventKey << "-" << eventData;
                newOstPropertyLine(keyprop, eventData);

            }
            if (eventType == "Flupdate")
            {
                double line = eventData[keyprop].toMap()["line"].toDouble();
                qDebug() << "dummy" << eventType << "-" << eventModule << "-" << eventKey << "-" << eventData;
                updateOstPropertyLine(keyprop, line, eventData);

            }
            if (eventType == "Flselect")
            {
                double line = eventData[keyprop].toMap()["line"].toDouble();
                qDebug() << "dummy" << eventType << "-" << eventModule << "-" << eventKey << "-" << eventData << "line=" << line;
                QString code = getOstElementLineValue("results", "code", line).toString();
                float ra = getOstElementLineValue("results", "RA", line).toFloat();
                float dec = getOstElementLineValue("results", "DEC", line).toFloat();
                QString ns = getOstElementLineValue("results", "NS", line).toString();
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

    if (
        (QString(pblob.getDeviceName()) == _camera)
    )
    {


        delete _image;
        _image = new fileio();
        _image->loadBlob(pblob);


        setOstPropertyAttribute("actions", "status", IPS_OK, true);
        setOstElementValue("imagevalues", "width", _image->getStats().width, false);
        setOstElementValue("imagevalues", "height", _image->getStats().height, false);
        setOstElementValue("imagevalues", "min", _image->getStats().min[0], false);
        setOstElementValue("imagevalues", "max", _image->getStats().max[0], false);
        setOstElementValue("imagevalues", "mean", _image->getStats().mean[0], false);
        setOstElementValue("imagevalues", "median", _image->getStats().median[0], false);
        setOstElementValue("imagevalues", "stddev", _image->getStats().stddev[0], false);
        setOstElementValue("imagevalues", "snr", _image->getStats().SNR, true);
        QList<fileio::Record> rec = _image->getRecords();
        stats = _image->getStats();
        _image->saveAsFITS(getWebroot() + "/" + getModuleName() + QString(pblob.getDeviceName()) + ".FITS", stats,
                           _image->getImageBuffer(),
                           FITSImage::Solution(), rec, false);

        QImage rawImage = _image->getRawQImage();
        rawImage.save(getWebroot() + "/" + getModuleName() + QString(pblob.getDeviceName()) + ".jpeg", "JPG", 100);
        setOstPropertyAttribute("testimage", "URL", getModuleName() + QString(pblob.getDeviceName()) + ".jpeg", true);

    }
    setOstPropertyAttribute("actions", "status", IPS_OK, true);


}
void Dummy::updateProperty(INDI::Property property)
{
    if (strcmp(property.getName(), "CCD Simulator") == 0)
    {
        qDebug() << "updateProperty " << property.getName();
    }
    if (strcmp(property.getName(), "CCD1") == 0)
    {
        qDebug() << "updateProperty " << property.getName();
        newBLOB(property);
    }
}

void Dummy::OnSucessSEP()
{
    setOstPropertyAttribute("actions", "status", IPS_OK, true);
    setOstElementValue("imagevalues", "hfravg", _solver.HFRavg, false);
    setOstElementValue("imagevalues", "starscount", _solver.stars.size(), true);
    disconnect(&_solver, &Solver::successSEP, this, &Dummy::OnSucessSEP);
    disconnect(&_solver, &Solver::solverLog, this, &Dummy::OnSolverLog);

}
void Dummy::OnSucessSolve()
{
    if (_solver.stellarSolver->failed())
    {
        sendMessage("Solver failed");
        setOstPropertyAttribute("actions", "status", IPS_ALERT, true);
        setOstPropertyAttribute("imagevalues", "status", IPS_ALERT, true);
        setOstElementValue("imagevalues", "solRA", 0, false);
        setOstElementValue("imagevalues", "solDEC", 0, true);
    }
    else
    {
        setOstPropertyAttribute("actions", "status", IPS_OK, true);
        setOstPropertyAttribute("imagevalues", "status", IPS_OK, true);
        setOstElementValue("imagevalues", "solRA", _solver.stellarSolver->getSolution().ra, false);
        setOstElementValue("imagevalues", "solDEC", _solver.stellarSolver->getSolution().dec, true);
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
    sendMessage("Searching " + getOstElementValue("search", "search").toString());
    resetOstElements("results");
    QList<catalogResult> results;
    searchCatalog(getOstElementValue("search", "search").toString(), results);
    if (results.count() == 0)
    {
        sendWarning("Searching " + getOstElementValue("search", "search").toString() + " gives no result");
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
        pushOstElements("results");
    }

}
