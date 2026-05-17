#include "navigator.h"
#include <QPainter>
#include "version.cc"

Navigator *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Navigator *basemodule = new Navigator(name, label, profile, availableModuleLibs);
    return basemodule;
}

Navigator::Navigator(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs),
      mMaxIterations(0),
      mCurrentIteration(0),
      mToleranceArcsec(0.0),
      mTargetRA(0.0),
      mTargetDEC(0.0),
      mTargetRAnow(0.0),
      mTargetDECnow(0.0),
      mWaitingSlew(false)
{

    loadOstPropertiesFromFile(":navigator.json");

    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Navigator module");
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("template", "navigator");


    giveMeADevice("camera", "Camera", INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("mount", "Mount", INDI::BaseDevice::TELESCOPE_INTERFACE);
    giveMeADevice("gps", "GPS", INDI::BaseDevice::GPS_INTERFACE);

    defineMeAsNavigator();

    connectIndi();
    connectAllDevices();

    connect(this, &Navigator::newImage, this, &Navigator::OnNewImage);

    connect(&stellarSolver, &StellarSolver::logOutput, this, &Navigator::OnSolverLog);
    connect(&stellarSolver, &StellarSolver::ready, this, &Navigator::OnSucessSolve);

}

Navigator::~Navigator()
{

}
void Navigator::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev == OST::ExtEvType::DP)
    {
        logDebug("Message DP %1", {event.prpkey});
    }

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        if (event.eltkey == "gototarget")
        {
            mState = "running";
            mCurrentIteration = 0;
            convertSelection();
            // Load centering parameters from properties
            mMaxIterations = getInt("centeringparams", "maxiterations");
            mToleranceArcsec = getFloat("centeringparams", "tolerance");
            // Store target coordinates for centering loop
            mTargetRAnow = getFloat("selectnow", "RA");
            mTargetDECnow = getFloat("selectnow", "DEC");
            logInfo(QString("Starting goto with centering: max %1 iterations, tolerance %2\"")
                    .arg(mMaxIterations).arg(mToleranceArcsec, 0, 'f', 1));
            initIndi();
            slewToSelection();
        }
        if (event.eltkey == "abortnavigator")
        {
            stellarSolver.abort();
            mState = "idle";
            mCurrentIteration = 0;
            getProperty(event.prpkey)->setState(OST::Ok, true);
        }
        if (event.eltkey == "addtoplanner")
        {
            getProperty(event.prpkey)->setState(OST::Ok, true);
            addTargeToPlanner();
        }
    }

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "popcat")
    {
        if (event.eltkey == "go")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                getProperty(event.prpkey)->setState(OST::Busy, true);
                /* we should avoid this, and do this in a different thread. We'll see later */
                populateCatalog(":" + getString("popcat", "catlg") + ".txt", getString("popcat", "catlg"));
                getProperty(event.prpkey)->setState(OST::Ok, true);
            }

        }
    }

    if (event.ev == OST::ExtEvType::GF && event.prpkey == "results")
    {
        getProperty("results")->fetchLine(event.line);
        QString code = getString("results", "code");
        float ra = getFloat("results", "RA");
        float dec = getFloat("results", "DEC");
        QString ns = getString("results", "NS");
        getEltString("target", "targetname")->setValue(code);
        getEltFloat("target", "targetra")->setValue(ra);
        getEltFloat("target", "targetde")->setValue(dec, true);
        convertSelection();
    }

    if (event.ev == OST::ExtEvType::J2 && event.prpkey == "search" && event.eltkey == "name")
    {
        updateSearchList();
    }


    //    if (getModuleName() == e.module)
    //    {
    //        foreach(const QString &keyprop, e.data.keys())
    //        {
    //            foreach(const QString &keyelt, e.data[keyprop].toMap()["elements"].toMap().keys())
    //            {
    //
    //                if (keyprop == "popcat")
    //                {
    //                    if (keyelt == "go")
    //                    {
    //                        if (getEltBool(keyprop, keyelt)->setValue(true, true))
    //                        {
    //                            getProperty(keyprop)->setState(OST::Busy, true);
    //                            /* we should avoid this, and do this in a different thread. We'll see later */
    //                            populateCatalog(":" + getString("popcat", "catlg") + ".txt", getString("popcat", "catlg"));
    //                            getProperty(keyprop)->setState(OST::Ok, true);
    //                        }
    //                    }
    //                    if (keyelt == "catlg")
    //                    {
    //                        QString s = e.data[keyprop].toMap()["elements"].toMap()[keyelt].toString();
    //                        qDebug() << "update catlg " << s;
    //                        getEltString("popcat", "catlg")->setValue(s, true);
    //                        getProperty("popcat")->enable();
    //                    }
    //
    //                }
    //
    //                if (keyprop == "actions")
    //                {
    //                    getProperty(keyprop)->setState(OST::Busy, true);
    //                    if (keyelt == "gototarget")
    //                    {
    //                        mState = "running";
    //                        mCurrentIteration = 0;
    //                        convertSelection();
    //                        // Load centering parameters from properties
    //                        mMaxIterations = getInt("centeringparams", "maxiterations");
    //                        mToleranceArcsec = getFloat("centeringparams", "tolerance");
    //                        // Store target coordinates for centering loop
    //                        mTargetRAnow = getFloat("selectnow", "RA");
    //                        mTargetDECnow = getFloat("selectnow", "DEC");
    //                        logInfo(QString("Starting goto with centering: max %1 iterations, tolerance %2\"")
    //                                .arg(mMaxIterations).arg(mToleranceArcsec, 0, 'f', 1));
    //                        initIndi();
    //                        slewToSelection();
    //                    }
    //                    if (keyelt == "abortnavigator")
    //                    {
    //                        stellarSolver.abort();
    //                        mState = "idle";
    //                        mCurrentIteration = 0;
    //                        getProperty(keyprop)->setState(OST::Ok, true);
    //                    }
    //                    if (keyelt == "addtoplanner")
    //                    {
    //                        addTargeToPlanner();
    //                    }
    //
    //                }
    //            }
    //            if (e.type == "Flselect")
    //            {
    //                double line = e.data[keyprop].toMap()["line"].toDouble();
    //                getProperty("results")->fetchLine(line);
    //                QString code = getString("results", "code");
    //                float ra = getFloat("results", "RA");
    //                float dec = getFloat("results", "DEC");
    //                QString ns = getString("results", "NS");
    //                getEltString("actions", "targetname")->setValue(code);
    //                getEltFloat("actions", "targetra")->setValue(ra);
    //                getEltFloat("actions", "targetde")->setValue(dec, true);
    //                convertSelection();
    //            }
    //        }
    //
    //    }
}

