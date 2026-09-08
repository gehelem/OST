/**
 * @file blindpec.cpp
 * @brief BlindPEC - optical-encoder RA guiding from a USB microscope on the RA axis.
 *
 * See blindpec.h and src/modules/blindpec-notes.fr.md for the rationale.
 *
 * Plumbing (sequenced capture, BLOB ownership guard, hardware watchdog, pulse
 * sending, JPEG preview, guiding/drift grids) mirrors the current guider module.
 * The measurement is done by pecmeter (OpenCV phase correlation), not by star
 * extraction. RA axis only.
 */

#include "blindpec.h"
#include "version.cc"

#include <QPainter>
#include <QRandomGenerator>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/// Sidereal rate, arcsec per second (360 deg / sidereal day).
static constexpr double SIDEREAL_ARCSEC_PER_S = 15.041067;

/// Atomically replace a JPEG preview (write to .tmp then rename).
static void atomicSaveJpeg(const QImage &img, const QString &finalPath)
{
    const QString tmp = finalPath + ".tmp";
    if (img.save(tmp, "JPG", 100))
        ::rename(tmp.toLocal8Bit().constData(), finalPath.toLocal8Bit().constData());
}

BlindPec *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    return new BlindPec(name, label, profile, availableModuleLibs);
}

BlindPec::BlindPec(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{
    loadOstPropertiesFromFile(":blindpec.json");

    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "BlindPEC module");
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    // Reuse the guider front-end template: same shapes (image, guiding graph,
    // drift graph, pulse values).
    setMetadata("template", "guider");

    buildStateMachine();

    giveMeADevice("camera", "Microscope", INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("guider", "Guide via",  INDI::BaseDevice::GUIDER_INTERFACE);

    defineMeAsGuider();

    // Extra action buttons (defineMeAsGuider() creates calibrate / guide /
    // abortguider; add dither and reset-calibration like the guider does).
    OST::PropertyMulti *pm = getProperty("actions");
    OST::ElementBool *rc = new OST::ElementBool("resetcalibration", "Reset calibration", "bpec10", "");
    rc->setValue(false, false);
    rc->setPreIcon("block");
    pm->addElt(rc);
    OST::ElementBool *di = new OST::ElementBool("dither", "Dither", "bpec11", "");
    di->setValue(false, false);
    di->setPreIcon("shuffle");
    pm->addElt(di);

    // Single-shot hardware watchdog, re-armed at every hardware wait, disarmed on
    // the matching callback or on abort. Same pattern as guider.
    _watchdog.setSingleShot(true);
    connect(&_watchdog, &QTimer::timeout, this, [this]()
    {
        if (!_sm.isRunning())
            return;
        logError("Watchdog: no hardware response for %1 s - aborting",
        {QString::number(_watchdog.interval() / 1000)});
        setStateEvent(OST::Error, "error", "timeout", "hardware timeout");
        emit Abort();
    });
}

BlindPec::~BlindPec()
{
}

void BlindPec::armWatchdog()
{
    int t = getInt("guideParams", "watchdog");
    if (t > 0)
        _watchdog.start(t * 1000);
}

void BlindPec::disarmWatchdog()
{
    _watchdog.stop();
}

// Clear every action button, then light the one that is now running and mark the
// property Busy (guider pattern). Pass "" to leave all cleared and mark it Ok.
void BlindPec::setActionRunning(const QString &running)
{
    getEltBool("actions", "calibrate")->setValue(false, false);
    getEltBool("actions", "guide")->setValue(false, false);
    getEltBool("actions", "abortguider")->setValue(false, false);
    getEltBool("actions", "dither")->setValue(false, false);
    getEltBool("actions", "resetcalibration")->setValue(false, false);
    if (!running.isEmpty())
    {
        getEltBool("actions", running)->setValue(true, false);
        getProperty("actions")->setState(OST::Busy, true);
    }
    else
    {
        getProperty("actions")->setState(OST::Ok, true);
    }
}

pecmeter::Params BlindPec::meterParams()
{
    pecmeter::Params p;
    if (getEltFloat("measParams", "reanchorfrac")) p.reanchorFrac = getFloat("measParams", "reanchorfrac");
    if (getEltFloat("measParams", "maxsteppx"))    p.maxStepPx    = getFloat("measParams", "maxsteppx");
    if (getEltFloat("measParams", "minresponse"))  p.minResponse  = getFloat("measParams", "minresponse");
    if (getEltFloat("measParams", "preblur"))      p.preBlur      = getFloat("measParams", "preblur");
    if (getEltBool("measParams", "eccrefine"))     p.eccRefine    = getBool("measParams", "eccrefine");
    if (getEltInt("measParams", "ecciters"))       p.eccIters     = getInt("measParams", "ecciters");
    if (getEltFloat("measParams", "ecceps"))       p.eccEps       = getFloat("measParams", "ecceps");
    if (getEltInt("measParams", "eccgauss"))       p.eccGaussFilt = getInt("measParams", "eccgauss");
    if (getEltBool("measParams", "scurve"))        p.sCurve       = getBool("measParams", "scurve");
    if (getEltInt("measParams", "scurvepoints"))   p.sCurvePoints = getInt("measParams", "scurvepoints");
    if (getEltInt("measParams", "scurveharm"))     p.sCurveHarm   = getInt("measParams", "scurveharm");
    if (getEltBool("measParams", "dftshift"))      p.dftShift     = getBool("measParams", "dftshift");
    if (getEltInt("measParams", "upsamplefactor")) p.upsampleFactor = getInt("measParams", "upsamplefactor");
    return p;
}

// ======================================================================
//  External events (action buttons)
// ======================================================================
void BlindPec::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev != OST::ExtEvType::SV || event.prpkey != "actions")
        return;

    if (event.eltkey == "abortguider")
    {
        if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            emit Abort();
        return;
    }

    if (event.eltkey == "calibrate" || event.eltkey == "guide")
    {
        if (!getEltBool(event.prpkey, event.eltkey)->setValue(false, true))
            return;
        if (_sm.isRunning())
        {
            logWarning("BlindPEC already running - ignoring %1", {event.eltkey});
            return;
        }

        _calibrateOnly = (event.eltkey == "calibrate");
        _finishOk = false;
        _sCurveLogged = false;

        _theta       = getFloat("calibrationvalues", "theta") * M_PI / 180.0;
        _V           = getFloat("calibrationvalues", "V");
        _arcsecPerPx = getFloat("calibrationvalues", "arcsecPerPx");
        _G           = getFloat("calibrationvalues", "G");
        _wDir        = getInt("calibrationvalues", "wdir");
        if (_wDir == 0) _wDir = -1;
        _guideRateK  = getFloat("calibrationvalues", "guideRateK");
        if (_guideRateK <= 0) _guideRateK = 0.5;

        // theta / V / scale are geometric (microscope mounting, axis radius,
        // magnification) - fixed until the setup moves. G / wDir depend on the
        // mount guide output. So a full stored calibration is reused as-is:
        //   "guide" + full calibration -> straight to guiding
        //   "guide" without one, or "calibrate" -> characterize (+ gain cal)
        // resetcalibration clears it and forces a fresh characterization.
        const bool haveCal = (_arcsecPerPx > 0.0 && _V > 0.0 && _G > 0.0);

        _skipChar         = (!_calibrateOnly && haveCal);
        _skipGainCal      = (!_calibrateOnly && haveCal);
        _stopAfterGainCal = _calibrateOnly;
        _phase            = PhCharacterize;   // SMInitInit switches to PhGuide if _skipChar

        if (_calibrateOnly)
            logInfo("BlindPEC: calibration (characterize + gain)");
        else if (_skipChar)
            logInfo("BlindPEC: guide, using stored calibration (theta=%1 deg, V=%2 px/s, scale=%3 arcsec/px, G=%4 ms/px)",
        {
            QString::number(_theta * 180.0 / M_PI, 'f', 1), QString::number(_V, 'f', 4),
            QString::number(_arcsecPerPx, 'f', 3), QString::number(_G, 'f', 1)
        });
        else
            logInfo("BlindPEC: no stored calibration - characterize + gain + guide");
        _sm.start();
        return;
    }

    if (event.eltkey == "dither")
    {
        if (getEltBool(event.prpkey, event.eltkey)->setValue(false, true))
        {
            if (_sm.isRunning() && _phase == PhGuide)
            {
                _ditherPending = true;
                logInfo("Dither requested (RA only) - applied on the next guide frame");
            }
            else
            {
                logWarning("Dither ignored - BlindPEC is not guiding");
            }
        }
        return;
    }

    if (event.eltkey == "resetcalibration")
    {
        if (getEltBool(event.prpkey, event.eltkey)->setValue(false, true))
        {
            getProperty(event.prpkey)->setState(OST::Ok, true);
            getEltFloat("calibrationvalues", "G")->setValue(0);
            getEltInt("calibrationvalues", "wdir")->setValue(-1);
            getEltFloat("calibrationvalues", "theta")->setValue(0);
            getEltFloat("calibrationvalues", "V")->setValue(0);
            getEltFloat("calibrationvalues", "arcsecPerPx")->setValue(0, true);
            logInfo("BlindPEC: calibration data cleared - a full calibrate is required before guiding");
        }
        return;
    }
}

