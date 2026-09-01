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

pecmeter::Params BlindPec::meterParams()
{
    pecmeter::Params p;
    if (getEltFloat("guideParams", "reanchorfrac")) p.reanchorFrac = getFloat("guideParams", "reanchorfrac");
    if (getEltFloat("guideParams", "maxsteppx"))    p.maxStepPx    = getFloat("guideParams", "maxsteppx");
    if (getEltFloat("guideParams", "minresponse"))  p.minResponse  = getFloat("guideParams", "minresponse");
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
        getProperty(event.prpkey)->setState(OST::Ok, true);

        _calibrateOnly = (event.eltkey == "calibrate");

        _G           = getFloat("calibrationvalues", "G");
        _arcsecPerPx = getFloat("calibrationvalues", "arcsecPerPx");
        _guideRateK  = getFloat("calibrationvalues", "guideRateK");
        if (_guideRateK <= 0) _guideRateK = 0.5;

        // "calibrate" always redoes the gain. "guide" reuses a stored gain if we
        // have one (and always re-characterizes V, which drifts with temperature).
        _phase = (_calibrateOnly || _G <= 0.0) ? PhGainCal : PhCharacterize;

        logInfo(_calibrateOnly ? "BlindPEC: calibration only" : "BlindPEC: calibrate + guide");
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
        logInfo("FrameResetDone (CCD_FRAME_RESET -> IPS_OK)");
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

    getProperty("actions")->setState(OST::Busy, true);
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

    // Prepare the phase we were told to run.
    _meter = pecmeter::Meter(meterParams());
    _consecutiveBad = 0;
    _phaseT0 = nowMs();

    if (_phase == PhGainCal)
    {
        _gainStep = 0;
        _gainDeltas.clear();
        _gainPulseMs = getInt("calParams", "calpulse");
        _G = 0;
        logInfo("Phase: gain calibration (%1 x %2 ms W pulses)",
        {QString::number(getInt("calParams", "calsteps")), QString::number((int)_gainPulseMs)});
    }
    else
    {
        _charFrames = 0;
        _charT.clear();
        _charP.clear();
        logInfo("Phase: characterization (%1 frames), gain G = %2 ms/px (loaded)",
        {QString::number(getInt("guideParams", "warmupframes")), QString::number(_G, 'f', 1)});
    }

    setStateEvent(OST::Busy, "initdone", "initialized", "BlindPEC initialized");
    emit InitDone();
}

void BlindPec::SMRequestFrameReset()
{
    logInfo("SM: RequestFrameReset");
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
        logInfo("FrameResetDone (fallback timer)");
        emit FrameResetDone();
    });
}