void Navigator::newBLOB(INDI::PropertyBlob pblob)
{
    if (
        (QString(pblob.getDeviceName()) == getString("devices", "camera")) && (mState != "idle")
    )
    {
        getProperty("actions")->setState(OST::Idle, true);
        delete pImage;
        pImage = new fileio();
        pImage->loadBlob(pblob, 64);
        QImage rawImage = pImage->getRawQImage();
        QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
        im.setColorTable(rawImage.colorTable());
        im.save(getWebroot()  + "/" + getModuleName() + ".jpeg", "JPG", 100);
        OST::ImgData dta = pImage->ImgStats();
        dta.mUrlJpeg = getModuleName() + ".jpeg";
        dta.isSolved = false;
        getEltImg("image", "image")->setValue(dta, true);

        emit newImage();
    }
}
void Navigator::OnNewImage()
{
    logInfo("Solve new image");
    double ra, dec, pix, ech;
    if (
        !getModNumber(getString("devices", "camera"), "CCD_INFO", "CCD_PIXEL_SIZE", pix)
    )
    {
        logInfo("Can't find camera device %1 solve aborted", {getString("devices", "camera")});
    }
    ech = 206 * pix / getFloat("optic", "fl");

    if (
        !getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "DEC", dec)
        || !getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "RA", ra)
    )
    {
        logInfo("Can't find mount device %1 solve aborted", {getString("devices", "mount")});
    }
    else
    {
        double wra = ra * 360 / 24;
        double wde = dec;
        stellarSolver.loadNewImageBuffer(pImage->getStats(), pImage->getImageBuffer());
        QStringList folders;
        folders.append("/usr/share/astrometry/");
        stellarSolver.setIndexFolderPaths(folders);
        stellarSolver.setProperty("UseScale", true);
        stellarSolver.setSearchScale(ech * 0.8, ech * 1.2, ScaleUnits::ARCSEC_PER_PIX);
        stellarSolver.setProperty("UsePosition", true);
        stellarSolver.setSearchPositionInDegrees(wra, wde);
        stellarSolver.setParameters(StellarSolver::getBuiltInProfiles()[SSolver::Parameters::DEFAULT]);
        stellarSolver.setProperty("ProcessType", SOLVE);
        stellarSolver.setProperty("ExtractorType", EXTRACTOR_INTERNAL);
        stellarSolver.setProperty("SolverType", SOLVER_STELLARSOLVER);
        stellarSolver.setLogLevel(LOG_MSG);
        stellarSolver.setSSLogLevel(LOG_NORMAL);
        stellarSolver.setProperty("LogToFile", true);
        stellarSolver.setProperty("LogFileName", getWebroot() + "/navigator_solver.log");
        stellarSolver.start();
    }
}
void Navigator::onUpdateProperty(INDI::Property property)
{
    //if (mState == "idle") return;

    if (strcmp(property.getName(), "CCD1") == 0)
    {
        newBLOB(property);
    }
    if (
        (property.getDeviceName() == getString("devices", "camera"))
        &&  (property.getState() == IPS_ALERT)
    )
    {
        logWarning("cameraAlert");
        emit cameraAlert();
    }
    if (
        (property.getDeviceName() == getString("devices", "mount"))
        &&  (property.getName()   == std::string("EQUATORIAL_EOD_COORD"))
    )
    {
        // Update mount position
        INDI::PropertyNumber prop = property;
        getEltFloat("mountposition", "RA")->setValue(prop.findWidgetByName("RA")->value, false);
        getEltFloat("mountposition", "DEC")->setValue(prop.findWidgetByName("DEC")->value, true);
    }
    if (
        (property.getDeviceName() == getString("devices", "gps"))
        &&  (property.getName()   == std::string("GEOGRAPHIC_COORD"))
    )
    {
        // Update GPS Coords
        INDI::PropertyNumber prop = property;
        getEltFloat("gpslocation", "alt")->setValue(prop.findWidgetByName("ELEV")->value, false);
        getEltFloat("gpslocation", "lat")->setValue(prop.findWidgetByName("LAT")->value, false);
        getEltFloat("gpslocation", "lon")->setValue(prop.findWidgetByName("LONG")->value, true);
    }
    if (
        (property.getDeviceName() == getString("devices", "gps"))
        &&  (property.getName()   == std::string("TIME_UTC"))
    )
    {
        // Update GPS Time and date
        INDI::PropertyText prop = property;
        QDateTime dt;
        QString strdt = prop.findWidgetByName("UTC")->text;
        QString offset = prop.findWidgetByName("OFFSET")->text;
        dt = dt.fromString(strdt, Qt::ISODate);
        getEltDate("gpstime", "date")->setValue(dt.date(), false);
        getEltTime("gpstime", "time")->setValue(dt.time(), false);
        getEltFloat("gpstime", "offset")->setValue(offset.toFloat(), true);
    }
    if (
        (property.getDeviceName() == getString("devices", "mount"))
        &&  (property.getName()   == std::string("EQUATORIAL_EOD_COORD"))
        &&  (property.getState() == IPS_OK)
        && mState == "running"
    )
    {

        if (mWaitingSlew)
        {
            mWaitingSlew = false;
            logInfo("Slew finished");
            Shoot();
        }
    }
}
void Navigator::Shoot()
{
    if (!setFocalLengthAndDiameter())
    {
        emit abort();
        return;
    }
    if (!requestCapture(getString("devices", "camera"), getFloat("parms", "exposure"), getInt("parms", "gain"), getInt("parms",
                        "offset")))
    {
        emit abort();
        return;
    }
    getProperty("actions")->setState(OST::Busy, true);
}
void Navigator::initIndi()
{
    connectIndi();
    connectDevice(getString("devices", "camera"));
    connectDevice(getString("devices", "mount"));
    setBLOBMode(B_ALSO, getString("devices", "camera").toStdString().c_str(), nullptr);
    if (getString("devices", "camera") == "CCD Simulator")
    {
        //sendModNewNumber(getString("devices", "camera"), "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 0.01 );
    }
    enableDirectBlobAccess(getString("devices", "camera").toStdString().c_str(), nullptr);

    // Update GPS Coords
    INDI::PropertyNumber pn = getDevice(getString("devices", "gps").toStdString().c_str()).getProperty("GEOGRAPHIC_COORD",
                              INDI_NUMBER);
    getEltFloat("gpslocation", "alt")->setValue(pn.findWidgetByName("ELEV")->value, false);
    getEltFloat("gpslocation", "lat")->setValue(pn.findWidgetByName("LAT")->value, false);
    getEltFloat("gpslocation", "lon")->setValue(pn.findWidgetByName("LONG")->value, true);
    // Update GPS Time and date
    INDI::PropertyText pt = getDevice(getString("devices", "gps").toStdString().c_str()).getProperty("TIME_UTC", INDI_TEXT);
    QDateTime dt;
    QString strdt = pt.findWidgetByName("UTC")->text;
    QString offset = pt.findWidgetByName("OFFSET")->text;
    dt = dt.fromString(strdt, Qt::ISODate);
    getEltDate("gpstime", "date")->setValue(dt.date(), false);
    getEltTime("gpstime", "time")->setValue(dt.time(), false);
    getEltFloat("gpstime", "offset")->setValue(offset.toFloat(), true);



}
void Navigator::OnSolverLog(QString text)
{
    //logInfo(text);
    //qDebug() << text;
}
void Navigator::OnSucessSolve()
{
    if (stellarSolver.failed())
    {
        logError("Image NOT solved - Centering aborted");
        stellarSolver.abort();
        getProperty("actions")->setState(OST::Error, true);
        setStateEvent(OST::Error, "error", "solverfailed", "Solver failed");
        mState = "idle";
        mCurrentIteration = 0;
        return;
    }
    // Get solved coordinates
    double solvedRA = stellarSolver.getSolution().ra * 24 / 360;
    double solvedDEC = stellarSolver.getSolution().dec;
    // Update image with solve data
    OST::ImgData dta = pImage->ImgStats();
    dta.mUrlJpeg = getModuleName() + ".jpeg";
    dta.solverRA = solvedRA;
    dta.solverDE = solvedDEC;
    dta.isSolved = true;
    getEltImg("image", "image")->setValue(dta, true);

    mCurrentIteration++;

    // Check if centering is satisfactory
    if (checkCentering(solvedRA, solvedDEC, mTargetRA, mTargetDEC))
    {
        logInfo(QString("Centering successful after %1 iteration(s) - within tolerance")
                .arg(mCurrentIteration));
        getProperty("actions")->setState(OST::Ok, true);
        setStateEvent(OST::Ok, "ready", "navigatordone", "Navigator done");
        getEltBool("actions", "gototarget")->setValue(false, false);
        getEltBool("actions", "abortnavigator")->setValue(false, false);
        getEltBool("actions", "addtoplanner")->setValue(false, true);
        syncMountIfNeeded(solvedRA, solvedDEC);
        mState = "idle";
        mCurrentIteration = 0;
        return;
    }

    // Check max iterations
    if (mCurrentIteration >= mMaxIterations)
    {
        logError(QString("Centering failed after %1 iterations - tolerance not reached")
                 .arg(mMaxIterations));
        getProperty("actions")->setState(OST::Error, true);
        setStateEvent(OST::Error, "error", "navigatorfailed", "Navigator failed");
        mState = "idle";
        mCurrentIteration = 0;
        return;
    }

    // Need correction - continue centering loop
    logInfo(QString("Centering iteration %1/%2 - applying correction")
            .arg(mCurrentIteration).arg(mMaxIterations));
    setStateEvent(OST::Busy, "centering", "correction", "Correction needed");

    correctOffset(solvedRA, solvedDEC);
}
void Navigator::updateSearchList(void)
{
    logInfo("Searching %1", {getString("search", "name")});
    getProperty("results")->clearGrid();
    QList<catalogResult> results;
    searchCatalog(getString("search", "name"), results);
    if (results.count() == 0)
    {
        logWarning("Searching %1 gives no result", {getString("search", "name")});
        return;
    }
    logInfo("Searching %1 gives %2 results", {getString("search", "name"), QString::number(results.count())});

    int max = 20;
    if (max < results.count())
    {
        logWarning("updateSearchList more than %1 objects found, limiting result to %2", {QString::number(max), QString::number(max)});
    }
    else
    {
        max = results.count();
    }

    for (int i = 0; i < max; i++)
    {
        //qDebug() << results[i].name << results[i].RA;
        getEltString("results", "catalog")->setValue(results[i].catalog);
        getEltString("results", "code")->setValue(results[i].code);
        getEltString("results", "NS")->setValue(results[i].NS);
        getEltString("results", "name")->setValue(results[i].name);
        getEltString("results", "alias")->setValue(results[i].alias);
        getEltFloat("results", "RA")->setValue(results[i].RA);
        getEltFloat("results", "DEC")->setValue(results[i].DEC);
        getEltFloat("results", "diam")->setValue(results[i].diam);
        getEltFloat("results", "mag")->setValue(results[i].mag);
        getProperty("results")->push();
    }

}
void Navigator::slewToSelection(void)
{
    QString mount = getString("devices", "mount");
    QString cam  = getString("devices", "camera");
    INDI::BaseDevice dp = getDevice(mount.toStdString().c_str());
    if (!dp.isValid())
    {
        logError("Error - unable to find %1 device. Aborting.", {mount});
        setStateEvent(OST::Error, "error", "devicefailed", "Device failed");
        return;
    }
    INDI::PropertyNumber prop = dp.getProperty("EQUATORIAL_EOD_COORD");
    if (!prop.isValid())
    {
        logError("Error - unable to find %1/EQUATORIAL_EOD_COORD property. Aborting.", {mount});
        setStateEvent(OST::Error, "error", "devicefailed", "Device failed");
        return;
    }
    prop.findWidgetByName("RA")->value = mTargetRAnow;
    prop.findWidgetByName("DEC")->value = mTargetDECnow;

    mWaitingSlew = true;
    sendNewNumber(prop);
    logInfo("Slewing to %1", {getString("target", "targetname")});
    setStateEvent(OST::Busy, "slewing", "slew", "Slew");
}
void Navigator::convertSelection(void)
{
    QString code = getString("target", "targetname");
    mTargetRA = getFloat("target", "targetra");
    mTargetDEC = getFloat("target", "targetde");
    double jd = ln_get_julian_from_sys();
    INDI::IEquatorialCoordinates j2000pos;
    INDI::IEquatorialCoordinates observed;
    j2000pos.declination = mTargetDEC;
    j2000pos.rightascension = mTargetRA;

    INDI::J2000toObserved(&j2000pos, jd, &observed);
    mTargetRAnow = observed.rightascension;
    mTargetDECnow = observed.declination;
    getEltString("selectnow", "jd")->setValue(""); // we'll see that later
    getEltString("selectnow", "code")->setValue(code);
    getEltFloat("selectnow", "RA")->setValue(observed.rightascension);
    getEltFloat("selectnow", "DEC")->setValue(observed.declination, true);

}