// ======================================================================
//  INDI property updates
// ======================================================================
void BlindPec::onUpdateProperty(INDI::Property property)
{
    const QString dev = property.getDeviceName();

    // --- trace: every update from the microscope / guide device -----------
    if (_trace && (dev == getString("devices", "camera") || dev == getString("devices", "guider")))
        logInfo("prop <- %1/%2 state=%3", {dev, QString(property.getName()), QString::number(property.getState())});

    if (strcmp(property.getName(), "CCD1") == 0)
        newBLOB(property);

    if (dev == getString("devices", "camera")
            && QString(property.getName()) == "CCD_FRAME_RESET"
            && property.getState() == IPS_OK)
    {
        if (_trace) logInfo("FrameResetDone (CCD_FRAME_RESET -> IPS_OK)");
        emit FrameResetDone();
    }

    // Camera rejected / failed the exposure: fail fast instead of waiting for the
    // watchdog. (Seen with the INDI V4L2 CCD driver.)
    if (dev == getString("devices", "camera")
            && QString(property.getName()) == "CCD_EXPOSURE"
            && property.getState() == IPS_ALERT
            && _expectingFrame)
    {
        _expectingFrame = false;
        logError("Camera rejected the exposure (CCD_EXPOSURE -> ALERT). "
                 "Check the camera driver in the INDI panel: capture format, "
                 "resolution, auto-exposure. Aborting.");
        setStateEvent(OST::Error, "error", "devicefailed", "exposure rejected");
        emit Abort();
    }

    if (dev == getString("devices", "guider")
            && QString(property.getName()) == "TELESCOPE_TIMED_GUIDE_WE"
            && property.getState() == IPS_IDLE)
    {
        _pulseRAfinished = true;
        if (_trace) logInfo("PulsesDone (TELESCOPE_TIMED_GUIDE_WE -> IPS_IDLE)");
        emit PulsesDone();
    }
}

void BlindPec::newBLOB(INDI::PropertyBlob pblob)
{
    if (_trace)
        logInfo("newBLOB: dev=%1 expecting=%2", {QString(pblob.getDeviceName()), _expectingFrame ? "yes" : "no"});

    // Only consume a frame we actually asked for (ignore stray / late BLOBs, e.g.
    // one arriving after an abort or from another module sharing the camera).
    if (!_expectingFrame)
        return;
    if (QString(pblob.getDeviceName()) != getString("devices", "camera"))
        return;

    _expectingFrame = false;

    delete _image;
    _image = new fileio();
    _image->loadBlob(pblob, 64);
    stats = _image->getStats();

    if (_trace)
        logInfo("frame received: %1x%2 bpp=%3 ch=%4",
    {
        QString::number(stats.width), QString::number(stats.height),
        QString::number(stats.bytesPerPixel), QString::number(stats.channels)
    });

    emit ExposureDone();
}

// ======================================================================
//  State machine
// ======================================================================
void BlindPec::buildStateMachine()
{
    auto *Run   = new QState();
    auto *Abort = new QState();
    auto *End   = new QFinalState();

    auto *InitInit        = new QState(Run);
    auto *ReqFrameReset   = new QState(Run);
    auto *WaitFrameReset  = new QState(Run);
    auto *ReqExposure     = new QState(Run);
    auto *WaitExposure    = new QState(Run);
    auto *Measure         = new QState(Run);
    auto *Compute         = new QState(Run);
    auto *ReqPulses       = new QState(Run);
    auto *WaitPulses      = new QState(Run);

    connect(InitInit,      &QState::entered, this, &BlindPec::SMInitInit);
    connect(ReqFrameReset, &QState::entered, this, &BlindPec::SMRequestFrameReset);
    connect(ReqExposure,   &QState::entered, this, &BlindPec::SMRequestExposure);
    connect(Measure,       &QState::entered, this, &BlindPec::SMMeasure);
    connect(Compute,       &QState::entered, this, &BlindPec::SMCompute);
    connect(ReqPulses,     &QState::entered, this, &BlindPec::SMRequestPulses);
    connect(Abort,         &QState::entered, this, &BlindPec::SMAbort);

    // Lightweight state tracing while bringing the module up.
    if (_trace)
    {
        connect(WaitFrameReset, &QState::entered, this, [this]() { logInfo("SM: WaitFrameReset"); });
        connect(WaitExposure,   &QState::entered, this, [this]() { logInfo("SM: WaitExposure"); });
        connect(WaitPulses,     &QState::entered, this, [this]() { logInfo("SM: WaitPulses"); });
    }

    Run->addTransition(this, &BlindPec::Abort, Abort);
    Abort->addTransition(this, &BlindPec::AbortDone, End);

    InitInit->addTransition(this, &BlindPec::InitDone, ReqFrameReset);
    ReqFrameReset->addTransition(this, &BlindPec::RequestFrameResetDone, WaitFrameReset);
    WaitFrameReset->addTransition(this, &BlindPec::FrameResetDone, ReqExposure);
    ReqExposure->addTransition(this, &BlindPec::RequestExposureDone, WaitExposure);
    WaitExposure->addTransition(this, &BlindPec::ExposureDone, Measure);
    Measure->addTransition(this, &BlindPec::MeasureDone, Compute);
    Compute->addTransition(this, &BlindPec::ComputeDone, ReqPulses);
    ReqPulses->addTransition(this, &BlindPec::RequestPulsesDone, WaitPulses);
    ReqPulses->addTransition(this, &BlindPec::PulsesDone, ReqExposure);
    WaitPulses->addTransition(this, &BlindPec::PulsesDone, ReqExposure);

    Run->setInitialState(InitInit);

    _sm.addState(Run);
    _sm.addState(Abort);
    _sm.addState(End);
    _sm.setInitialState(Run);
}

