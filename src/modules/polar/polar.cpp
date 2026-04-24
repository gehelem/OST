#include "polar.h"
#include "rotations.h"
#include <QPainter>
#include "version.cc"

#define PI 3.14159265

Polar *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Polar *basemodule = new Polar( name,  label,  profile,  availableModuleLibs);
    return basemodule;
}
Polar::Polar(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{

    loadOstPropertiesFromFile(":polar.json");

    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("description", "Polar assistant");
    setMetadata("template", "polar");

    giveMeADevice("camera", "Camera", INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("mount", "Mount", INDI::BaseDevice::TELESCOPE_INTERFACE);
    giveMeADevice("gps", "GPS", INDI::BaseDevice::GPS_INTERFACE);
    defineMeAsImager();

    auto* b = new OST::ElementBool("start", "Start", "0", "");
    getProperty("actions")->addElt( b);
    b = new OST::ElementBool("abort", "Abort", "2", "");
    getProperty("actions")->addElt( b);
    b = new OST::ElementBool("test", "Test", "4", "");
    getProperty("actions")->addElt( b);


    buildStateMachine();

}
Polar::~Polar()
{

}
void Polar::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        if (event.eltkey == "start")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                connectIndi();
                connectDevice(getString("devices", "camera"));
                connectDevice(getString("devices", "mount"));
                connectDevice(getString("devices", "gps"));
                setBLOBMode(B_ALSO, getString("devices", "camera").toStdString().c_str(), nullptr);
                if (getString("devices", "camera") == "CCD Simulator")
                {
                    sendModNewNumber(getString("devices", "camera"), "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 0.01 );
                }
                sendModNewNumber(getString("devices", "camera"), "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 0.01 );
                enableDirectBlobAccess(getString("devices", "camera").toStdString().c_str(), nullptr);

                _machine.start();
            }
        }
        if (event.eltkey == "abort")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                emit Abort();
            }
        }
        if (event.eltkey == "test")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                SMComputeFinal();
            }
        }
    }

}