void BlindPec::SMRequestExposure()
{
    double exp = getFloat("parms", "exposure");
    int gain = getInt("parms", "gain");
    int offset = getInt("parms", "offset");
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

    pecmeter::Sample s = _meter.update(raw);

    _measOk     = s.ok;
    _measResp   = s.response;
    _reanchored = s.reanchored;
    if (s.ok)
    {
        _measX = s.x;
        _measY = s.y;
    }

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

// ---- Phase: gain calibration -----------------------------------------------
void BlindPec::computeGainCal()
{
    const int calsteps = getInt("calParams", "calsteps");

    // Record the effect of the pulse sent on the previous cycle.
    if (_gainStep > 0 && _measOk)
    {
        const double moved = std::hypot(_measX - _gainRefX, _measY - _gainRefY);
        _gainDeltas.push_back(moved);
        logInfo("Gain cal %1/%2: pulse %3 ms -> moved %4 px",
        {
            QString::number(_gainStep), QString::number(calsteps),
            QString::number((int)_gainPulseMs), QString::number(moved, 'f', 2)
        });
    }
    else if (_gainStep > 0 && !_measOk)
    {
        logWarning("Gain cal %1: frame not trusted (response %2) - repeating",
        {QString::number(_gainStep), QString::number(_measResp, 'f', 3)});
        _gainStep--;   // retry this step
    }

    if ((int)_gainDeltas.size() < calsteps)
    {
        _gainRefX = _measX;
        _gainRefY = _measY;
        _pulseW = (int)_gainPulseMs;   // measured next cycle
        _gainStep++;
        emit ComputeDone();
        return;
    }

    // Combine.
    double mean = 0;
    for (double d : _gainDeltas) mean += d;
    mean /= _gainDeltas.size();

    if (mean < 1e-3)
    {
        logError("Gain calibration: pulses produced no measurable motion - abort. "
                 "Increase calpulse or check the mount / illumination.");
        setStateEvent(OST::Error, "error", "calbad", "no motion");
        emit Abort();
        return;
    }

    _G = _gainPulseMs / mean;   // ms of pulse per px
    // Target rate from the gain (notes s.3 method 1): a P-ms pulse at k x sidereal
    // moves the image by `mean` px, so V = mean * 1000 / (k * P)  [px/s].
    _V = mean * 1000.0 / (_guideRateK * _gainPulseMs);
    _arcsecPerPx = (_guideRateK * SIDEREAL_ARCSEC_PER_S * _gainPulseMs / 1000.0) / mean;

    logInfo("Gain: G = %1 ms/px | seed V = %2 px/s | scale = %3 arcsec/px (guideRate %4x)",
    {
        QString::number(_G, 'f', 2), QString::number(_V, 'f', 4),
        QString::number(_arcsecPerPx, 'f', 4), QString::number(_guideRateK, 'f', 2)
    });

    getEltFloat("calibrationvalues", "G")->setValue(_G);
    getEltFloat("calibrationvalues", "arcsecPerPx")->setValue(_arcsecPerPx);
    getEltFloat("calibrationvalues", "guideRateK")->setValue(_guideRateK, true);

    // Move on to characterization.
    _phase = PhCharacterize;
    _charFrames = 0;
    _charT.clear();
    _charP.clear();
    _meter.reset();
    _measX = _measY = 0;
    _phaseT0 = nowMs();
    setStateEvent(OST::Busy, "cal", "gaindone", "gain calibrated");
    emit ComputeDone();
}

// ---- Phase: characterization (fit the target rate V) ----------------------
void BlindPec::computeCharacterize()
{
    const int    want = getInt("guideParams", "warmupframes");
    const double  minSecs = getFloat("guideParams", "charminsecs");

    if (_measOk)
    {
        _charT.push_back((nowMs() - _phaseT0) / 1000.0);
        _charP.push_back(_measX);
        _charFrames++;
    }

    const double span = _charT.empty() ? 0.0 : (_charT.back() - _charT.front());

    // Need BOTH: enough frames AND enough wall-clock time. A slow sidereal drift
    // (a few px/s at most) cannot be measured over a fraction of a second even
    // with hundreds of frames.
    if (_charFrames < want || span < minSecs)
    {
        if (_trace && (_charFrames % 20) == 0)
            logInfo("Characterize: %1/%2 frames, span %3 s / %4 s, p=%5 px",
        {
            QString::number(_charFrames), QString::number(want),
            QString::number(span, 'f', 1), QString::number(minSecs, 'f', 0),
            QString::number(_measX, 'f', 2)
        });
        emit ComputeDone();   // no pulse
        return;
    }

    fitTargetRate();

    const double dP = _charP.empty() ? 0.0 : (_charP.back() - _charP.front());
    logInfo("Characterization: %1 frames over %2 s, image moved %3 px -> V = %4 px/s (%5 arcsec/s)",
    {
        QString::number(_charFrames), QString::number(span, 'f', 1),
        QString::number(dP, 'f', 2), QString::number(_V, 'f', 4),
        QString::number(_V * _arcsecPerPx, 'f', 3)
    });
    if (std::fabs(_V) < 1e-3)
        logWarning("Target rate V is ~0: is the mount actually tracking? is the "
                   "microscope on a moving surface? Guiding will do nothing useful.");
    getEltFloat("calibrationvalues", "V")->setValue(_V, true);

    if (_calibrateOnly)
    {
        logInfo("BlindPEC: calibration complete");
        getProperty("actions")->setState(OST::Ok, true);
        setStateEvent(OST::Ok, "caldone", "calcompleted", "calibration completed");
        emit Abort();   // TODO: a proper clean stop instead of reusing Abort
        return;
    }

    // Enter guiding.
    _phase = PhGuide;
    _meter.reset();
    _measX = _measY = 0;
    _t0Guide = nowMs();
    _residual = _residualPrev = 0;
    _intR = 0;
    _intRsat = false;
    _blank = 0;
    _lastPulseDir = 0;
    _rmsBuf.clear();
    _consecutiveBad = 0;
    getProperty("drift")->clearGrid();
    getProperty("guiding")->clearGrid();
    int rmsOver = getInt("guideParams", "rmsOver");
    getProperty("drift")->setGridLimit(rmsOver);
    getProperty("guiding")->setGridLimit(rmsOver);
    setStateEvent(OST::Busy, "guiding", "startguiding", "start guiding");
    emit ComputeDone();
}

// ---- Phase: guiding ------------------------------------------------------
void BlindPec::computeGuide()
{
    if (!_measOk)
    {
        _consecutiveBad++;
        int maxFail = getInt("guideParams", "maxmatchfail");
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

    const double t = (nowMs() - _t0Guide) / 1000.0;
    const double p = _measX;                 // RA axis = image X for v1
    const double cross = _measY;             // health signal, should stay ~0

    _residual = p - _V * t;

    // RA-only dither: bias the target position by a random pixel offset, then
    // let the loop chase it. DEC component of a sequencer dither is a no-op here.
    if (_ditherPending)
    {
        int amp = getInt("guideParams", "ditherpixel");
        double d = (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * amp;
        _ditherOffset += d;
        _ditherPending = false;
        logInfo("Dither: RA offset now %1 px", {QString::number(_ditherOffset, 'f', 1)});
    }
    const double err = _residual - _ditherOffset;

    // Right after a pulse: measure but don't correct (skip the mechanical transient).
    if (_blank > 0)
    {
        _blank--;
        _residualPrev = _residual;
        pushGuiding(err * _arcsecPerPx, cross * _arcsecPerPx, 0);
        emit ComputeDone();
        return;
    }

    // Adaptive target rate: a wrong V shows up as a slow ramp in `err`; nudge V
    // to flatten it so the integrator never has to carry the DC. (notes s.3/s.8)
    double alphaV = getFloat("guideParams", "alphaV");
    if (alphaV > 0)
        _V += alphaV * (err - _residualPrev) / std::max(0.001, t - _tPrev);
    _tPrev = t;

    // P + I (+ D) on the error, in px.
    double kp = getFloat("pid", "kp");
    double ki = getFloat("pid", "ki");
    double kd = getFloat("pid", "kd");
    double intMax = getInt("guideParams", "pulsemax") * 0.0 + 50.0; // TODO: expose intmax; 50 px clamp for now
    if (!_intRsat) _intR += err;
    _intR = qBound(-intMax, _intR, intMax);
    double corrPx = kp * err + ki * _intR + kd * (err - _residualPrev);

    // px -> ms, then sense.
    double pulseMs = std::fabs(corrPx) * _G;
    int dir = (corrPx > 0) ? +1 : -1;
    if (getBool("revCorrections", "revRA")) dir = -dir;

    int pmin = getInt("guideParams", "pulsemin");
    int pmax = getInt("guideParams", "pulsemax");
    if (pulseMs < pmin) pulseMs = 0;
    if (pulseMs > pmax) pulseMs = pmax;

    bool disP = getBool("disCorrections", "disRA+");
    bool disM = getBool("disCorrections", "disRA-");
    if (dir > 0 && !disP) _pulseE = (int)pulseMs;
    if (dir < 0 && !disM) _pulseW = (int)pulseMs;

    _intRsat = (_pulseE >= pmax || _pulseW >= pmax);
    if (_pulseE > 0 || _pulseW > 0)
    {
        _blank = getInt("guideParams", "blankframes");
        _lastPulseDir = dir;
    }

    // RMS of the residual (arcsec), bounded by rmsOver.
    int rmsOver = getInt("guideParams", "rmsOver");
    _rmsBuf.push_back(err * _arcsecPerPx);
    while ((int)_rmsBuf.size() > rmsOver) _rmsBuf.erase(_rmsBuf.begin());
    double rms = 0;
    for (double v : _rmsBuf) rms += square(v);
    rms = _rmsBuf.empty() ? 0 : std::sqrt(rms / _rmsBuf.size());

    getEltInt("values", "pulseE")->setValue(_pulseE);
    getEltInt("values", "pulseW")->setValue(_pulseW);
    getEltFloat("values", "rmsRA")->setValue(rms);
    getEltFloat("values", "residual")->setValue(err);
    getEltFloat("values", "response")->setValue(_measResp);
    getEltFloat("values", "crossaxis")->setValue(cross, true);

    pushGuiding(err * _arcsecPerPx, cross * _arcsecPerPx, rms);

    _residualPrev = err;
    emit ComputeDone();
}

void BlindPec::fitTargetRate()
{
    // v1: ordinary least-squares slope of (t, p). TODO: Theil-Sen / window to an
    // integer number of worm periods to kill the PE bias.
    const size_t n = _charT.size();
    if (n < 3)
    {
        logWarning("Characterization: too few samples (%1) - keeping seed V = %2",
        {QString::number((int)n), QString::number(_V, 'f', 4)});
        return;
    }
    double st = 0, sp = 0, stt = 0, stp = 0;
    for (size_t i = 0; i < n; ++i)
    {
        st += _charT[i];
        sp += _charP[i];
        stt += _charT[i] * _charT[i];
        stp += _charT[i] * _charP[i];
    }
    double denom = n * stt - st * st;
    if (std::fabs(denom) < 1e-9)
        return;
    double slope = (n * stp - st * sp) / denom;
    if (std::isfinite(slope) && slope != 0.0)
        _V = slope;
}

// ======================================================================
//  Pulses (RA / West-East only)
// ======================================================================
void BlindPec::SMRequestPulses()
{
    if (_pulseE > 0 || _pulseW > 0)
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

void BlindPec::pushGuiding(double raArcsec, double deArcsec, double rms)
{
    getEltFloat("drift", "RA")->setValue(raArcsec);
    getEltFloat("drift", "DEC")->setValue(deArcsec, true);
    getProperty("drift")->push();

    getEltFloat("guiding", "time")->setValue(nowMs());
    getEltFloat("guiding", "RA")->setValue(raArcsec);
    getEltFloat("guiding", "DE")->setValue(deArcsec);
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
    logInfo("Aborting BlindPEC");
    disarmWatchdog();
    _expectingFrame = false;
    _pulseE = _pulseW = 0;

    getEltBool("actions", "calibrate")->setValue(false, false);
    getEltBool("actions", "guide")->setValue(false, false);
    getEltBool("actions", "abortguider")->setValue(false, false);
    getEltBool("actions", "abortguider")->setValue(true, true);
    getProperty("actions")->setState(OST::Ok, true);

    _sm.stop();

    emit AbortDone();
    QTimer::singleShot(0, this, [this]()
    {
        setStateEvent(OST::Ok, "ready", "abortguide", "BlindPEC ready");
        logInfo("BlindPEC aborted");
    });
}