void BlindPec::SMInitInit()
{
    setStateEvent(OST::Busy, "init", "initialisation", "BlindPEC initialize");

    if (!connectDevice(getString("devices", "camera")))
    {
        getProperty("actions")->setState(OST::Error, true);
        setStateEvent(OST::Error, "error", "devicefailed", "camera failed");
        logError("Failed to connect camera device");
        emit Abort();
        return;
    }

    connectIndi();
    connectDevice(getString("devices", "camera"));
    connectDevice(getString("devices", "guider"));

    setBLOBMode(B_ALSO, getString("devices", "camera").toStdString().c_str(), nullptr);
    enableDirectBlobAccess(getString("devices", "camera").toStdString().c_str(), nullptr);
    frameReset(getString("devices", "camera"));
    setFocalLengthAndDiameter();

    setActionRunning(_calibrateOnly ? "calibrate" : "guide");
    getProperty("drift")->clearGrid();
    getProperty("guiding")->clearGrid();

    // Mount info (best effort - none of it is fatal for BlindPEC, it closes the
    // loop on the axis, not the sky).
    getModNumber(getString("devices", "guider"), "EQUATORIAL_EOD_COORD", "RA",  _mountRA);
    getModNumber(getString("devices", "guider"), "EQUATORIAL_EOD_COORD", "DEC", _mountDEC);
    if (!getModNumber(getString("devices", "guider"), "HORIZONTAL_COORD", "ALT", _mountALT))
        logWarning("Could not read mount ALT (refraction rate correction unavailable)");
    getModNumber(getString("devices", "guider"), "HORIZONTAL_COORD", "AZ", _mountAZ);

    double k = 0;
    if (getModNumber(getString("devices", "guider"), "GUIDE_RATE", "GUIDE_RATE_WE", k) && k > 0)
        _guideRateK = k;
    else
        logWarning("Could not read mount GUIDE_RATE - assuming %1x sidereal", {QString::number(_guideRateK, 'f', 2)});

    _meter = pecmeter::Meter(meterParams());
    _consecutiveBad = 0;

    if (_skipChar)
    {
        // Full calibration reused - go straight to guiding (theta/V/scale/G/wDir
        // were loaded in onExternalEvent). The mount must still be tracking.
        enterGuide();
    }
    else
    {
        // Step 1: free-run characterization.
        _phase = PhCharacterize;
        _phaseT0 = nowMs();
        _charFrames = 0;
        _charRejected = 0;
        _charT.clear();
        _charX.clear();
        _charY.clear();
        _charShiftFrac.clear();
        _measX = _measY = 0;
        logInfo("Phase: characterization (free-run drift, %1 s) - mount must be tracking sidereal",
        {QString::number(getFloat("calParams", "chardur"), 'f', 0)});
    }

    setStateEvent(OST::Busy, "initdone", "initialized", "BlindPEC initialized");
    emit InitDone();
}

void BlindPec::SMRequestFrameReset()
{
    if (_trace) logInfo("SM: RequestFrameReset");
    if (!frameReset(getString("devices", "camera")))
    {
        setStateEvent(OST::Error, "error", "devicefailed", "camera failed");
        emit Abort();
        return;
    }
    armWatchdog();
    emit RequestFrameResetDone();

    // Safety net: some camera drivers never bring CCD_FRAME_RESET to IPS_OK
    // (it is a fire-and-forget "clear any partial frame"). Don't hang the whole
    // state machine on it - fall through after a short delay if the real
    // property update hasn't already advanced us.
    QTimer::singleShot(1500, this, [this]()
    {
        if (_trace) logInfo("FrameResetDone (fallback timer)");
        emit FrameResetDone();
    });
}

void BlindPec::SMRequestExposure()
{
    double exp = getFloat("parms", "exposure");
    int gain = getInt("parms", "gain");
    int offset = getInt("parms", "offset");
    if (_trace)
        logInfo("SM: RequestExposure exp=%1s gain=%2 offset=%3",
    {QString::number(exp, 'f', 3), QString::number(gain), QString::number(offset)});

    if (!requestCapture(getString("devices", "camera"), exp, gain, offset))
    {
        setStateEvent(OST::Error, "error", "devicefailed", "camera failed");
        emit Abort();
        return;
    }
    _expectingFrame = true;
    armWatchdog();
    emit RequestExposureDone();
}

void BlindPec::SMMeasure()
{
    disarmWatchdog();   // the exposure BLOB arrived

    // Wrap the raw buffer as a single-channel cv::Mat. Mono microscope / guide
    // camera: 8 or 16 bit. For a 3-channel FITS the planes are stored one after
    // another, so this takes the first (red) plane - acceptable for v1.
    const int cvType = (stats.bytesPerPixel <= 1) ? CV_8UC1 : CV_16UC1;
    cv::Mat raw((int)stats.height, (int)stats.width, cvType, _image->getImageBuffer());

    const double _t0meas = nowMs();
    pecmeter::Sample s = _meter.update(raw);
    _measMs = nowMs() - _t0meas;   // wall-clock cost of the measurement, ms

    // The S-curve (pixel-locking) calibration runs once inside the first update();
    // log its result the first time it becomes available.
    if (!_sCurveLogged && _meter.haveSCurve())
    {
        _sCurveLogged = true;
        logInfo("Pixel-locking S-curve calibrated: peak-to-peak X=%1 px, Y=%2 px - now subtracted from every shift",
        {
            QString::number(_meter.sCurvePeakToPeakX(), 'f', 3),
            QString::number(_meter.sCurvePeakToPeakY(), 'f', 3)
        });
    }

    _measOk     = s.ok;
    _measResp   = s.response;
    _reanchored = s.reanchored;
    _shiftX = s.shiftX;
    _shiftY = s.shiftY;
    if (s.ok)
    {
        _measX = s.x;
        _measY = s.y;
    }

    if (_trace)
        logInfo("SM: Measure %1x%2 -> ok=%3 resp=%4 x=%5 y=%6 step=(%7,%8)%9",
    {
        QString::number(stats.width), QString::number(stats.height),
        s.ok ? "yes" : "NO", QString::number(s.response, 'f', 3),
        QString::number(_measX, 'f', 2), QString::number(_measY, 'f', 2),
        QString::number(s.stepX, 'f', 2), QString::number(s.stepY, 'f', 2),
        s.reanchored ? " [re-anchored]" : ""
    });

    publishFrame();
    emit MeasureDone();
}