void Polar::updateProperty(INDI::Property property)
{
    if (!_machine.isRunning()) return;

    if (strcmp(property.getName(), "CCD1") == 0)
    {
        newBLOB(property);
    }
    if (
        (property.getDeviceName() == getString("devices", "mount"))
        &&  (property.getName()   == std::string("EQUATORIAL_EOD_COORD"))
    )
    {
        if (property.getState() == IPS_BUSY)
        {
            _slewing = true;
        }
        if (property.getState() == IPS_OK && _slewing)
        {
            _slewing = false;
            logInfo("MoveDone");
            emit MoveDone();
        }
    }
    if (
        (property.getDeviceName() == getString("devices", "camera"))
        &&  (property.getName()   == std::string("CCD_FRAME_RESET"))
        &&  (property.getState() == IPS_OK)
    )
    {
        logInfo("FrameResetDone");
        emit FrameResetDone();
    }
}
void Polar::newBLOB(INDI::PropertyBlob  bp)
{
    if (
        (QString(bp.getDeviceName()) == getString("devices", "camera")) && (_machine.isRunning())
    )
    {
        delete image;
        image = new fileio();
        image->loadBlob(bp, 64);
        QImage rawImage = image->getRawQImage();
        QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
        im.setColorTable(rawImage.colorTable());
        _lastRawImage = im;  // keep a copy for error overlay
        im.save(getWebroot()  + "/" + getModuleName() + ".jpeg", "JPG", 100);
        OST::ImgData dta = image->ImgStats();
        dta.mUrlJpeg = getModuleName() + ".jpeg";
        getEltImg("image", "image")->setValue(dta, true);
        emit ExposureDone();
    }

}
void Polar::buildStateMachine(void)
{
    auto *Abort = new QState();
    auto *Polar = new QState();
    auto *End   = new QFinalState();

    auto *InitInit             = new QState(Polar);
    auto *RequestFrameReset    = new QState(Polar);
    auto *WaitFrameReset       = new QState(Polar);
    auto *RequestExposure      = new QState(Polar);
    auto *WaitExposure         = new QState(Polar);
    auto *FindStars            = new QState(Polar);
    auto *Compute              = new QState(Polar);
    auto *RequestMove          = new QState(Polar);
    auto *WaitMove             = new QState(Polar);
    auto *FinalCompute         = new QState(Polar);
    // Correction loop states (single-shot, no mount moves)
    auto *CorrExposure         = new QState(Polar);
    auto *CorrWaitExposure     = new QState(Polar);
    auto *CorrFindStars        = new QState(Polar);
    auto *CorrCompute          = new QState(Polar);

    connect(InitInit,      &QState::entered, this, &Polar::SMInit);
    connect(RequestFrameReset, &QState::entered, this, &Polar::SMRequestFrameReset);
    connect(RequestExposure,   &QState::entered, this, &Polar::SMRequestExposure);
    connect(RequestMove,       &QState::entered, this, &Polar::SMRequestMove);
    connect(FindStars,         &QState::entered, this, &Polar::SMFindStars);
    connect(Compute,           &QState::entered, this, &Polar::SMCompute);
    connect(FinalCompute,      &QState::entered, this, &Polar::SMComputeFinal);
    connect(CorrExposure,      &QState::entered, this, &Polar::SMRequestExposure);
    connect(CorrFindStars,     &QState::entered, this, &Polar::SMFindStars);
    connect(CorrCompute,       &QState::entered, this, &Polar::SMCorrCompute);
    connect(Abort,             &QState::entered, this, &Polar::SMAbort);

    Polar->               addTransition(this, &Polar::Abort, Abort);
    Abort->               addTransition(this, &Polar::AbortDone, End);

    // Calibration sequence (3 shots with mount moves)
    InitInit->            addTransition(this, &Polar::InitDone,             RequestFrameReset);
    RequestFrameReset->   addTransition(this, &Polar::RequestFrameResetDone, WaitFrameReset);
    WaitFrameReset->      addTransition(this, &Polar::FrameResetDone,       RequestExposure);
    RequestExposure->     addTransition(this, &Polar::RequestExposureDone,  WaitExposure);
    WaitExposure->        addTransition(this, &Polar::ExposureDone,         FindStars);
    FindStars->           addTransition(this, &Polar::FindStarsDone,        Compute);
    FindStars->           addTransition(this, &Polar::FindStarsFailed,      Abort);
    Compute->             addTransition(this, &Polar::ComputeDone,          RequestMove);
    Compute->             addTransition(this, &Polar::PolarDone,            FinalCompute);
    RequestMove->         addTransition(this, &Polar::RequestMoveDone,      WaitMove);
    WaitMove->            addTransition(this, &Polar::MoveDone,             RequestExposure);

    // After calibration: enter continuous correction loop (no mount moves)
    FinalCompute->        addTransition(this, &Polar::ComputeFinalDone,     CorrExposure);
    CorrExposure->        addTransition(this, &Polar::RequestExposureDone,  CorrWaitExposure);
    CorrWaitExposure->    addTransition(this, &Polar::ExposureDone,         CorrFindStars);
    CorrFindStars->       addTransition(this, &Polar::FindStarsDone,        CorrCompute);
    CorrFindStars->       addTransition(this, &Polar::FindStarsFailed,      CorrExposure);
    CorrCompute->         addTransition(this, &Polar::CorrComputeDone,      CorrExposure);

    Polar->setInitialState(InitInit);

    _machine.addState(Polar);
    _machine.addState(Abort);
    _machine.addState(End);
    _machine.setInitialState(Polar);


}
void Polar::SMInit()
{
    logInfo("SMInit");

    /* get mount DEC */
    if (!getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "DEC", _mountDEC))
    {
        emit Abort();
        return;
    }

    /* get mount RA */
    if (!getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "RA", _mountRA))
    {
        emit Abort();
        return;
    }

    /* get pixel size */
    if (!getModNumber(getString("devices", "camera"), "CCD_INFO", "CCD_PIXEL_SIZE", _pixelSize))
    {
        emit Abort();
        return;
    }

    /* get ccd width */
    if (!getModNumber(getString("devices", "camera"), "CCD_INFO", "CCD_MAX_X", _ccdX))
    {
        emit Abort();
        return;
    }

    /* get ccd height */
    if (!getModNumber(getString("devices", "camera"), "CCD_INFO", "CCD_MAX_Y", _ccdY))
    {
        emit Abort();
        return;
    }

    _focalLength = getFloat("optic", "fl");
    _ccdSampling = getSampling();
    _ccdFov = sqrt(square(_ccdX) + square(_ccdY)) * _ccdSampling;

    /* get mount Pier position  */
    if (!getModSwitch(getString("devices", "mount"), "TELESCOPE_PIER_SIDE", "PIER_WEST", _mountPointingWest))
    {
        emit Abort();
        return;
    }

    /* get observer GPS position from mount */
    if (!getModNumber(getString("devices", "gps"), "GEOGRAPHIC_COORD", "LAT", _observerLat))
    {
        logWarning("Cannot read observer latitude from mount (GEOGRAPHIC_COORD/LAT)");
        emit Abort();
        return;
    }
    if (!getModNumber(getString("devices", "gps"), "GEOGRAPHIC_COORD", "LONG", _observerLon))
    {
        logWarning("Cannot read observer longitude from mount (GEOGRAPHIC_COORD/LONG)");
        emit Abort();
        return;
    }
    logInfo("Observer position: lat=" + QString::number(_observerLat) + " lon=" + QString::number(_observerLon));

    _itt = 0;
    _ra0 = 0;
    _de0 = 0;
    _t0 = 0;
    _ra1 = 0;
    _de1 = 0;
    _t1 = 0;
    _ra2 = 0;
    _de2 = 0;
    _t2 = 0;

    getEltFloat("values", "ra0")->setValue(_ra0, false);
    getEltFloat("values", "de0")->setValue(_de0, false);
    getEltFloat("values", "t0")->setValue(_t0, false);
    getEltFloat("values", "ra1")->setValue(_ra1, false);
    getEltFloat("values", "de1")->setValue(_de1, false);
    getEltFloat("values", "t1")->setValue(_t1, false);
    getEltFloat("values", "ra2")->setValue(_ra2, false);
    getEltFloat("values", "de2")->setValue(_de2, false);
    getEltFloat("values", "t2")->setValue(_t2, true);

    _erraz = 0;
    _erralt = 0;
    _errtot = 0;

    getEltFloat("errors", "erraz")->setValue(_erraz, false);
    getEltFloat("errors", "erralt")->setValue(_erralt, false);
    getEltFloat("errors", "errtot")->setValue(_errtot, false);
    getEltFloat("errors", "errazmn")->setValue(_erraz * 60, false);
    getEltFloat("errors", "erraltmn")->setValue(_erralt * 60, false);
    getEltFloat("errors", "errtotmn")->setValue(_errtot * 60, true);

    emit InitDone();
}
void Polar::SMRequestFrameReset()
{
    logInfo("SMRequestFrameReset");
    if (!frameReset(getString("devices", "camera")))
    {
        emit Abort();
        return;
    }
    emit RequestFrameResetDone();
}
void Polar::SMRequestExposure()
{
    logInfo("SMRequestExposure");
    getEltLight("states", "idle")->setValue(OST::Idle, false);
    getEltLight("states", "moving")->setValue(OST::Idle, false);
    getEltLight("states", "shooting")->setValue(OST::Busy, false);
    getEltLight("states", "solving")->setValue(OST::Idle, false);
    getEltLight("states", "compute")->setValue(OST::Idle, true);

    double t = QDateTime::currentMSecsSinceEpoch() + _exposure * 500.0;  // mid-exposure estimate

    if (_itt == 0)
    {
        _t0 = t;
    }
    if (_itt == 1)
    {
        _t1 = t;
    }
    if (_itt == 2)
    {
        _t2 = t;
    }

    if (!requestCapture(getString("devices", "camera"), _exposure, getInt("parms", "gain"), getInt("parms", "offset")))
    {
        emit Abort();
        return;
    }
    emit RequestExposureDone();
}
void Polar::SMRequestMove()
{
    logInfo("SMRequestMove");
    getEltLight("states", "idle")->setValue(OST::Idle, false);
    getEltLight("states", "moving")->setValue(OST::Busy, false);
    getEltLight("states", "shooting")->setValue(OST::Idle, false);
    getEltLight("states", "solving")->setValue(OST::Idle, false);
    getEltLight("states", "compute")->setValue(OST::Idle, true);

    logInfo("SMRequestMove");

    double oldRA, newRA;

    /* get mount RA */
    if (!getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "RA", oldRA))
    {
        logInfo("SMRequestMove error 1");
        emit Abort();
        return;
    }

    if (_mountPointingWest)
    {
        newRA = oldRA - 0.5; // 1h = 15°
        if (newRA < 0) newRA = newRA + 24;
    }
    else
    {
        newRA = oldRA + 0.5;
        if (newRA >= 24) newRA = newRA - 24;
    }

    _slewing = false;  // wait for IPS_BUSY before accepting IPS_OK as move done
    logInfo("SMRequestMove oldRA=%1 newRA=%2",{oldRA,newRA});
    if (!sendModNewNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "RA", newRA))
    {
        logInfo("SMRequestMove error 2");
        emit Abort();
        return;
    }
    emit RequestMoveDone();

}
void Polar::SMCompute()
{
    logInfo("SMCompute");
    getEltLight("states", "idle")->setValue(OST::Idle, false);
    getEltLight("states", "moving")->setValue(OST::Idle, false);
    getEltLight("states", "shooting")->setValue(OST::Idle, false);
    getEltLight("states", "solving")->setValue(OST::Idle, false);
    getEltLight("states", "compute")->setValue(OST::Busy, true);

    INDI::IEquatorialCoordinates coord2000, coordNow;
    coordNow.rightascension = _solver.stellarSolver.getSolution().ra * 24 / 360;
    coordNow.declination = _solver.stellarSolver.getSolution().dec;

    //coord2000.rightascension=_solver.stellarSolver.getSolution().ra;
    //coord2000.declination=_solver.stellarSolver.getSolution().dec;

    OST::ImgData dta = image->ImgStats();
    dta.starsCount = _solver.stars.size();
    dta.solverRA = _solver.stellarSolver.getSolution().ra;
    dta.solverDE = _solver.stellarSolver.getSolution().dec;
    dta.isSolved = true;
    getEltImg("image", "image")->setValue(dta, true);

    if (_itt == 0)
    {
        //INDI::ObservedToJ2000(&coordNow,_t0,&coord2000);
        //_ra0=coord2000.rightascension;
        //_de0=coord2000.declination;
        _ra0 = _solver.stellarSolver.getSolution().ra;
        _de0 = _solver.stellarSolver.getSolution().dec;
        logInfo("ra0=" + QString::number(_ra0));
        logInfo("de0=" + QString::number(_de0));

    }
    if (_itt == 1)
    {
        //INDI::ObservedToJ2000(&coordNow,_t1,&coord2000);
        //_ra1=coord2000.rightascension;
        //_de1=coord2000.declination;
        _ra1 = _solver.stellarSolver.getSolution().ra;
        _de1 = _solver.stellarSolver.getSolution().dec;
        logInfo("ra1=" + QString::number(_ra1));
        logInfo("de1=" + QString::number(_de1));
    }
    if (_itt == 2)
    {
        //INDI::ObservedToJ2000(&coordNow,_t2,&coord2000);
        //_ra2=coord2000.rightascension;
        //_de2=coord2000.declination;
        _ra2 = _solver.stellarSolver.getSolution().ra;
        _de2 = _solver.stellarSolver.getSolution().dec;
        logInfo("ra2=" + QString::number(_ra2));
        logInfo("de2=" + QString::number(_de2));
    }

    getEltFloat("values", "ra0")->setValue(_ra0, false);
    getEltFloat("values", "de0")->setValue(_de0, false);
    getEltFloat("values", "t0")->setValue(_t0, false);
    getEltFloat("values", "ra1")->setValue(_ra1, false);
    getEltFloat("values", "de1")->setValue(_de1, false);
    getEltFloat("values", "t1")->setValue(_t1, false);
    getEltFloat("values", "ra2")->setValue(_ra2, false);
    getEltFloat("values", "de2")->setValue(_de2, false);
    getEltFloat("values", "t2")->setValue(_t2, true);

    _itt++;
    if (_itt < 3) emit ComputeDone();
    else emit PolarDone();

}
void Polar::syncMount(double ra_j2000_deg, double dec_j2000_deg)
{
    QString mount = getString("devices", "mount");
    INDI::BaseDevice dp = getDevice(mount.toStdString().c_str());
    if (!dp.isValid())
    {
        logWarning("syncMount: mount device not found");
        return;
    }
    if (!sendModNewSwitch(mount, "ON_COORD_SET", "SYNC", ISS_ON))
    {
        logWarning("syncMount: failed to switch to SYNC mode");
        return;
    }
    INDI::PropertyNumber prop = dp.getNumber("EQUATORIAL_EOD_COORD");
    if (!prop.isValid())
    {
        logWarning("syncMount: EQUATORIAL_EOD_COORD not found");
        sendModNewSwitch(mount, "ON_COORD_SET", "SLEW", ISS_ON);
        return;
    }
    INDI::IEquatorialCoordinates j2000pos, observed;
    j2000pos.rightascension = ra_j2000_deg / 15.0;  // degrees -> hours
    j2000pos.declination    = dec_j2000_deg;
    double jd = ln_get_julian_from_sys();
    INDI::J2000toObserved(&j2000pos, jd, &observed);
    prop.findWidgetByName("RA")->value  = observed.rightascension;
    prop.findWidgetByName("DEC")->value = observed.declination;
    sendNewNumber(prop);
    if (!sendModNewSwitch(mount, "ON_COORD_SET", "SLEW", ISS_ON))
        logWarning("syncMount: failed to restore SLEW mode");
    logInfo("Mount synced: RA=" + QString::number(observed.rightascension) + "h DEC=" + QString::number(observed.declination) + "°");
}
QPointF Polar::solverToAzAlt(double ra_j2000_deg, double dec_j2000_deg, double jd)
{
    // Step 1: J2000 -> apparent (accounts for precession, nutation, aberration)
    INDI::IEquatorialCoordinates j2000, apparent;
    j2000.rightascension = ra_j2000_deg * 24.0 / 360.0;  // degrees -> hours
    j2000.declination    = dec_j2000_deg;
    INDI::J2000toObserved(&j2000, jd, &apparent);
    // apparent.rightascension in hours, apparent.declination in degrees

    // Step 2: hour angle (LST - RA_apparent)
    double gst = ln_get_apparent_sidereal_time(jd);       // hours, Greenwich
    double lst = gst + _observerLon / 15.0;               // hours, local (East positive)
    double ha  = lst - apparent.rightascension;            // hours
    while (ha >  12.0) ha -= 24.0;
    while (ha < -12.0) ha += 24.0;

    // Step 3: equatorial -> horizontal (standard trigonometric formula)
    // Convention: Az = 0 North, 90 East, 180 South, 270 West
    double lat_r = _observerLat          * M_PI / 180.0;
    double ha_r  = ha * 15.0             * M_PI / 180.0;  // hours -> degrees -> radians
    double dec_r = apparent.declination  * M_PI / 180.0;

    double sin_alt = sin(lat_r) * sin(dec_r) + cos(lat_r) * cos(dec_r) * cos(ha_r);
    sin_alt = qBound(-1.0, sin_alt, 1.0);
    double alt_rad = asin(sin_alt);

    double cos_az_num = sin(dec_r) - sin(alt_rad) * sin(lat_r);
    double cos_az_den = cos(alt_rad) * cos(lat_r);
    double az_rad;
    if (fabs(cos_az_den) < 1e-10)
    {
        az_rad = 0.0;  // zenith or nadir, azimuth undefined
    }
    else
    {
        double cos_az = qBound(-1.0, cos_az_num / cos_az_den, 1.0);
        az_rad = acos(cos_az);
        if (sin(ha_r) > 0.0) az_rad = 2.0 * M_PI - az_rad;  // quadrant correction
    }

    return QPointF(az_rad * 180.0 / M_PI, alt_rad * 180.0 / M_PI);
}

