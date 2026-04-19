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
    //if (mState == "idle") return;

    if (strcmp(property.getName(), "CCD1") == 0)
    {
        newBLOB(property);
    }
    if (
        (property.getDeviceName() == getString("devices", "mount"))
        &&  (property.getState() == IPS_OK)
    )
    {
        logInfo("MoveDone");
        emit MoveDone();
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

    connect(InitInit, &QState::entered, this, &Polar::SMInit);
    connect(RequestFrameReset, &QState::entered, this, &Polar::SMRequestFrameReset);
    connect(RequestExposure, &QState::entered, this, &Polar::SMRequestExposure);
    connect(RequestMove, &QState::entered, this, &Polar::SMRequestMove);
    connect(FindStars, &QState::entered, this, &Polar::SMFindStars);
    connect(Compute, &QState::entered, this, &Polar::SMCompute);
    connect(FinalCompute, &QState::entered, this, &Polar::SMComputeFinal);
    connect(Abort,               &QState::entered, this, &Polar::SMAbort);

    Polar->               addTransition(this, &Polar::Abort, Abort);
    Abort->               addTransition(this, &Polar::AbortDone, End);

    InitInit->            addTransition(this, &Polar::InitDone, RequestFrameReset);
    RequestFrameReset->   addTransition(this, &Polar::RequestFrameResetDone, WaitFrameReset);
    WaitFrameReset->      addTransition(this, &Polar::FrameResetDone, RequestExposure);
    RequestExposure->     addTransition(this, &Polar::RequestExposureDone, WaitExposure);
    WaitExposure->        addTransition(this, &Polar::ExposureDone, FindStars);
    FindStars->           addTransition(this, &Polar::FindStarsDone, Compute);
    Compute->             addTransition(this, &Polar::ComputeDone, RequestMove);
    Compute->             addTransition(this, &Polar::PolarDone, FinalCompute);
    RequestMove->         addTransition(this, &Polar::RequestMoveDone, WaitMove);
    WaitMove->            addTransition(this, &Polar::MoveDone, RequestExposure);
    FinalCompute->        addTransition(this, &Polar::ComputeFinalDone, End);

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
    getEltFloat("errors", "errtot")->setValue(_errtot, true);

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

    double t = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    //double t = ln_get_julian_from_sys();

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
    logInfo("SMRequestMove oldRA=" + QString::number(oldRA));

    if (_mountPointingWest)
    {
        newRA = oldRA - 0.25;
        if (newRA < 0) newRA = newRA + 24;
    }
    else
    {
        newRA = oldRA + +0.25;
        if (newRA >= 24) newRA = newRA - 24;
    }

    logInfo("SMRequestMove newRA=" + QString::number(newRA));
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

    Rotations::V3 p0(Rotations::azAlt2xyz(QPointF(_ra0, _de0)));
    Rotations::V3 p1(Rotations::azAlt2xyz(QPointF(_ra1, _de1)));
    Rotations::V3 p2(Rotations::azAlt2xyz(QPointF(_ra2, _de2)));
    //Rotations::V3 p0(Rotations::haDec2xyz(QPointF(_ra0, _de0),0));
    //Rotations::V3 p1(Rotations::haDec2xyz(QPointF(_ra1, _de1),0));
    //Rotations::V3 p2(Rotations::haDec2xyz(QPointF(_ra2, _de2),0));
    Rotations::V3 axis = Rotations::getAxis(p0, p1, p2);

    if (axis.length() < 0.9)
    {
        // It failed to normalize the vector, something's wrong.
        logWarning("Normal vector too short. findAxis failed.");
        emit Abort();
        return;
    }

    // Need to make sure we're pointing to the right pole.
    //if ((northernHemisphere() && (axis.x() < 0)) || (!northernHemisphere() && axis.x() > 0))
    if (axis.x() < 0)
    {
        axis = Rotations::V3(-axis.x(), -axis.y(), -axis.z());
    }

    //BOOST_LOG_TRIVIAL(debug) << "axis.x() " << axis.x();
    //BOOST_LOG_TRIVIAL(debug) << "axis.y() " << axis.y();
    //BOOST_LOG_TRIVIAL(debug) << "axis.z() " << axis.z();
    QPointF azAlt = Rotations::xyz2azAlt(axis);
    //azimuthCenter = azAlt.x();
    //altitudeCenter = azAlt.y();
    _erraz = azAlt.x();
    _erralt = 90 - azAlt.y();
    //_erralt=azAlt.y();
    _errtot = sqrt(square(_erraz) + square(_erralt));
    //BOOST_LOG_TRIVIAL(debug) << "azimuthCenter "  << _erraz;
    //BOOST_LOG_TRIVIAL(debug) << "altitudeCenter " << _erralt;
    //BOOST_LOG_TRIVIAL(debug) << "PA error °"       << _errtot;
    qDebug() << _erraz;
    qDebug() << _erralt;
    qDebug() << _errtot;
    getEltFloat("errors", "erraz")->setValue(_erraz, false);
    getEltFloat("errors", "erralt")->setValue(_erralt, false);
    getEltFloat("errors", "errtot")->setValue(_errtot, true);

    getEltLight("states", "idle")->setValue(OST::Idle, false);
    getEltLight("states", "moving")->setValue(OST::Idle, false);
    getEltLight("states", "shooting")->setValue(OST::Idle, false);
    getEltLight("states", "solving")->setValue(OST::Idle, false);
    getEltLight("states", "compute")->setValue(OST::Busy, true);

    emit ComputeFinalDone();
    return;

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
    connect(&_solver, &Solver::successSolve, this, &Polar::OnSucessSolve);
    connect(&_solver, &Solver::solverLog, this, &Polar::OnSolverLog);
    _solver.stars.clear();
    SSolver::Parameters params = _solver.stellarSolverProfiles[0];
    params.minwidth = 0.1 * _ccdFov / 3600;
    params.maxwidth = 1.1 * _ccdFov / 3600;
    params.search_radius = 2;
    //BOOST_LOG_TRIVIAL(debug) << "minwidth " << params.minwidth;
    //BOOST_LOG_TRIVIAL(debug) << "maxidth " << params.maxwidth;
    //_solver.setSearchScale(0.1*_ccdFov/3600,1.1*_ccdFov/3600,ScaleUnits::DEG_WIDTH);
    //_solver.setSearchPositionInDegrees(_mountRA*360/24,_mountDEC);
    _solver.stellarSolver.setSearchPositionInDegrees(_mountRA * 360 / 24, _mountDEC);
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
    disconnect(&_solver, &Solver::solverLog, this, &Polar::OnSolverLog);
    emit FindStarsDone();
}
void Polar::OnSolverLog(QString &text)
{
    //logInfo(text);
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