void BlindPec::SMCompute()
{
    _pulseE = 0;
    _pulseW = 0;

    static const char *pn[] = {"Init", "GainCal", "Characterize", "Guide"};
    if (_trace) logInfo("SM: Compute (phase %1)", {pn[_phase]});

    switch (_phase)
    {
        case PhGainCal:      computeGainCal();      break;
        case PhCharacterize: computeCharacterize(); break;
        case PhGuide:        computeGuide();        break;
        default:             emit ComputeDone();    break;
    }
}

// ---- Step 1: free-run characterization (mount tracking, no pulses) ---------
// Observe the sidereal drift for a user-set duration. The drift direction gives
// the RA axis orientation in the image (_theta); its rate gives the target rate
// (_V). The residual around the fitted line is the periodic error (not used yet).
void BlindPec::computeCharacterize()
{
    const double dur = getFloat("calParams", "chardur");

    if (_measOk)
    {
        _charT.push_back((nowMs() - _phaseT0) / 1000.0);
        _charX.push_back(_measX);
        _charY.push_back(_measY);
        _charShiftFrac.push_back(_shiftX - std::round(_shiftX));
        _charFrames++;
    }
    else
    {
        _charRejected++;
        if ((_charRejected % 10) == 1)
            logWarning("Characterize: frame not trusted (NCC %1 < %2, step (%3,%4) px) - %5 rejected so far",
        {
            QString::number(_measResp, 'f', 3),
            QString::number(getFloat("measParams", "minresponse"), 'f', 2),
            QString::number(_shiftX, 'f', 1), QString::number(_shiftY, 'f', 1),
            QString::number(_charRejected)
        });
    }

    const double span = _charT.empty() ? 0.0 : (_charT.back() - _charT.front());

    if (span < dur || _charFrames < 10)
    {
        if (_charFrames > 0 && (_charFrames % 20) == 0)
            logInfo("Characterize: %1 frames, %2 s / %3 s (%4 s/frame, meas %5 ms)",
        {
            QString::number(_charFrames), QString::number(span, 'f', 1),
            QString::number(dur, 'f', 0),
            QString::number(_charFrames > 1 ? span / (_charFrames - 1) : 0.0, 'f', 2),
            QString::number(_measMs, 'f', 0)
        });
        emit ComputeDone();   // no pulse
        return;
    }

    fitDriftLine();

    // Scale from the drift alone: the RA axis turns at the sidereal rate
    // (~15.04 arcsec/s of axis rotation), so arcsec/px = sidereal_rate / V.
    // This is the ONLY thing that sets the scale - no optics / focal length /
    // magnification / mount guide rate involved (they are all already folded into
    // the measured V in px/s).
    if (_V > 1e-6)
        _arcsecPerPx = SIDEREAL_ARCSEC_PER_S / _V;

    const double dTot = std::hypot(_charX.back() - _charX.front(), _charY.back() - _charY.front());
    logInfo("Characterization: %1 frames kept / %2 rejected / %3 s | moved %4 px (dx=%5 dy=%6) | theta=%7 deg | V=%8 px/s | scale=%9 arcsec/px",
    {
        QString::number(_charFrames), QString::number(_charRejected),
        QString::number(span, 'f', 1), QString::number(dTot, 'f', 2),
        QString::number(_charX.back() - _charX.front(), 'f', 2), QString::number(_charY.back() - _charY.front(), 'f', 2),
        QString::number(_theta * 180.0 / M_PI, 'f', 1), QString::number(_V, 'f', 4),
        QString::number(_arcsecPerPx, 'f', 3)
    });
    logInfo("Periodic error over the window: peak-to-peak %1 px (%2 arcsec), RMS %3 px (%4 arcsec) | perpendicular RMS %5 px (%6 arcsec, should be small)",
    {
        QString::number(_charPPAlong, 'f', 2), QString::number(_charPPAlong * _arcsecPerPx, 'f', 1),
        QString::number(_charFitRmsAlong, 'f', 3), QString::number(_charFitRmsAlong * _arcsecPerPx, 'f', 2),
        QString::number(_charFitRmsPerp, 'f', 3), QString::number(_charFitRmsPerp * _arcsecPerPx, 'f', 2)
    });
    // Decimated PE curve so it can be read / plotted from the log (cap ~40 lines).
    {
        const int n = (int)_charResidAlong.size();
        const int step = std::max(1, n / 40);
        for (int i = 0; i < n; i += step)
            logInfo("PE  t=%1 s  along=%2 arcsec  (%3 px)  sfrac=%4",
        {
            QString::number(_charT[i], 'f', 1),
            QString::number(_charResidAlong[i] * _arcsecPerPx, 'f', 2),
            QString::number(_charResidAlong[i], 'f', 3),
            QString::number(i < (int)_charShiftFrac.size() ? _charShiftFrac[i] : 0.0, 'f', 2)
        });
    }
    if (_V < 1e-3)
        logWarning("Drift rate V is ~0: is the mount tracking? is the microscope on a "
                   "moving surface? Calibration / guiding will do nothing useful.");
    getEltFloat("calibrationvalues", "V")->setValue(_V);
    getEltFloat("calibrationvalues", "arcsecPerPx")->setValue(_arcsecPerPx);
    getEltFloat("calibrationvalues", "theta")->setValue(_theta * 180.0 / M_PI, true);

    if (getBool("calParams", "observeonly"))
    {
        logInfo("BlindPEC: observe-only run complete (no gain calibration, no guiding)");
        _finishOk = true;
        setStateEvent(OST::Ok, "caldone", "calcompleted", "observation completed");
        emit Abort();
        return;
    }

    if (_skipGainCal)
    {
        logInfo("Reusing stored gain G=%1 ms/px, wDir=%2 - skipping gain calibration",
        {QString::number(_G, 'f', 1), QString::number(_wDir)});
        enterGuide();
        emit ComputeDone();
        return;
    }

    // Step 2 next.
    _phase = PhGainCal;
    _gainStep = 0;
    _gainPending = 0;
    _gainWeff.clear();
    _gainEeff.clear();
    logInfo("Phase: gain calibration (%1 W + %1 E pulses of %2 ms, drift removed with V)",
    {QString::number(getInt("calParams", "calsteps")), QString::number(getInt("calParams", "calpulse"))});
    emit ComputeDone();
}