void Polar::SMComputeFinal()
{
    logInfo("SMComputeFinal");
    //_ra0=354.1265137671062;  _de0=0.2921369570805727;_t0=1643110224331;
    //_ra1=339.12724652172227; _de1=0.300002845425132; _t1=1643110229671;
    //_ra2=324.1279546867718;  _de2=0.315854040156525; _t2=1643110234681;
    //BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal RA 0 = " <<     _ra0;
    //BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DE 0 = " <<     _de0;
    //BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal t 0  = " <<     _t0;

    //BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal RA 1 = " <<     _ra1;
    //BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DE 1 = " <<     _de1;
    //BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal t 1  = " <<     _t1;

    //BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal RA 2 = " <<     _ra2;
    //BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DE 2 = " <<     _de2;
    //BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal t 2  = " <<     _t2;
    /*double dra1=360*(_t1-_t0)/(1000*3600*24);
    double dra2=360*(_t2-_t0)/(1000*3600*24);

    BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DRA1-0 = " << dra1;
    BOOST_LOG_TRIVIAL(debug) << "SMComputeFinal DRA2-0 = " << dra2;*/

    // Convert each J2000 solution to Az/Alt using observer position and capture time
    double jd0 = 2440587.5 + _t0 / (86400.0 * 1000.0);
    double jd1 = 2440587.5 + _t1 / (86400.0 * 1000.0);
    double jd2 = 2440587.5 + _t2 / (86400.0 * 1000.0);

    QPointF azAlt0 = solverToAzAlt(_ra0, _de0, jd0);
    QPointF azAlt1 = solverToAzAlt(_ra1, _de1, jd1);
    QPointF azAlt2 = solverToAzAlt(_ra2, _de2, jd2);
    logInfo("azAlt0: az=" + QString::number(azAlt0.x()) + " alt=" + QString::number(azAlt0.y()));
    logInfo("azAlt1: az=" + QString::number(azAlt1.x()) + " alt=" + QString::number(azAlt1.y()));
    logInfo("azAlt2: az=" + QString::number(azAlt2.x()) + " alt=" + QString::number(azAlt2.y()));

    Rotations::V3 p0(Rotations::azAlt2xyz(azAlt0));
    Rotations::V3 p1(Rotations::azAlt2xyz(azAlt1));
    Rotations::V3 p2(Rotations::azAlt2xyz(azAlt2));
    Rotations::V3 axis = Rotations::getAxis(p0, p1, p2);

    if (axis.length() < 0.9)
    {
        // It failed to normalize the vector, something's wrong.
        logWarning("Normal vector too short. findAxis failed.");
        emit Abort();
        return;
    }

    // Make sure we're pointing to the correct pole (north or south)
    bool northernHemisphere = (_observerLat >= 0.0);
    if ((northernHemisphere && axis.x() < 0) || (!northernHemisphere && axis.x() > 0))
    {
        axis = Rotations::V3(-axis.x(), -axis.y(), -axis.z());
    }

    QPointF azAlt = Rotations::xyz2azAlt(axis);
    logInfo("axis azAlt: az=" + QString::number(azAlt.x()) + " alt=" + QString::number(azAlt.y()));

    // Azimuth error: how far the axis is from due North (should be 0)
    _erraz = azAlt.x();
    // Altitude error: difference between actual axis altitude and observer latitude
    _erralt = azAlt.y() - _observerLat;
    _errtot = sqrt(square(_erraz) + square(_erralt));
    //BOOST_LOG_TRIVIAL(debug) << "azimuthCenter "  << _erraz;
    //BOOST_LOG_TRIVIAL(debug) << "altitudeCenter " << _erralt;
    //BOOST_LOG_TRIVIAL(debug) << "PA error °"       << _errtot;
    getEltFloat("errors", "erraz")->setValue(_erraz, false);
    getEltFloat("errors", "erralt")->setValue(_erralt, false);
    getEltFloat("errors", "errtot")->setValue(_errtot, false);
    getEltFloat("errors", "errazmn")->setValue(_erraz * 60, false);
    getEltFloat("errors", "erraltmn")->setValue(_erralt * 60, false);
    getEltFloat("errors", "errtotmn")->setValue(_errtot * 60, true);

    // Store the Az/Alt of image 3 as the reference for the correction loop
    _refAz  = azAlt2.x();
    _refAlt = azAlt2.y();
    logInfo("Correction reference: refAz=" + QString::number(_refAz) + " refAlt=" + QString::number(_refAlt));

    // Draw initial error overlay on the last image and publish it
    if (!_lastRawImage.isNull())
    {
        QImage overlaid = _lastRawImage.copy();
        drawErrorOverlay(overlaid, _erraz, _erralt, _errtot);
        overlaid.save(getWebroot() + "/" + getModuleName() + ".jpeg", "JPG", 90);
        OST::ImgData dta = image->ImgStats();
        dta.mUrlJpeg = getModuleName() + ".jpeg";
        dta.isSolved = true;
        getEltImg("image", "image")->setValue(dta, true);
    }

    getEltLight("states", "idle")->setValue(OST::Idle, false);
    getEltLight("states", "moving")->setValue(OST::Idle, false);
    getEltLight("states", "shooting")->setValue(OST::Idle, false);
    getEltLight("states", "solving")->setValue(OST::Idle, false);
    getEltLight("states", "compute")->setValue(OST::Busy, true);

    emit ComputeFinalDone();
    return;

}