bool Navigator::checkCentering(double solvedRA, double solvedDEC, double targetRA, double targetDEC)
{
    // Calculate angular distance in arcseconds
    //double deltaRA = (solvedRA * 360 / 24 - targetRA * 360 / 24) * cos(solvedDEC * M_PI / 180.0) * 3600;
    double deltaRA = (solvedRA * 360 / 24 - targetRA * 360 / 24) * 3600;
    double deltaDEC = (solvedDEC - targetDEC) * 3600 ;

    // Total angular distance in arcseconds
    double distance = sqrt(deltaRA * deltaRA + deltaDEC * deltaDEC);


    // Check tolerance (convert to arcsec from JSON parameter if needed)
    return (distance <= mToleranceArcsec);
}

void Navigator::correctOffset(double solvedRA, double solvedDEC)
{
    // Calculate correction offset
    //double deltaRA = (solvedRA  - mTargetRA) * cos(solvedDEC * M_PI / 180.0);
    double deltaRA = solvedRA  - mTargetRA;
    double deltaDEC = solvedDEC - mTargetDEC;
    logInfo("Offset correction : dRA %1arcs dDEC %2arcs", {deltaRA, deltaDEC});

    // Get current mount position
    QString mount = getString("devices", "mount");
    INDI::BaseDevice dp = getDevice(mount.toStdString().c_str());
    if (!dp.isValid())
    {
        logError("Error - unable to find %1 device for correction.", {mount});
        setStateEvent(OST::Error, "error", "devicefailed", "Device failed");
        mState = "idle";
        return;
    }

    INDI::PropertyNumber prop = dp.getProperty("EQUATORIAL_EOD_COORD");
    if (!prop.isValid())
    {
        logError("Error - unable to find mount coordinates for correction.");
        setStateEvent(OST::Error, "error", "devicefailed", "Device failed");
        mState = "idle";
        return;
    }

    // Send corrected position to mount (RA in hours, DEC in degrees)
    prop.findWidgetByName("RA")->value  = prop.findWidgetByName("RA")->value - deltaRA;
    prop.findWidgetByName("DEC")->value = prop.findWidgetByName("DEC")->value - deltaDEC;
    mWaitingSlew = true;
    sendNewNumber(prop);

    // Mount will trigger updateProperty when slew is complete
    // which will call Shoot() again for next iteration
}
void Navigator::syncMountIfNeeded(double solvedRA, double solvedDEC)
{

    setStateEvent(OST::Busy, "synchronise", "syncrequest", "Sync requested");
    // Get mount device
    QString mount = getString("devices", "mount");
    INDI::BaseDevice dp = getDevice(mount.toStdString().c_str());
    if (!(dp.getDriverInterface() & INDI::BaseDevice::TELESCOPE_INTERFACE))
    {
        logError("Device %1 has no telescope interface", {mount});
        setStateEvent(OST::Error, "error", "devicefailed", "Device failed");
        return;
    }

    if (!sendModNewSwitch(mount, "ON_COORD_SET", "SYNC", ISS_ON)) return;

    INDI::PropertyNumber prop = dp.getProperty("EQUATORIAL_EOD_COORD");
    if (!prop.isValid())
    {
        logError("Error - unable to find %1 / EQUATORIAL_EOD_COORD property. Aborting.", {mount});
        setStateEvent(OST::Error, "error", "devicefailed", "Device failed");
        return;
    }
    double jd = ln_get_julian_from_sys();
    INDI::IEquatorialCoordinates j2000pos;
    INDI::IEquatorialCoordinates observed;
    j2000pos.rightascension = solvedRA;
    j2000pos.declination = solvedDEC;
    INDI::J2000toObserved(&j2000pos, jd, &observed);

    prop.findWidgetByName("DEC")->value = observed.declination;
    prop.findWidgetByName("RA")->value = observed.rightascension;
    sendNewNumber(prop);

    //restore slew
    if (!sendModNewSwitch(mount, "ON_COORD_SET", "SLEW", ISS_ON)) return;

    logInfo("Mount successfully synchronized with solved field");


}
void Navigator::addTargeToPlanner()
{
    //{"evt":"Flcreate","mod":"Planner","dta":{"planning":{"elements":{"object":"TT","ra":10,"dec":20,"profile":"default"}}}}
    QVariantMap eltData;
    eltData["object"] = getString("target", "targetname");
    eltData["ra"] = getFloat("target", "targetra");
    eltData["dec"] = getFloat("target", "targetde");
    eltData["profile"] = "default";
    logInfo("Current target sent to %1", {getString("parms", "plannermodule")});

    otherModuleCreateLine(getString("slaves", "plannermodule"), "planning", eltData);

}