// ---- Step 2: pulse gain calibration (mount still tracking) ----------------
// Alternate W and E pulses. For each, the projected image move minus the known
// sidereal contribution (V * elapsed) is the pulse-only effect. The antisymmetric
// mean |W|,|E| cancels any residual slow drift; its sign gives the W direction.
void BlindPec::computeGainCal()
{
    const int    calsteps = getInt("calParams", "calsteps");
    const int    calpulse = getInt("calParams", "calpulse");
    const int    total    = 2 * calsteps;

    // Close out the pulse sent on the previous cycle.
    if (_gainPending != 0)
    {
        if (_measOk)
        {
            const double dP   = projRA(_measX, _measY) - _gainSnapP;
            const double dTau = (nowMs() - _gainSnapT) / 1000.0;
            const double eff  = dP - _V * dTau;   // pulse-only, signed along +theta
            if (_gainPending < 0) _gainWeff.push_back(eff);
            else                  _gainEeff.push_back(eff);
            logInfo("Gain cal %1/%2: %3 %4 ms -> %5 px (raw %6, drift -%7)",
            {
                QString::number((int)(_gainWeff.size() + _gainEeff.size())), QString::number(total),
                _gainPending < 0 ? "W" : "E", QString::number(calpulse),
                QString::number(eff, 'f', 2), QString::number(dP, 'f', 2),
                QString::number(_V * dTau, 'f', 2)
            });
        }
        else
        {
            logWarning("Gain cal: frame not trusted (response %1) - repeating this pulse",
            {QString::number(_measResp, 'f', 3)});
        }
        _gainPending = 0;
    }

    if ((int)(_gainWeff.size() + _gainEeff.size()) < total)
    {
        const int dir = (_gainStep % 2 == 0) ? -1 : +1;   // W, E, W, E, ...
        _gainSnapP = projRA(_measX, _measY);
        _gainSnapT = nowMs();
        if (dir < 0) _pulseW = calpulse;
        else         _pulseE = calpulse;
        _gainPending = dir;
        _gainStep++;
        emit ComputeDone();
        return;
    }

    // --- combine ---------------------------------------------------------
    auto mean = [](const std::vector<double> &v)
    {
        double s = 0;
        for (double x : v) s += x;
        return v.empty() ? 0.0 : s / v.size();
    };
    const double mW = mean(_gainWeff);
    const double mE = mean(_gainEeff);
    const double gpx = (std::fabs(mW) + std::fabs(mE)) / 2.0;   // px per calpulse ms

    if (gpx < 1e-3)
    {
        logError("Gain calibration: pulses produced no measurable motion - abort. "
                 "Increase calpulse, or check the mount guide output / illumination.");
        setStateEvent(OST::Error, "error", "calbad", "no motion");
        emit Abort();
        return;
    }

    auto sd = [](const std::vector<double> &v, double m)
    {
        double s = 0;
        for (double x : v) s += (x - m) * (x - m);
        return v.empty() ? 0.0 : std::sqrt(s / v.size());
    };

    _G    = calpulse / gpx;                       // ms per px
    _wDir = (mW >= 0.0) ? +1 : -1;

    const double asym = std::fabs(std::fabs(mW) - std::fabs(mE)) / gpx;
    logInfo("Gain: G=%1 ms/px | wDir=%2 | W=%3 +-%4 px  E=%5 +-%6 px  (per %7 ms pulse) | asym %8%%",
    {
        QString::number(_G, 'f', 2), QString::number(_wDir),
        QString::number(mW, 'f', 2), QString::number(sd(_gainWeff, mW), 'f', 2),
        QString::number(mE, 'f', 2), QString::number(sd(_gainEeff, mE), 'f', 2),
        QString::number(calpulse), QString::number(asym * 100.0, 'f', 0)
    });
    if (asym > 0.5)
        logWarning("Gain calibration: W and E effects differ by %1%% - possible backlash "
                   "or a wrong V. Guiding may be rough.", {QString::number(asym * 100.0, 'f', 0)});

    // Cross-check only (the scale itself comes from step 1): a calpulse-ms pulse
    // at k x sidereal should move the axis by k*15.04*calpulse/1000 arcsec, i.e.
    // that / arcsecPerPx px. Compare to what we measured.
    if (_arcsecPerPx > 1e-6)
    {
        const double expectPx = (_guideRateK * SIDEREAL_ARCSEC_PER_S * calpulse / 1000.0) / _arcsecPerPx;
        logInfo("Gain cross-check: measured %1 px/pulse vs %2 expected at guide rate %3x (%4%% off)",
        {
            QString::number(gpx, 'f', 2), QString::number(expectPx, 'f', 2),
            QString::number(_guideRateK, 'f', 2),
            QString::number(expectPx > 1e-6 ? 100.0 * (gpx - expectPx) / expectPx : 0.0, 'f', 0)
        });
    }

    getEltFloat("calibrationvalues", "G")->setValue(_G);
    getEltInt("calibrationvalues", "wdir")->setValue(_wDir);
    getEltFloat("calibrationvalues", "guideRateK")->setValue(_guideRateK, true);

    if (_stopAfterGainCal)
    {
        _finishOk = true;   // SMAbort: clean finish, don't light abortguider
        setStateEvent(OST::Ok, "caldone", "calcompleted", "calibration completed");
        emit Abort();
        return;
    }

    enterGuide();
    emit ComputeDone();
}

void BlindPec::enterGuide()
{
    _phase = PhGuide;
    setActionRunning("guide");   // switch the lit button from calibrate to guide
    _V0 = _V;                    // freeze the characterized rate for the adaptive clamp
    _meter.reset();
    _measX = _measY = 0;
    _t0Guide = nowMs();
    _tPrev = 0;
    _residual = _residualPrev = 0;
    _errRate = 0;
    _intR = 0;
    _intRsat = false;
    _blank = 0;
    _lastPulseDir = 0;
    _ditherOffset = 0;
    _ditherPending = false;
    _rmsBuf.clear();
    _consecutiveBad = 0;
    _guideFrame = 0;
    _pPrev = 0;
    _setpoint = 0;
    _setpointInit = false;
    getProperty("drift")->clearGrid();
    getProperty("guiding")->clearGrid();
    const int rmsOver = getInt("guideParams", "rmsOver");
    getProperty("drift")->setGridLimit(rmsOver);
    getProperty("guiding")->setGridLimit(rmsOver);
    logInfo("Phase: guiding (theta=%1 deg, V=%2 px/s, G=%3 ms/px, wDir=%4)",
    {
        QString::number(_theta * 180.0 / M_PI, 'f', 1), QString::number(_V, 'f', 4),
        QString::number(_G, 'f', 1), QString::number(_wDir)
    });
    openGuideLog();
    setStateEvent(OST::Busy, "guiding", "startguiding", "start guiding");
}