void Polar::SMCorrCompute()
{
    logInfo("SMCorrCompute");
    getEltLight("states", "compute")->setValue(OST::Busy, true);

    // Current J2000 position from solver
    double currentRA  = _solver.stellarSolver.getSolution().ra;   // J2000 degrees
    double currentDEC = _solver.stellarSolver.getSolution().dec;  // J2000 degrees

    // Convert to Az/Alt using the same epoch as the reference image (_t2),
    // so sidereal rotation cancels out and the Az/Alt delta is purely mechanical.
    double jdRef = 2440587.5 + _t2 / (86400.0 * 1000.0);
    QPointF currentAzAlt = solverToAzAlt(currentRA, currentDEC, jdRef);

    // Displacement in Az/Alt from the calibration reference image
    // When the user adjusts the mount, this displacement tracks the correction applied
    double dAz  = currentAzAlt.x() - _refAz;
    double dAlt = currentAzAlt.y() - _refAlt;

    // Remaining error = initial error minus the correction already applied
    double remainAz  = _erraz  - dAz;
    double remainAlt = _erralt - dAlt;
    double remainTot = sqrt(remainAz * remainAz + remainAlt * remainAlt);

    logInfo("Correction loop: dAz=" + QString::number(dAz) + " dAlt=" + QString::number(dAlt)
            + " remainAz=" + QString::number(remainAz) + " remainAlt=" + QString::number(remainAlt));

    getEltFloat("errors", "erraz")->setValue(remainAz, false);
    getEltFloat("errors", "erralt")->setValue(remainAlt, false);
    getEltFloat("errors", "errtot")->setValue(remainTot, false);
    getEltFloat("errors", "errazmn")->setValue(remainAz * 60, false);
    getEltFloat("errors", "erraltmn")->setValue(remainAlt * 60, false);
    getEltFloat("errors", "errtotmn")->setValue(remainTot * 60, true);

    // Draw overlay with remaining error on the latest image
    if (!_lastRawImage.isNull())
    {
        QImage overlaid = _lastRawImage.copy();
        drawErrorOverlay(overlaid, remainAz, remainAlt, remainTot);
        overlaid.save(getWebroot() + "/" + getModuleName() + ".jpeg", "JPG", 90);
        OST::ImgData dta = image->ImgStats();
        dta.mUrlJpeg = getModuleName() + ".jpeg";
        dta.isSolved = true;
        getEltImg("image", "image")->setValue(dta, true);
    }

    emit CorrComputeDone();
}

void Polar::drawErrorOverlay(QImage &img, double erraz, double erralt, double errtot)
{
    if (img.isNull()) return;

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    int cx = img.width()  / 2;
    int cy = img.height() / 2;

    // Scale so the total error spans 30% of image height; minimum 50 px/degree
    double totalDeg = qMax(fabs(errtot), 0.01);
    double maxPx    = img.height() * 0.30;
    double scale    = qMax(maxPx / totalDeg, 50.0);  // pixels per degree

    // Triangle vertices:
    // P1 = image center (current polar axis position)
    // P2 = P1 + altitude correction (vertical)
    // P3 = P2 + azimuth correction (horizontal) = target position
    QPoint p1(cx, cy);
    QPoint p2(cx,                          cy - qRound(erralt * scale));
    QPoint p3(cx + qRound(erraz  * scale), cy - qRound(erralt * scale));

    // Altitude error line (yellow)
    painter.setPen(QPen(QColor(255, 220, 0), 2));
    painter.drawLine(p1, p2);

    // Azimuth error line (green)
    painter.setPen(QPen(QColor(0, 230, 0), 2));
    painter.drawLine(p2, p3);

    // Total error line (red)
    painter.setPen(QPen(QColor(230, 0, 0), 2));
    painter.drawLine(p1, p3);

    // Vertex dots
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawEllipse(p1, 4, 4);
    painter.setBrush(QColor(255, 220, 0));
    painter.drawEllipse(p2, 4, 4);
    painter.setBrush(QColor(0, 230, 0));
    painter.drawEllipse(p3, 4, 4);

    // Labels (values in arcminutes)
    QFont font = painter.font();
    font.setPixelSize(14);
    font.setBold(true);
    painter.setFont(font);

    painter.setPen(QColor(255, 220, 0));
    painter.drawText(p2 + QPoint(8, 0), QString("ALT %1'").arg(erralt * 60.0, 0, 'f', 1));

    painter.setPen(QColor(0, 230, 0));
    painter.drawText(p3 + QPoint(8, 0), QString("AZ %1'").arg(erraz * 60.0, 0, 'f', 1));

    painter.setPen(QColor(230, 0, 0));
    painter.drawText((p1 + p3) / 2 + QPoint(8, -6), QString("TOT %1'").arg(errtot * 60.0, 0, 'f', 1));

    painter.end();
}