// ======================================================================
//  PHD2-format GuideLog (written to getWebroot(), openable in PHD Log Viewer)
// ======================================================================
//
// Only a subset of PHD2's header is emitted - PHD Log Viewer tolerates the
// missing INFO / calibration / summary sections. What must be right: the
// "Guiding Begins at", the "Pixel scale =" line, the "Mount = ..." line with
// xAngle / xRate, the exact 18-column header, per-frame rows with Frame/Time
// reset per session, the sign convention (positive RARawDistance -> W pulse),
// and the closing "Guiding Ends at". DEC is not guided here: every DEC column
// is 0 / empty.
void BlindPec::openGuideLog()
{
    if (_guideLog.isOpen())
        _guideLog.close();

    const QString ts    = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    const QString fname  = getWebroot() + "/PHD2_GuideLog_" +
                           QDateTime::currentDateTime().toString("yyyy-MM-dd_HHmmss") + ".txt";
    _guideLog.setFileName(fname);
    if (!_guideLog.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        logWarning("Could not open GuideLog %1 - PHD2-format logging disabled", {fname});
        return;
    }
    _glFrame = 0;
    _glT0    = nowMs();

    // C-locale number formatting (QString::number never localises; unlike arg()).
    auto n = [](double v, int p) { return QString::number(v, 'f', p); };

    const double xAngle   = _theta * 180.0 / M_PI;
    const double xRate    = (_G > 1e-6) ? 1000.0 / _G : 0.0;                 // px/s during a guide pulse
    const double minMove  = (_G > 1e-6) ? getInt("guideParams", "pulsemin") / _G : 0.0;
    const int    expMs    = (int)std::lround(getFloat("parms", "exposure") * 1000.0);
    const double raSpeed  = _guideRateK * SIDEREAL_ARCSEC_PER_S;            // a-s/s

    QString h;
    h += "PHD2 version 2.6.13 [OST BlindPEC], Log version 2.5. Log enabled at " + ts + "\n";
    h += "\n";
    h += "Guiding Begins at " + ts + "\n";
    h += "Equipment Profile = BlindPEC\n";
    h += "Dither = RA only, Dither scale = 1.000, Image noise reduction = none, Guide-frame time lapse = 0, Server disabled\n";
    h += "Pixel scale = " + n(_arcsecPerPx, 2) + " arc-sec/px, Binning = 1, Focal length = 800 mm\n";
    h += "Exposure = " + QString::number(expMs) + " ms\n";
    h += "Mount = \"Mount\", connected, guiding enabled, xAngle = " + n(xAngle, 1) +
         ", xRate = " + n(xRate, 3) + ", yAngle = " + n(xAngle - 90.0, 1) +
         ", yRate = " + n(xRate, 3) + ", parity = +/+\n";
    h += "X guide algorithm = Lead-compensated PI, Minimum move = " + n(minMove, 3) + "\n";
    h += "Y guide algorithm = None, Minimum move = 0.000\n";
    h += "Max RA duration = " + QString::number(getInt("guideParams", "pulsemax")) +
         ", Max DEC duration = 0, DEC guide mode = Off\n";
    h += "RA Guide Speed = " + n(raSpeed, 1) + " a-s/s, Dec Guide Speed = N/A, Cal Dec = 0.0, "
         "Last Cal Issue = None, Timestamp = " + ts + "\n";
    h += "RA = " + n(_mountRA, 2) + " hr, Dec = " + n(_mountDEC, 1) +
         " deg, Hour angle = N/A hr, Pier side = N/A, Rotator pos = N/A, Alt = " + n(_mountALT, 1) +
         " deg, Az = " + n(_mountAZ, 1) + " deg\n";
    h += "Lock position = 0.000, 0.000, Star position = 0.000, 0.000, HFD = 2.00 px\n";
    h += "Frame,Time,mount,dx,dy,RARawDistance,DECRawDistance,RAGuideDistance,DECGuideDistance,"
         "RADuration,RADirection,DECDuration,DECDirection,XStep,YStep,StarMass,SNR,ErrorCode\n";

    _guideLog.write(h.toUtf8());
    _guideLog.flush();
    logInfo("PHD2 GuideLog: %1", {fname});
}

void BlindPec::writeGuideLogRow(double raErr, double crossErr, double needPx)
{
    if (!_guideLog.isOpen())
        return;
    _glFrame++;

    auto n = [](double v, int p) { return QString::number(v, 'f', p); };

    // dx,dy are camera-frame in PHD; rebuild them from the along/cross residual
    // so PHD Log Viewer's scatter plot stays meaningful.
    const double ct = std::cos(_theta), sn = std::sin(_theta);
    const double dx = raErr * ct - crossErr * sn;
    const double dy = raErr * sn + crossErr * ct;

    // PHD sign convention: positive RARawDistance <=> a West pulse. Derive the
    // sign from the direction that nulls the current error (so it is defined
    // even on blank / deadband frames), applying the same wDir / revRA flips the
    // control loop uses.
    double raRawSigned = -raErr * _wDir;
    if (getBool("revCorrections", "revRA"))
        raRawSigned = -raRawSigned;
    const double s = (raRawSigned >= 0.0) ? 1.0 : -1.0;

    const double raRaw   = s * std::fabs(raErr);
    const double raGuide = s * std::fabs(needPx);
    const int    raDurMs = (_pulseW > 0) ? _pulseW : (_pulseE > 0) ? _pulseE : 0;
    const char  *raDir   = (_pulseW > 0) ? "W" : (_pulseE > 0) ? "E" : "";

    // NCC trust metric [~0..1] shown on PHD's SNR axis; StarMass has no analogue.
    const double snr      = _measResp * 100.0;
    const double starMass = 1000.0;
    const double tSec     = (nowMs() - _glT0) / 1000.0;

    QString row = QString::number(_glFrame) + "," + n(tSec, 3) + ",\"Mount\"," +
                  n(dx, 3) + "," + n(dy, 3) + "," +
                  n(raRaw, 3) + ",0.000," + n(raGuide, 3) + ",0.000," +
                  QString::number(raDurMs) + "," + raDir + ",0,,,," +
                  n(starMass, 0) + "," + n(snr, 2) + ",0\n";

    _guideLog.write(row.toUtf8());
    _guideLog.flush();
}

void BlindPec::closeGuideLog()
{
    if (!_guideLog.isOpen())
        return;
    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    _guideLog.write(QString("Guiding Ends at " + ts + "\n\nLog closed at " + ts + "\n").toUtf8());
    _guideLog.flush();
    _guideLog.close();
}

// ---- Phase: guiding ------------------------------------------------------
void BlindPec::computeGuide()
{
    if (!_measOk)
    {
        _consecutiveBad++;
        int maxFail = getInt("measParams", "maxmatchfail");
        logWarning("Untrusted frame (%1/%2, response %3) - skipping",
        {QString::number(_consecutiveBad), QString::number(maxFail), QString::number(_measResp, 'f', 3)});
        if (_consecutiveBad >= maxFail)
        {
            logError("Lost the surface for %1 consecutive frames - abort", {QString::number(_consecutiveBad)});
            setStateEvent(OST::Error, "error", "nostars", "lost correlation");
            emit Abort();
            return;
        }
        emit ComputeDone();
        return;
    }
    _consecutiveBad = 0;

    const double t     = (nowMs() - _t0Guide) / 1000.0;
    const double dt    = (t > _tPrev) ? (t - _tPrev) : 0.0;
    _tPrev = t;
    const double p     = projRA(_measX, _measY);     // along the calibrated RA axis
    const double cross = projCross(_measX, _measY);  // health signal, should stay ~0

    // Incremental setpoint: it advances at the current rate estimate. Unlike
    // p - V*t, changing V later does NOT retroactively jump the residual.
    if (!_setpointInit)
    {
        _setpoint = p;               // first trusted frame: no correction, define the origin
        _setpointInit = true;
    }
    else
    {
        _setpoint += _V * dt;
    }
    _residual = p - _setpoint;

    // RA-only dither: bias the setpoint by a random pixel offset, then let the
    // loop chase it. DEC component of a sequencer dither is a no-op here.
    if (_ditherPending)
    {
        int amp = getInt("guideParams", "ditherpixel");
        double d = (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * amp;
        _ditherOffset += d;
        _ditherPending = false;
        logInfo("Dither: RA offset now %1 px", {QString::number(_ditherOffset, 'f', 1)});
    }
    const double err = _residual - _ditherOffset;

    // Smoothed error rate (px/s) for lead compensation. A raw (err - errPrev)/dt
    // is pure noise; low-pass it over ~1 s (frame-rate independent).
    if (dt > 1e-3)
    {
        const double instRate = (err - _residualPrev) / dt;
        const double beta = 1.0 - std::exp(-dt / 1.0);   // 1 s smoothing
        _errRate += beta * (instRate - _errRate);
    }

    // RMS of the residual (arcsec) over rmsOver frames - computed EVERY frame,
    // blank frames included (a blank frame is still a real pointing error, it is
    // only excluded from *correction*, not from the statistic / the graph).
    const int rmsOver = getInt("guideParams", "rmsOver");
    _rmsBuf.push_back(err * _arcsecPerPx);
    while ((int)_rmsBuf.size() > rmsOver) _rmsBuf.erase(_rmsBuf.begin());
    double rms = 0;
    for (double v : _rmsBuf) rms += square(v);
    rms = _rmsBuf.empty() ? 0 : std::sqrt(rms / _rmsBuf.size());

    _guideFrame++;

    // "blank": the frame right after a pulse (or the first frame) - measure and
    // publish, but do not compute a correction (skip the mechanical transient).
    const bool blank = (_blank > 0);
    if (blank) _blank--;

    // Both corrections disabled -> pure observation: measure/publish only.
    const bool observe = getBool("disCorrections", "disRA+") && getBool("disCorrections", "disRA-");

    double u = 0.0;
    double needPx = 0.0;   // commanded correction along the RA axis (px), post revRA
    if (!blank && !observe)
    {
        // Adaptive rate (PI on V): a wrong V shows up as a SUSTAINED residual the
        // P+I loop can only fight with a permanent pulse bias / integrator DC.
        // Nudge V by the residual VALUE - a persistent err moves V, a zero-mean
        // err leaves it alone. Clamped to +-20% of the characterized rate.
        const double alphaV = getFloat("guideParams", "alphaV");
        if (alphaV > 0)
        {
            _V += alphaV * err;
            _V = qBound(_V0 * 0.8, _V, _V0 * 1.2);
        }

        // P + I (+ D) on the error, in px.
        const double kp = getFloat("pid", "kp");
        const double ki = getFloat("pid", "ki");
        const double kd = getFloat("pid", "kd");
        double intMax = getFloat("guideParams", "intmax");
        if (intMax <= 0) intMax = 50.0;
        if (!_intRsat) _intR += err;
        _intR = qBound(-intMax, _intR, intMax);

        // Lead compensation: correct for where the error WILL be ~1 s from now
        // (the measurement -> pulse-effect latency), using the smoothed rate.
        // This is what lets a slow loop keep pace with a smooth periodic error
        // without a phase-locked table. The setpoint still advances at the fixed
        // V, so `err` stays the true axis error (no "follow the PE" trap).
        const double LEAD = 1.0;   // s
        const double errLead = err + _errRate * LEAD;

        u = kp * errLead + ki * _intR + kd * _errRate;

        needPx = -u;
        if (getBool("revCorrections", "revRA")) needPx = -needPx;

        // A W pulse moves the projected position by sign _wDir; E by -_wDir.
        double pulseMs = std::fabs(needPx) * _G;
        const int dir = ((needPx >= 0) == (_wDir > 0)) ? -1 : +1;   // -1 = W, +1 = E

        const int pmin = getInt("guideParams", "pulsemin");
        const int pmax = getInt("guideParams", "pulsemax");
        if (pulseMs < pmin) pulseMs = 0;
        if (pulseMs > pmax) pulseMs = pmax;

        if (dir < 0 && !getBool("disCorrections", "disRA-")) _pulseW = (int)pulseMs;
        if (dir > 0 && !getBool("disCorrections", "disRA+")) _pulseE = (int)pulseMs;

        _intRsat = (_pulseE >= pmax || _pulseW >= pmax);
        if (_pulseE > 0 || _pulseW > 0)
        {
            _blank = getInt("guideParams", "blankframes");
            _lastPulseDir = dir;
        }
    }

    // Periodic numeric summary (not per frame). 0 = off.
    const int logEvery = getInt("guideParams", "logevery");
    if (logEvery > 0 && (_guideFrame % logEvery) == 0)
    {
        const int spulse = (_pulseE > 0) ? _pulseE : (_pulseW > 0) ? -_pulseW : 0;
        const double sfrac = _shiftX - std::round(_shiftX);   // pixel-locking diagnostic
        logInfo("guide #%1 t=%2s dt=%3s meas=%4ms%5 | resid=%6 px (%7\") | sfrac=%8 | eR=%9 px/s | u=%10 pulse=%11 ms | I=%12 | rms=%13\" resp=%14",
        {
            QString::number(_guideFrame), QString::number(t, 'f', 0), QString::number(dt, 'f', 2),
            QString::number(_measMs, 'f', 0),
            blank ? " BLANK" : "",
            QString::number(err, 'f', 2), QString::number(err * _arcsecPerPx, 'f', 2),
            QString::number(sfrac, 'f', 2),
            QString::number(_errRate, 'f', 3),
            QString::number(u, 'f', 2), QString::number(spulse),
            QString::number(_intR, 'f', 1),
            QString::number(rms, 'f', 2), QString::number(_measResp, 'f', 3)
        });
    }
    _pPrev = p;

    getEltInt("values", "pulseE")->setValue(_pulseE);
    getEltInt("values", "pulseW")->setValue(_pulseW);
    getEltFloat("values", "rmsRA")->setValue(rms);
    getEltFloat("values", "residual")->setValue(err);
    getEltFloat("values", "response")->setValue(_measResp);
    getEltFloat("values", "crossaxis")->setValue(cross, true);

    pushGuiding(err * _arcsecPerPx, rms);

    writeGuideLogRow(err, cross, needPx);

    _residualPrev = err;
    emit ComputeDone();
}

void BlindPec::fitDriftLine()
{
    // Ordinary least-squares slope of x(t) and y(t) over the free run. The drift
    // velocity vector (sx, sy) IS the RA axis: theta = atan2(sy, sx), V = |(sx,sy)|.
    // TODO: Theil-Sen and/or windowing to an integer number of worm periods to
    // remove the periodic-error bias.
    const size_t n = _charT.size();
    if (n < 3)
    {
        logWarning("Characterization: only %1 samples - keeping theta/V unchanged", {QString::number((int)n)});
        return;
    }
    double st = 0, sx = 0, sy = 0, stt = 0, stx = 0, sty = 0;
    for (size_t i = 0; i < n; ++i)
    {
        st  += _charT[i];
        sx  += _charX[i];
        sy  += _charY[i];
        stt += _charT[i] * _charT[i];
        stx += _charT[i] * _charX[i];
        sty += _charT[i] * _charY[i];
    }
    const double denom = n * stt - st * st;
    if (std::fabs(denom) < 1e-9)
        return;
    const double slopeX = (n * stx - st * sx) / denom;
    const double slopeY = (n * sty - st * sy) / denom;
    const double interX = (sx - slopeX * st) / n;
    const double interY = (sy - slopeY * st) / n;

    _V = std::hypot(slopeX, slopeY);
    if (_V > 1e-6)
        _theta = std::atan2(slopeY, slopeX);

    // Residuals of the point cloud around the fitted line, split into the
    // along-axis part (periodic error + noise) and the perpendicular part
    // (should be small; large -> theta wrong / CD wobble / non-planar surface).
    const double ct = std::cos(_theta), sn = std::sin(_theta);
    double sa = 0, sp = 0, amin = 1e30, amax = -1e30;
    _charResidAlong.clear();
    _charResidAlong.reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
        const double rx = _charX[i] - (interX + slopeX * _charT[i]);
        const double ry = _charY[i] - (interY + slopeY * _charT[i]);
        const double a =  rx * ct + ry * sn;   // along axis = periodic error + noise
        const double p = -rx * sn + ry * ct;   // perpendicular
        _charResidAlong.push_back(a);
        sa += a * a;
        sp += p * p;
        if (a < amin) amin = a;
        if (a > amax) amax = a;
    }
    _charFitRmsAlong = std::sqrt(sa / n);
    _charFitRmsPerp  = std::sqrt(sp / n);
    _charPPAlong     = amax - amin;   // peak-to-peak along-axis = PE peak-to-peak over the window
}