void Polar::SMFindStars()
{

    getEltLight("states", "idle")->setValue(OST::Idle, false);
    getEltLight("states", "moving")->setValue(OST::Idle, false);
    getEltLight("states", "shooting")->setValue(OST::Idle, false);
    getEltLight("states", "solving")->setValue(OST::Busy, false);
    getEltLight("states", "compute")->setValue(OST::Idle, true);

    logInfo("SMFindStars");

    /* get mount DEC */
    if (!getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "DEC", _mountDEC))
    {
        emit Abort();
        return;
    }

    /* get mount RA */
    if (!getModNumber(getString("devices", "mount"), "EQUATORIAL_EOD_COORD", "RA", _mountRA))
    {
        emit Abort();
        return;
    }

    mStats = image->getStats();
    _solver.ResetSolver(mStats, image->getImageBuffer());
    QStringList folders;
    folders.append("/usr/share/astrometry");
    _solver.stellarSolver.setIndexFolderPaths(folders);
    connect(&_solver, &Solver::successSolve, this, &Polar::OnSucessSolve, Qt::UniqueConnection);
    connect(&_solver, &Solver::failSolve,    this, &Polar::OnFailSolve,    Qt::UniqueConnection);
    connect(&_solver, &Solver::solverLog,    this, &Polar::OnSolverLog,    Qt::UniqueConnection);
    _solver.stars.clear();
    SSolver::Parameters params = _solver.stellarSolverProfiles[0];
    double fovWidthDeg = _ccdX * _ccdSampling / 3600.0;  // actual horizontal FOV, not diagonal
    params.minwidth = 0.5 * fovWidthDeg;
    params.maxwidth = 1.1 * fovWidthDeg;
    params.search_radius = 3;
    //BOOST_LOG_TRIVIAL(debug) << "minwidth " << params.minwidth;
    //BOOST_LOG_TRIVIAL(debug) << "maxidth " << params.maxwidth;
    //_solver.setSearchScale(0.1*_ccdFov/3600,1.1*_ccdFov/3600,ScaleUnits::DEG_WIDTH);
    // Convert mount hint from EOD (EQUATORIAL_EOD_COORD) to J2000 — solver expects J2000
    INDI::IEquatorialCoordinates mountEod, mountJ2000;
    mountEod.rightascension = _mountRA;
    mountEod.declination    = _mountDEC;
    double jdHint = 2440587.5 + QDateTime::currentMSecsSinceEpoch() / (86400.0 * 1000.0);
    INDI::ObservedToJ2000(&mountEod, jdHint, &mountJ2000);
    _solver.stellarSolver.setSearchPositionInDegrees(mountJ2000.rightascension * 360.0 / 24.0, mountJ2000.declination);
    _solver.SolveStars(params);
}
void Polar::OnSucessSolve()
{

    logInfo("SEP finished");
    OST::ImgData dta = image->ImgStats();
    dta.mUrlJpeg = getModuleName() + ".jpeg";
    dta.starsCount = _solver.stars.size();
    dta.solverRA = _solver.stellarSolver.getSolution().ra;
    dta.solverDE = _solver.stellarSolver.getSolution().dec;
    dta.isSolved = true;
    getEltImg("image", "image")->setValue(dta, true);

    disconnect(&_solver, &Solver::successSolve, this, &Polar::OnSucessSolve);
    disconnect(&_solver, &Solver::failSolve,    this, &Polar::OnFailSolve);
    disconnect(&_solver, &Solver::solverLog,    this, &Polar::OnSolverLog);
    emit FindStarsDone();
}
void Polar::OnFailSolve()
{
    logWarning("Solver failed to find a solution");
    disconnect(&_solver, &Solver::successSolve, this, &Polar::OnSucessSolve);
    disconnect(&_solver, &Solver::failSolve,    this, &Polar::OnFailSolve);
    disconnect(&_solver, &Solver::solverLog,    this, &Polar::OnSolverLog);
    emit FindStarsFailed();
}
void Polar::OnSolverLog(QString &text)
{
    //logDebug(text);
}
void Polar::SMAbort()
{
    emit AbortDone();
    _machine.stop();
    getEltLight("states", "idle")->setValue(OST::Idle, false);
    getEltLight("states", "moving")->setValue(OST::Idle, false);
    getEltLight("states", "shooting")->setValue(OST::Idle, false);
    getEltLight("states", "solving")->setValue(OST::Idle, false);
    getEltLight("states", "compute")->setValue(OST::Idle, true);

}