// ======================================================================
//  Pulses (RA / West-East only)
// ======================================================================
void BlindPec::SMRequestPulses()
{
    if (_trace && (_pulseE > 0 || _pulseW > 0))
        logInfo("SM: RequestPulses E=%1 W=%2 ms", {QString::number(_pulseE), QString::number(_pulseW)});

    INDI::BaseDevice dp = getDevice(getString("devices", "guider").toStdString().c_str());

    auto sendWE = [&](const char *elt, int value)
    {
        INDI::PropertyNumber prop = dp.getNumber("TELESCOPE_TIMED_GUIDE_WE");
        for (std::size_t i = 0; i < prop.size(); i++)
            prop[i].value = (strcmp(prop[i].name, elt) == 0) ? value : 0;
        sendNewNumber(prop);
    };

    if (_pulseE > 0)
    {
        _pulseRAfinished = false;
        sendWE("TIMED_GUIDE_E", _pulseE);
    }
    else if (_pulseW > 0)
    {
        _pulseRAfinished = false;
        sendWE("TIMED_GUIDE_W", _pulseW);
    }

    emit RequestPulsesDone();

    if (_pulseE == 0 && _pulseW == 0)
        emit PulsesDone();     // nothing to send this cycle
    else
        armWatchdog();         // waiting for TELESCOPE_TIMED_GUIDE_WE -> IPS_IDLE
}

// ======================================================================
//  Preview + graphs
// ======================================================================
void BlindPec::publishFrame()
{
    QImage rawImage = _image->getRawQImage();
    QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
    im.setColorTable(rawImage.colorTable());
    {
        QPainter painter(&im);
        // Cross-hair at the frame centre + a short vector showing the measured
        // cumulative displacement (scaled down, just for a visual sanity check).
        const double cx = im.width() / 2.0, cy = im.height() / 2.0;
        painter.setPen(QPen(Qt::green, 1));
        painter.drawLine(QPointF(cx - 12, cy), QPointF(cx + 12, cy));
        painter.drawLine(QPointF(cx, cy - 12), QPointF(cx, cy + 12));
        painter.setPen(QPen(_measOk ? Qt::red : Qt::yellow, 2));
        painter.drawLine(QPointF(cx, cy),
                         QPointF(cx + _measX / 2.0, cy + _measY / 2.0));
    }
    atomicSaveJpeg(im, getWebroot() + "/" + getModuleName() + ".jpeg");

    OST::ImgData dta = _image->ImgStats();
    dta.mUrlJpeg = getModuleName() + ".jpeg";
    getEltImg("image", "image")->setValue(dta, true);
}

// BlindPEC is single-axis: only the RA residual is a guiding quantity. The DEC /
// DE fields of the reused guider graphs are pinned to 0 so nothing reads as a
// second controlled axis. The perpendicular ("cross-axis") component is a health
// signal only and is published as values/crossaxis, not on these graphs.
void BlindPec::pushGuiding(double raArcsec, double rms)
{
    getEltFloat("drift", "RA")->setValue(raArcsec);
    getEltFloat("drift", "DEC")->setValue(0, true);
    getProperty("drift")->push();

    getEltFloat("guiding", "time")->setValue(nowMs());
    getEltFloat("guiding", "RA")->setValue(raArcsec);
    getEltFloat("guiding", "DE")->setValue(0);
    getEltFloat("guiding", "pRA")->setValue(_pulseE - _pulseW);
    getEltFloat("guiding", "pDE")->setValue(0);
    getEltFloat("guiding", "SNR")->setValue(_measResp);
    getEltFloat("guiding", "RMS")->setValue(rms);
    getProperty("guiding")->push();

    setStateEvent(OST::Busy, "guiding", "guideRMS", "guide RMS", rms);
}

// ======================================================================
//  Abort
// ======================================================================
void BlindPec::SMAbort()
{
    const bool ok = _finishOk;   // true = we reached here from a clean finish
    _finishOk = false;

    logInfo(ok ? "BlindPEC: stopping (calibration done)" : "Aborting BlindPEC");
    disarmWatchdog();
    closeGuideLog();
    _expectingFrame = false;
    _pulseE = _pulseW = 0;

    // Clear every action button; on a real abort also light abortguider (guider
    // pattern), on a clean finish leave them all off.
    setActionRunning("");
    if (!ok)
        getEltBool("actions", "abortguider")->setValue(true, true);

    _sm.stop();

    emit AbortDone();
    QTimer::singleShot(0, this, [this, ok]()
    {
        if (ok)
        {
            setStateEvent(OST::Ok, "ready", "calcompleted", "BlindPEC calibration done");
            logInfo("BlindPEC calibration done");
        }
        else
        {
            setStateEvent(OST::Ok, "ready", "abortguide", "BlindPEC ready");
            logInfo("BlindPEC aborted");
        }
    });
}
