/**
 * @file guider.cpp
 * @brief Autoguiding module for OST - Corrects telescope tracking drift
 *
 * This module implements a complete autoguiding system with three phases:
 *   1. INITIALIZATION: Connect devices, capture reference star field
 *   2. CALIBRATION: Measure how many pixels correspond to 1ms pulse in each direction
 *   3. GUIDING: Continuous loop - detect drift, send correcting pulses
 *
 * Algorithm: the star field drift between frames is measured by voting for the
 * dominant translation between the two star lists, then a robust refit (see
 * startracker.{h,cpp}).
 *
 * Workflow:
 *   User clicks "Calibrate & Guide" → SMInit → SMCalibration → SMGuide (loops)
 *
 * Key compensation:
 *   - DEC compensation: RA pulses scaled by cos(mount_DEC) (critical at high latitudes!)
 *   - Pier-side: Optionally reverses RA/DEC corrections when mount flips (configurable)
 *
 * @note RMS buffers (_dRAvector, _dDEvector) are bounded by rmsOver parameter from guideParams
 * @todo Implement full PID controller (currently P only - good enough for most mounts)
 * @todo Add timeout/retry mechanism for INDI command failures
 */

#include "guider.h"
#include "version.cc"
#include <QRandomGenerator>
#include <QPainter>

static void atomicSaveJpeg(const QImage &img, const QString &finalPath)
{
    const QString tmp = finalPath + ".tmp";
    if (img.save(tmp, "JPG", 100))
        ::rename(tmp.toLocal8Bit().constData(), finalPath.toLocal8Bit().constData());
}

/// Convert a solver star list to the star-tracker's lightweight representation.
static QVector<startracker::Star> toTrackStars(const QList<FITSImage::Star> &in)
{
    QVector<startracker::Star> out;
    out.reserve(in.size());
    for (const FITSImage::Star &s : in)
        out.push_back({s.x, s.y, s.flux});
    return out;
}

#define PI 3.14159265

/**
 * @brief Plugin entry point - called by OST to load module
 * @return New Guider instance
 */
Guider *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Guider *basemodule = new Guider(name, label, profile, availableModuleLibs);
    return basemodule;
}

/**
 * @brief Constructor - Initialize guider module
 *
 * Sets up:
 *   - Properties from guider.json (parameters, results, grids)
 *   - Three state machines (Init, Calibration, Guide)
 *   - INDI device requirements (camera, guider mount interface)
 *   - Module role declaration (defineMeAsGuider)
 *   - Custom "Reset calibration" button
 */
Guider::Guider(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{
    // Load property definitions from guider.json
    loadOstPropertiesFromFile(":guider.json");

    // Set module metadata
    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Guider module");
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("template", "guider");

    // Build the three state machines that orchestrate the guiding workflow
    buildInitStateMachines();
    buildCalStateMachines();
    buildGuideStateMachines();

    // Declare required INDI devices
    giveMeADevice("camera", "Camera", INDI::BaseDevice::CCD_INTERFACE);      // Guiding camera
    giveMeADevice("guider", "Guide via", INDI::BaseDevice::GUIDER_INTERFACE); // Mount guide interface

    // Register this module as the guider
    defineMeAsGuider();

    // Add custom "Reset calibration" button to actions (not defined in JSON)
    OST::PropertyMulti* pm = getProperty("actions");
    OST::ElementBool* b = new OST::ElementBool("resetcalibration", "Reset calibration", "guid10", "");
    b->setValue(false, false);
    b->setPreIcon("block");
    pm->addElt(b);
    OST::ElementBool* dither = new OST::ElementBool("dither", "Dither", "guid11", "");
    dither->setValue(false, false);
    dither->setPreIcon("shuffle");
    pm->addElt(dither);

    // Watchdog: the state machines wait on INDI callbacks (exposure BLOB, pulse
    // completion, frame reset, SEP). If one never arrives the machine would hang
    // forever - fire an Abort instead. It is single-shot and re-armed at every
    // hardware step (start() restarts it), so it is never stopped from the INDI
    // / solver callback threads - only from SMAbort (module thread). A stray
    // fire after a sequence finished cleanly just no-ops.
    _watchdog.setSingleShot(true);
    connect(&_watchdog, &QTimer::timeout, this, [this]()
    {
        if (!_SMInit.isRunning() && !_SMCalibration.isRunning() && !_SMGuide.isRunning())
            return;
        logError("Watchdog: no hardware response for %1 s - aborting", {QString::number(_watchdog.interval() / 1000)});
        setStateEvent(OST::Error, "error", "timeout", "hardware timeout");
        emit Abort();
    });
}

void Guider::armWatchdog()
{
    int t = getInt("guideParams", "watchdog");
    if (t > 0)
        _watchdog.start(t * 1000);
}

void Guider::disarmWatchdog()
{
    _watchdog.stop();   // module thread only (SMAbort)
}

/**
 * @brief Destructor
 */
Guider::~Guider()
{
    // Cleanup (if needed)
}
/**
 * @brief Handle external events from other modules (mainly sequencer)
 *
 * Two main types of events:
 *   1. Suspend/Resume guiding - from sequencer during autofocus
 *   2. Action buttons - from UI (calguide, calibrate, guide, abortguider, resetcalibration)
 *
 * State machine orchestration:
 *   calguide:   Init → Calibration → Guide (full workflow)
 *   calibrate:  Init → Calibration only
 *   guide:      Init → Guide (or Init → Calibration → Guide if no prior calib)
 *   abortguider: Stop all state machines
 *   resetcalibration: Clear calibration data (forces recalibration)
 */
void Guider::onExternalEvent(OST::ExtEvent event)
{
    // === EVENTS FROM SEQUENCER ===

    // When sequencer starts autofocus, request guiding suspension
    // This prevents focus movement from corrupting guidng measurements
    //if (e.type == "suspendguiding" && getModuleName() == e.module)
    //{
    //    logInfo("Guiding suspended by external request (focus in progress)");
    //    _SMGuide.stop();  // Pause the guiding loop
    //    return;
    //}

    // When sequencer finishes autofocus, resume guiding
    // Restart from initialization to redetect guide star and recompute reference
    //if (e.type == "resumeguiding" && getModuleName() == e.module)
    //{
    //    logInfo("Resuming guiding after external suspension (focus completed)");
    //    // Reconnect state machines: Init → Guide
    //    disconnect(&_SMInit,        &QStateMachine::finished, nullptr, nullptr);
    //    disconnect(&_SMCalibration, &QStateMachine::finished, nullptr, nullptr);
    //    connect(&_SMInit,           &QStateMachine::finished, &_SMGuide, &QStateMachine::start);
    //    _SMInit.start();
    //    return;
    //}

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        if (event.eltkey == "calguide")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                getProperty(event.prpkey)->setState(OST::Busy, true);
                logInfo("Starting full calibration and guiding");
                // Wire state machines: Init → Calibration → Guide
                disconnect(&_SMInit,        &QStateMachine::finished, nullptr, nullptr);
                disconnect(&_SMCalibration, &QStateMachine::finished, nullptr, nullptr);
                connect(&_SMInit,           &QStateMachine::finished, &_SMCalibration, &QStateMachine::start);
                connect(&_SMCalibration,    &QStateMachine::finished, &_SMGuide, &QStateMachine::start);
                _SMInit.start();
            }
        }
        if (event.eltkey == "abortguider")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                emit Abort();  // Triggers all state machines to abort state
            }
        }
        if (event.eltkey == "calibrate")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(false, true))
            {
                getProperty(event.prpkey)->setState(OST::Ok, true);
                logInfo("Starting calibration only");
                // Wire state machines: Init → Calibration (no Guide)
                disconnect(&_SMInit,        &QStateMachine::finished, nullptr, nullptr);
                disconnect(&_SMCalibration, &QStateMachine::finished, nullptr, nullptr);
                connect(&_SMInit,           &QStateMachine::finished, &_SMCalibration, &QStateMachine::start);
                _SMInit.start();
            }
        }
        if (event.eltkey == "guide")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(false, true))
            {
                getProperty(event.prpkey)->setState(OST::Ok, true);
                logInfo("Starting guiding");

                // If no calibration, must do it first
                if (getFloat("calibrationvalues", "calPulseRA") == 0
                        || getFloat("calibrationvalues", "calPulseDE") == 0)
                {
                    logInfo("No calibration data found - starting calibration first");
                    disconnect(&_SMInit,        &QStateMachine::finished, nullptr, nullptr);
                    disconnect(&_SMCalibration, &QStateMachine::finished, nullptr, nullptr);
                    connect(&_SMInit,           &QStateMachine::finished, &_SMCalibration, &QStateMachine::start);
                    connect(&_SMCalibration,    &QStateMachine::finished, &_SMGuide, &QStateMachine::start);
                    _SMInit.start();
                }
                else
                {
                    // Wire: Init → Guide (skip Calibration)
                    disconnect(&_SMInit,        &QStateMachine::finished, nullptr, nullptr);
                    disconnect(&_SMCalibration, &QStateMachine::finished, nullptr, nullptr);
                    connect(&_SMInit,           &QStateMachine::finished, &_SMGuide, &QStateMachine::start);
                    _SMInit.start();
                }
            }
        }
        if (event.eltkey == "dither")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(false, true))
            {
                if (_SMGuide.isRunning())
                {
                    _doDither = true;
                    logInfo("Dither requested - will apply on next guide frame");
                }
                else
                {
                    logWarning("Dither ignored - guiding is not active");
                }
            }
        }
        if (event.eltkey == "resetcalibration")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(false, true))
            {
                getProperty(event.prpkey)->setState(OST::Ok, true);
                logInfo("Resetting calibration data");
                // Clear all calibration values to force recalibration
                getEltFloat("calibrationvalues", "calPulseRA")->setValue(0);
                getEltFloat("calibrationvalues", "calPulseDE")->setValue(0);
                getEltFloat("calibrationvalues", "calPulseN")->setValue(0);
                getEltFloat("calibrationvalues", "calPulseS")->setValue(0);
                getEltFloat("calibrationvalues", "calPulseE")->setValue(0);
                getEltFloat("calibrationvalues", "calPulseW")->setValue(0);
                getEltFloat("calibrationvalues", "ccdOrientation")->setValue(0);
                getEltFloat("calibrationvalues", "calMountDEC")->setValue(0);
                getEltBool("calibrationvalues", "revRA")->setValue(false);
                getEltBool("calibrationvalues", "revDE")->setValue(false, true);
                logInfo("Calibration data reset - recalibration required before guiding");
            }
        }
    }
}

void Guider::updateProperty(INDI::Property property)
{
    if (strcmp(property.getName(), "CCD1") == 0)
    {
        newBLOB(property);
    }
    if (
        (property.getDeviceName() == getString("devices", "camera"))
        &&  (QString(property.getName()) == "CCD_FRAME_RESET")
        &&  (property.getState() == IPS_OK)
    )
    {
        //logInfo("FrameResetDone");
        emit FrameResetDone();
    }
    if (
        (property.getDeviceName() == getString("devices", "guider")) &&
        (QString(property.getName())   == "TELESCOPE_TIMED_GUIDE_NS") &&
        (property.getState()  == IPS_IDLE)

    )
    {
        _pulseDECfinished = true;
    }

    if (
        (property.getDeviceName() == getString("devices", "guider")) &&
        (QString(property.getName())  == "TELESCOPE_TIMED_GUIDE_WE") &&
        (property.getState()  == IPS_IDLE)

    )
    {
        _pulseRAfinished = true;
    }

    if (
        (property.getDeviceName() == getString("devices", "guider")) &&
        ( (QString(property.getName())   == "TELESCOPE_TIMED_GUIDE_WE") ||
          (QString(property.getName())  == "TELESCOPE_TIMED_GUIDE_NS") ) &&
        (property.getState()  == IPS_IDLE)

    )
    {
        if (_pulseRAfinished && _pulseDECfinished)
            emit PulsesDone();
    }


}

void Guider::newBLOB(INDI::PropertyBlob pblob)
{
    // Only consume a frame we actually asked for: ignore stray/late BLOBs
    // (e.g. one arriving after an abort, or from another module sharing the
    // camera) so they can't advance a state machine unexpectedly.
    if (!_expectingFrame)
        return;
    if (QString(pblob.getDeviceName()) != getString("devices", "camera"))
        return;

    _expectingFrame = false;

    delete _image;
    _image = new fileio();
    _image->loadBlob(pblob, 64);
    stats = _image->getStats();

    emit ExposureDone();
}

void Guider::buildInitStateMachines(void)
{
    /* Initialization statemachine = SMInit */

    auto *Abort = new QState();
    auto *Init  = new QState();
    auto *End   = new QFinalState();

    auto *InitInit             = new QState(Init);
    auto *RequestFrameReset    = new QState(Init);
    auto *WaitFrameReset       = new QState(Init);
    auto *RequestFirstExposure = new QState(Init);
    auto *WaitFirstExposure    = new QState(Init);
    auto *FindStarsFirst       = new QState(Init);
    auto *ComputeFirst         = new QState(Init);

    connect(InitInit, &QState::entered, this, &Guider::SMInitInit);
    connect(RequestFrameReset, &QState::entered, this, &Guider::SMRequestFrameReset);
    connect(RequestFirstExposure, &QState::entered, this, &Guider::SMRequestExposure);
    connect(FindStarsFirst, &QState::entered, this, &Guider::SMFindStars);
    connect(ComputeFirst, &QState::entered, this, &Guider::SMComputeFirst);
    connect(Abort,               &QState::entered, this, &Guider::SMAbort);

    Init->                addTransition(this, &Guider::Abort, Abort);
    Abort->               addTransition(this, &Guider::AbortDone, End);
    InitInit->            addTransition(this, &Guider::InitDone, RequestFrameReset);
    RequestFrameReset->   addTransition(this, &Guider::RequestFrameResetDone, WaitFrameReset);
    WaitFrameReset->      addTransition(this, &Guider::FrameResetDone, RequestFirstExposure);
    RequestFirstExposure->addTransition(this, &Guider::RequestExposureDone, WaitFirstExposure);
    WaitFirstExposure->   addTransition(this, &Guider::ExposureDone, FindStarsFirst);
    FindStarsFirst->      addTransition(this, &Guider::FindStarsDone, ComputeFirst);
    ComputeFirst->        addTransition(this, &Guider::ComputeFirstDone, End);

    Init->setInitialState(InitInit);

    _SMInit.addState(Init);
    _SMInit.addState(Abort);
    _SMInit.addState(End);
    _SMInit.setInitialState(Init);


}
void Guider::buildCalStateMachines(void)
{

    auto *Abort = new QState();
    auto *Cal  = new QState();
    auto *End   = new QFinalState();

    auto *InitCal             = new QState(Cal);
    auto *RequestCalPulses    = new QState(Cal);
    auto *WaitCalPulses       = new QState(Cal);
    auto *RequestCalExposure  = new QState(Cal);
    auto *WaitCalExposure     = new QState(Cal);
    auto *FindStarsCal        = new QState(Cal);
    auto *ComputeCal          = new QState(Cal);

    connect(InitCal, &QState::entered, this, &Guider::SMInitCal);
    connect(RequestCalExposure, &QState::entered, this, &Guider::SMRequestExposure);
    connect(FindStarsCal, &QState::entered, this, &Guider::SMFindStars);
    connect(ComputeCal, &QState::entered, this, &Guider::SMComputeCal);
    connect(RequestCalPulses,    &QState::entered, this, &Guider::SMRequestPulses);
    connect(Abort,               &QState::entered, this, &Guider::SMAbort);

    Cal->                 addTransition(this, &Guider::Abort, Abort);
    Abort->               addTransition(this, &Guider::AbortDone, End);
    InitCal->             addTransition(this, &Guider::InitCalDone, RequestCalPulses);

    RequestCalPulses->    addTransition(this, &Guider::RequestPulsesDone, WaitCalPulses);
    WaitCalPulses->       addTransition(this, &Guider::PulsesDone, RequestCalExposure);
    RequestCalExposure->  addTransition(this, &Guider::RequestExposureDone, WaitCalExposure);
    WaitCalExposure->     addTransition(this, &Guider::ExposureDone, FindStarsCal);
    FindStarsCal->        addTransition(this, &Guider::FindStarsDone, ComputeCal);
    ComputeCal->          addTransition(this, &Guider::ComputeCalDone, RequestCalPulses);
    ComputeCal->          addTransition(this, &Guider::CalibrationDone, End);


    Cal->setInitialState(InitCal);

    _SMCalibration.addState(Cal);
    _SMCalibration.addState(Abort);
    _SMCalibration.addState(End);
    _SMCalibration.setInitialState(Cal);


}
void Guider::buildGuideStateMachines(void)
{

    auto *Abort = new QState();
    auto *Guide  = new QState();
    auto *End   = new QFinalState();

    auto *InitGuide              = new QState(Guide);
    auto *RequestGuideExposure   = new QState(Guide);
    auto *WaitGuideExposure      = new QState(Guide);
    auto *FindStarsGuide         = new QState(Guide);
    auto *ComputeGuide           = new QState(Guide);
    auto *RequestGuidePulses     = new QState(Guide);
    auto *WaitGuidePulses        = new QState(Guide);
    // Dither branch: send dither pulses then rebuild reference before resuming
    auto *RequestDitherPulses    = new QState(Guide);
    auto *WaitDitherPulses       = new QState(Guide);
    auto *RequestRefExposure     = new QState(Guide);
    auto *WaitRefExposure        = new QState(Guide);
    auto *FindStarsRef           = new QState(Guide);
    auto *ComputeNewRef          = new QState(Guide);

    connect(InitGuide,            &QState::entered, this, &Guider::SMInitGuide);
    connect(RequestGuideExposure, &QState::entered, this, &Guider::SMRequestExposure);
    connect(FindStarsGuide,       &QState::entered, this, &Guider::SMFindStars);
    connect(ComputeGuide,         &QState::entered, this, &Guider::SMComputeGuide);
    connect(RequestGuidePulses,   &QState::entered, this, &Guider::SMRequestPulses);
    connect(RequestDitherPulses,  &QState::entered, this, &Guider::SMRequestPulses);
    connect(RequestRefExposure,   &QState::entered, this, &Guider::SMRequestExposure);
    connect(FindStarsRef,         &QState::entered, this, &Guider::SMFindStars);
    connect(ComputeNewRef,        &QState::entered, this, &Guider::SMComputeFirst);
    connect(Abort,                &QState::entered, this, &Guider::SMAbort);

    Guide->                  addTransition(this, &Guider::Abort,              Abort);
    Abort->                  addTransition(this, &Guider::AbortDone,          End);
    InitGuide->              addTransition(this, &Guider::InitGuideDone,      RequestGuideExposure);

    // Normal guiding loop
    RequestGuideExposure->   addTransition(this, &Guider::RequestExposureDone, WaitGuideExposure);
    WaitGuideExposure->      addTransition(this, &Guider::ExposureDone,        FindStarsGuide);
    FindStarsGuide->         addTransition(this, &Guider::FindStarsDone,       ComputeGuide);
    ComputeGuide->           addTransition(this, &Guider::ComputeGuideDone,    RequestGuidePulses);
    RequestGuidePulses->     addTransition(this, &Guider::RequestPulsesDone,   WaitGuidePulses);
    RequestGuidePulses->     addTransition(this, &Guider::PulsesDone,          RequestGuideExposure);
    WaitGuidePulses->        addTransition(this, &Guider::PulsesDone,          RequestGuideExposure);

    // Dither branch: triggered by DitherNow from ComputeGuide
    ComputeGuide->           addTransition(this, &Guider::DitherNow,           RequestDitherPulses);
    RequestDitherPulses->    addTransition(this, &Guider::RequestPulsesDone,   WaitDitherPulses);
    RequestDitherPulses->    addTransition(this, &Guider::PulsesDone,          RequestRefExposure);
    WaitDitherPulses->       addTransition(this, &Guider::PulsesDone,          RequestRefExposure);
    RequestRefExposure->     addTransition(this, &Guider::RequestExposureDone, WaitRefExposure);
    WaitRefExposure->        addTransition(this, &Guider::ExposureDone,        FindStarsRef);
    FindStarsRef->           addTransition(this, &Guider::FindStarsDone,       ComputeNewRef);
    ComputeNewRef->          addTransition(this, &Guider::ComputeFirstDone,    InitGuide);

    Guide->setInitialState(InitGuide);

    _SMGuide.addState(Guide);
    _SMGuide.addState(Abort);
    _SMGuide.addState(End);
    _SMGuide.setInitialState(Guide);


}
/**
 * @brief PHASE 1: Initialization - Connect to devices and capture reference stars
 *
 * Steps:
 *  1. Connect to camera (CCD) and guider mount device via INDI
 *  2. Configure camera (BLOB mode, direct access, frame reset)
 *  3. Query mount position (RA, DEC) for compensation calculations
 *  4. Query pier side (West/East) for CCD orientation determination
 *  5. Clear history grids (drift and guiding)
 *
 * After this handler completes, SMInit transitions to SMRequestFrameReset
 * which will trigger a CCD exposure to capture the reference star field.
 */
void Guider::SMInitInit()
{
    setStateEvent(OST::Busy, "init", "intialisation", "Guider initialize");
    // Connect camera device (required)
    if (connectDevice(getString("devices", "camera")))
    {
        // Initialize INDI connection
        connectIndi();

        // Connect both required devices
        connectDevice(getString("devices", "camera"));        // CCD camera
        connectDevice(getString("devices", "guider"));         // Guider mount interface

        // Configure CCD for image streaming
        setBLOBMode(B_ALSO, getString("devices", "camera").toStdString().c_str(), nullptr);  // Receive BLOBs
        enableDirectBlobAccess(getString("devices", "camera").toStdString().c_str(), nullptr); // Direct access
        frameReset(getString("devices", "camera"));  // Reset frame to clear any previous capture
        setFocalLengthAndDiameter(); // Mandatory for simulators to work


        // For simulator, speed up exposure time
        if (getString("devices", "camera") == "CCD Simulator")
        {
            //sendModNewNumber(getString("devices", "camera"), "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 0.01);
        }

        // Set UI state to "busy"
        getProperty("actions")->setState(OST::Busy, true);

        // Clear history from previous sessions
        getProperty("drift")->clearGrid();
        getProperty("guiding")->clearGrid();
    }
    else
    {
        // Failed to connect camera
        getProperty("actions")->setState(OST::Error, true);
        setStateEvent(OST::Error, "error", "devicefailed", "camera failed");
        logError("Failed to connect camera device");
        emit Abort();
        return;
    }

    // === Query Mount Position (for DEC compensation) ===

    // Get current DEC (critical for RA pulse compensation)
    // At DEC=90° (north pole), RA pulses are ineffective (cos(90°)=0)
    // At DEC=0° (equator), RA pulses are full strength (cos(0°)=1)
    if (!getModNumber(getString("devices", "guider"), "EQUATORIAL_EOD_COORD", "DEC", _mountDEC))
    {
        logError("Failed to read mount DEC position");
        setStateEvent(OST::Error, "error", "devicefailed", "mount failed");
        emit Abort();
        return;
    }

    // Get current RA (stored but not currently used in compensation)
    if (!getModNumber(getString("devices", "guider"), "EQUATORIAL_EOD_COORD", "RA", _mountRA))
    {
        logError("Failed to read mount RA position");
        setStateEvent(OST::Error, "error", "devicefailed", "mount failed");
        emit Abort();
        return;
    }

    // Get pier side (West or East of pier). Optional: only the
    // enablepiersidereverse feature uses it (off by default), and fork / alt-az
    // mounts don't expose TELESCOPE_PIER_SIDE. A read failure is not fatal -
    // keep the previous value and carry on (do NOT hang init here).
    if (!getModSwitch(getString("devices", "guider"), "TELESCOPE_PIER_SIDE", "PIER_WEST", _mountPointingWest))
    {
        logWarning("Could not read mount pier side - pier-side reversal disabled for this session");
    }

    logInfo(QString("Mount position: RA=%1, DEC=%2, Pier=%3")
            .arg(_mountRA, 0, 'f', 1)
            .arg(_mountDEC, 0, 'f', 1)
            .arg(_mountPointingWest ? "West" : "East"));

    // Initialization complete - transition to exposure
    setStateEvent(OST::Busy, "initdone", "intialized", "Guider initialized");
    emit InitDone();
}
/**
 * @brief PHASE 2: Calibration initialization - Prepare for pulse measurement
 *
 * Calibration workflow:
 *  - Send test pulse N (1000ms) → measure drift → store N offset
 *  - Send test pulse S (1000ms) → measure drift → store S offset
 *  - Send test pulse E (1000ms) → measure drift → store E offset
 *  - Send test pulse W (1000ms) → measure drift → store W offset
 *  - Repeat for 2 iterations (parameter: calsteps)
 *
 * Results stored in calibrationvalues:
 *  calPulseN, calPulseS, calPulseE, calPulseW = pixels moved per ms of pulse
 *  These are used to convert pixel drift → pulse duration in guiding phase
 *
 * This handler initializes counters and seeds the per-pulse reference
 * (_prevStars) from the initialization phase's star field.
 */
void Guider::SMInitCal()
{
    setStateEvent(OST::Busy, "initcal", "initcal", "init calibration start");
    logInfo("Initializing calibration sequence");
    getEltBool("actions", "calibrate")->setValue(false, false);
    getEltBool("actions", "abortguider")->setValue(false, false);
    getEltBool("actions", "guide")->setValue(false, false);
    getEltBool("actions", "resetcalibration")->setValue(false, false);
    getEltBool("actions", "calibrate")->setValue(true, true);

    // Reset calibration loop counters
    _calState = 0;   // Calibration phase (0-2)
    _calStep = 0;    // Pulse direction counter (0-7 for 4 directions × 2 iterations)
    _calAwaitingKick = false;
    _calRetry = 0;
    getEltLight("calibrationvalues", "calqual")->setValue(OST::Idle, true);

    // Clear previous calibration results
    _calPulseRA = 0;
    _calPulseDEC = 0;
    _calPulseN = 0;  // filled by calibration
    _calPulseS = 0;
    _calPulseE = 0;
    _calPulseW = 0;
    for (int i = 0; i < 4; i++) _calPassMean[i][0] = _calPassMean[i][1] = 0;

    // Update UI with current (empty) calibration values
    getEltFloat("calibrationvalues", "calPulseRA")->setValue(_calPulseRA);
    getEltFloat("calibrationvalues", "calPulseDE")->setValue(_calPulseDEC);
    getEltFloat("calibrationvalues", "calPulseN")->setValue(_calPulseN);
    getEltFloat("calibrationvalues", "calPulseS")->setValue(_calPulseS);
    getEltFloat("calibrationvalues", "calPulseE")->setValue(_calPulseE);
    getEltFloat("calibrationvalues", "calPulseW")->setValue(_calPulseW, true);

    // Store current pier side at calibration time (for future pier-side compensation)
    // True = West, False = East
    _calMountPointingWest = _mountPointingWest;
    getEltBool("calibrationvalues", "calPier")->setValue(_calMountPointingWest, true);

    // Reset pulse tracking (used in SMRequestPulses)
    _pulseN = 0;
    _pulseS = 0;
    _pulseE = 0;
    _pulseW = getInt("calParams", "pulse");  // Load calibration pulse duration (default 1000ms)

    // The per-pulse increment is measured against the previous calibration
    // frame; seed it with the initialization reference.
    _prevStars = starsFirst;
    _dxPrev = 0;
    _dyPrev = 0;
    _dxFirst = 0;
    _dyFirst = 0;

    // Clear per-axis drift accumulators used for the pulse-rate calculation
    _dxvector.clear();
    _dyvector.clear();

    // Reset flags
    _itt = 0;
    _pulseDECfinished = true;   // Mark pulses as done (ready for next)
    _pulseRAfinished = true;

    logInfo("Calibration ready - sending test pulses");
    emit InitCalDone();
    setStateEvent(OST::Busy, "initcal", "initcaldone", "init calibration done");

}
/**
 * @brief PHASE 3: Guiding initialization - Load calibration and setup DEC compensation
 *
 * CRITICAL COMPENSATION: RA pulses must be scaled by cos(mount_DEC)
 *
 * Why?
 * - At equator (DEC=0°): RA and DEC axes are orthogonal, pulses work 1:1
 * - At high latitude (DEC=60°): RA lines converge, need cos(60°)=0.5 scaling
 * - At pole (DEC=90°): RA lines are parallel, RA pulses have NO effect (cos(90°)=0)
 *
 * Example: Telescope at DEC=+45° (mid-north)
 *  - Calibration measured: East pulse = 100 px/sec
 *  - Current DEC = +45°
 *  - cos(45°) = 0.707
 *  - Compensated East = 100 × 0.707 = 70.7 px/sec
 *  - If measured drift = 1 pixel RA, send pulse = 1 / 70.7 = 14.1 ms (not 10 ms)
 *
 * This compensation is essential for multi-latitude observation sites!
 */
void Guider::SMInitGuide()
{
    setStateEvent(OST::Busy, "guiding", "startguiding", "startguiding");
    getEltBool("actions", "calibrate")->setValue(false, false);
    getEltBool("actions", "abortguider")->setValue(false, false);
    getEltBool("actions", "guide")->setValue(false, false);
    getEltBool("actions", "resetcalibration")->setValue(false, false);
    getEltBool("actions", "guide")->setValue(true, true);

    // Clear drift history from calibration phase
    getProperty("drift")->clearGrid();
    getProperty("guiding")->clearGrid();

    // Reset DEC backlash compensation state for this new guiding session
    _lastDecDir          = 0;
    _decBacklashLearning = false;
    _consecutiveMatchFail = 0;
    // starsFirst is the fresh reference (just set by calibration or SMComputeFirst);
    // drift vs it starts at zero.
    _dxFirst = 0;
    _dyFirst = 0;
    _intRA = 0;
    _intDE = 0;
    _intRAsat = false;
    _intDEsat = false;

    // Set grid limits to match RMS buffer size for consistent visualization
    int rmsOver = getInt("guideParams", "rmsOver");
    getProperty("drift")->setGridLimit(rmsOver);
    getProperty("guiding")->setGridLimit(rmsOver);
    logInfo("Grid limits set to %1 frames (rmsOver parameter)", {QString::number(rmsOver)});

    // Load calibration results from database
    _calPulseRA  = getFloat("calibrationvalues", "calPulseRA");
    _calPulseDEC = getFloat("calibrationvalues", "calPulseDE");
    _calCcdOrientation = getFloat("calibrationvalues", "ccdOrientation") * PI / 180.0;  // Convert degrees to radians
    _calMountDEC = getFloat("calibrationvalues", "calMountDEC");  // DEC at calibration time

    // Load and apply stored correction reversals from calibration
    bool storedRevRA = getBool("calibrationvalues", "revRA");
    bool storedRevDE = getBool("calibrationvalues", "revDE");
    getEltBool("revCorrections", "revRA")->setValue(storedRevRA);
    getEltBool("revCorrections", "revDE")->setValue(storedRevDE, true);
    logInfo("Applied stored corrections from calibration: revRA=%1, revDE=%2", {QString(storedRevRA ? "true" : "false"),
            QString(storedRevDE ? "true" : "false")
                                                                               });

    // Get CURRENT mount DEC (may differ from calibration time!)
    // This is needed because target position may have changed since calibration
    if (!getModNumber(getString("devices", "guider"), "EQUATORIAL_EOD_COORD", "DEC", _mountDEC))
    {
        logWarning("Could not read mount DEC, assuming DEC=0 (use with caution at high latitudes!)");
        _mountDEC = 0;  // Fallback - assumes equator
    }

    // Show calibration reference information
    logInfo("Calibration performed at DEC: %1°", {QString::number(_calMountDEC, 'f', 1)});
    logInfo("Current target DEC: %1°", {QString::number(_mountDEC, 'f', 1)});

    // === PIER SIDE COMPENSATION CHECK ===

    // Re-read current pier side from mount (may have flipped since initialization)
    bool currentPierWest = _mountPointingWest;
    if (!getModSwitch(getString("devices", "guider"), "TELESCOPE_PIER_SIDE", "PIER_WEST", currentPierWest))
    {
        logWarning("Could not read current pier side position");
    }

    // Check if pier side has changed since calibration
    bool enablePierSideReverse = getBool("guideParams", "enablepiersidereverse");
    bool calibrationPierWest = getBool("calibrationvalues", "calPier");

    if (enablePierSideReverse)
    {
        if (currentPierWest != calibrationPierWest)
        {
            // Pier side has flipped - invert RA/DEC corrections
            logInfo("Pier side calibration differs from actual position, reversing RA/DEC");

            // Read current correction states
            bool revRA = getBool("revCorrections", "revRA");
            bool revDE = getBool("revCorrections", "revDE");

            // Invert them
            revRA = !revRA;
            revDE = !revDE;

            // Write inverted values back
            getEltBool("revCorrections", "revRA")->setValue(revRA);
            getEltBool("revCorrections", "revDE")->setValue(revDE, true);

            logInfo("RA reverse: %1, DEC reverse: %2", {QString(revRA ? "true" : "false"), QString(revDE ? "true" : "false")});
        }
        else
        {
            // Pier side matches calibration - leave corrections as they are
            // (no action needed)
        }
    }

    // === DEC COMPENSATION (most critical code!) ===

    logInfo("Starting guiding session");
    logInfo("Current DEC: %1", {QString::number(_mountDEC, 'f', 1) + "°"});

    // Calculate compensation factor: cos(DEC in radians)
    // This scales RA pulses for current latitude
    double currentDecCompensation = cos(_mountDEC * PI / 180.0);

    logInfo("RA compensation factor: %1 (cos(%2 deg))", {QString::number(currentDecCompensation, 'f', 3), QString::number(_mountDEC, 'f', 1)});
    logInfo("Guide rates: RA=%1 ms/px (compensated %2), DEC=%3 ms/px",
    {
        QString::number(_calPulseRA, 'f', 1),
        QString::number(_calPulseRA * currentDecCompensation, 'f', 1),
        QString::number(_calPulseDEC, 'f', 1)
    });

    // Clear RMS drift history from previous guiding sessions
    // These will accumulate as new measurements come in
    _dRAvector.clear();
    _dDEvector.clear();

    _doDither = false;

    // TEST AID (guideParams/simflip): the INDI CCD Simulator doesn't rotate its
    // field with TELESCOPE_PIER_SIDE, so meridian-flip handling can't be tested
    // end to end. A meridian flip's effect on the guider is that the drift ->
    // RA/DEC mapping rotates 180 deg (adding PI to the calibration angle, which
    // is exactly revRA = revDE = -1). Simulate that here: with simflip set,
    // guiding should run away UNLESS both reverse flags are on - that verifies
    // the "flip both rev bits" handling.
    if (getBool("guideParams", "simflip"))
    {
        _calCcdOrientation += PI;
        logWarning("SIM: calibration angle rotated 180 deg (meridian-flip test aid) - expect to need revRA+revDE");
    }

    logInfo("Guide start: ccdOrientation=%1 deg  revRA=%2 revDE=%3",
    {
        QString::number(_calCcdOrientation * 180.0 / PI, 'f', 1),
        QString(getBool("revCorrections", "revRA") ? "on" : "off"),
        QString(getBool("revCorrections", "revDE") ? "on" : "off")
    });

    emit InitGuideDone();
    setStateEvent(OST::Busy, "initguidedone", "initguidedone", "init guide done");

}
void Guider::SMRequestFrameReset()
{
    //logInfo("SMRequestFrameReset");
    if (!frameReset(getString("devices", "camera")))
    {
        setStateEvent(OST::Error, "error", "devicefailed", "camera failed");
        emit Abort();
        return;
    }
    armWatchdog();   // waiting for CCD_FRAME_RESET -> IPS_OK
    emit RequestFrameResetDone();
}


void Guider::SMRequestExposure()
{
    //logInfo("SMRequestExposure");
    if (!requestCapture(getString("devices", "camera"), getFloat("parms", "exposure"), getInt("parms", "gain"), getInt("parms",
                        "offset")))
    {
        setStateEvent(OST::Error, "error", "devicefailed", "camera failed");
        emit Abort();
        return;
    }
    _expectingFrame = true;
    armWatchdog();   // waiting for the exposure BLOB
    emit RequestExposureDone();
}
startracker::Params Guider::trackParams()
{
    startracker::Params p;   // startracker's own defaults if a param is missing
    if (getEltInt("guideParams", "matchmaxstars"))   p.maxStars   = getInt("guideParams", "matchmaxstars");
    if (getEltInt("guideParams", "matchmininliers")) p.minInliers = getInt("guideParams", "matchmininliers");
    if (getEltInt("guideParams", "matchgate"))       p.coarseGate = getInt("guideParams", "matchgate");
    p.fineGate = qBound(0.4, p.coarseGate * 0.3, 2.0);
    return p;
}

void Guider::SMComputeFirst()
{
    // Snapshot the current star field as the fixed reference. The star-tracker
    // works directly on star positions, no pre-indexing needed.
    starsFirst = _solver.stars;

    emit ComputeFirstDone();
}
void Guider::SMComputeCal()
{
    //qDebug()  << "SMComputeCal" << _calStep << _calState;
    _ccdOrientation = 0;

    const QVector<startracker::Star> cur = toTrackStars(_solver.stars);
    const startracker::Params tp = trackParams();

    // Per-pulse increment: current vs previous calibration frame (drives the
    // pulse-rate calculation).
    startracker::MatchResult inc = startracker::match(toTrackStars(_prevStars), cur,
                                                     _dxPrev, _dyPrev, tp);
    // Total drift vs the initialization reference (display only).
    startracker::MatchResult tot = startracker::match(toTrackStars(starsFirst), cur,
                                                     0.0, 0.0, tp);

    if (!inc.ok)
    {
        logError("Calibration: lost star correlation (ref=%1 cur=%2 matched=%3, need %4) - abort",
        {
            QString::number(inc.nRef), QString::number(inc.nCur),
            QString::number(inc.nInliers), QString::number(tp.minInliers)
        });
        setStateEvent(OST::Error, "error", "nostars", "no correlation");
        emit Abort();
        return;
    }
    logInfo("Calibration match: d=(%1,%2)px  matched %3/%4  rms=%5px",
    {
        QString::number(inc.dx, 'f', 2), QString::number(inc.dy, 'f', 2),
        QString::number(inc.nInliers), QString::number(inc.nCur),
        QString::number(inc.rms, 'f', 2)
    });
    _dxPrev = inc.dx;
    _dyPrev = inc.dy;
    if (tot.ok)
    {
        _dxFirst = tot.dx;
        _dyFirst = tot.dy;
    }

    // The DEC backlash kick's own resulting movement isn't a clean measurement
    // (that's the point - it's meant to be absorbed by mechanical backlash),
    // so it's excluded from the rate calculation below.
    if (!_calAwaitingKick)
    {
        _dxvector.push_back(_dxPrev);
        _dyvector.push_back(_dyPrev);
    }

    _prevStars = _solver.stars;

    /*if (_calState==0) {
        BOOST_LOG_TRIVIAL(debug) << "RA drift " << sqrt(square(_avdx)+square(_avdy)) << " drift / ms = " << 1000*sqrt(square(_avdx)+square(_avdy))/_pulseWTot;
    }
    if (_calState==2) {
        BOOST_LOG_TRIVIAL(debug) << "DEC drift " << sqrt(square(_avdx)+square(_avdy)) << " drift / ms = " << 1000*sqrt(square(_avdx)+square(_avdy))/_pulseNTot;
    }*/
    bool wasKick = _calAwaitingKick;
    _calAwaitingKick = false;

    _pulseN = 0;
    _pulseS = 0;
    _pulseE = 0;
    _pulseW = 0;

    if (!wasKick)
    {
    _calStep++;

    // Send progress messages during calibration
    QString directionName;
    if (_calState == 0) directionName = "West";
    else if (_calState == 1) directionName = "East";
    else if (_calState == 2) directionName = "North";
    else if (_calState == 3) directionName = "South";

    logInfo("Calibration %1 - step %2/%3", {directionName, QString::number(_calStep), QString::number(getInt("calParams", "calsteps"))});

    if (_calStep >= getInt("calParams", "calsteps") )
    {
        double ddx = 0;
        double ddy = 0;
        for (unsigned int i = 0; i < _dxvector.size(); i++)
        {
            ddx = ddx + _dxvector[i];
            ddy = ddy + _dyvector[i];
        }
        ddx = ddx / (_dxvector.size());
        ddy = ddy / (_dyvector.size());

        // Keep each pass' mean (ref-cur) increment; the axis rates and the CCD
        // orientation are derived below from the antisymmetric combinations
        // West-East and North-South, which cancel the sidereal / periodic-error
        // drift that otherwise biases every one-directional pass.
        _calPassMean[_calState][0] = ddx;
        _calPassMean[_calState][1] = ddy;

        double ech = getSampling();
        double rawMag = sqrt(square(ddx) + square(ddy));
        double rawRate = (rawMag > 1e-6) ? getInt("calParams", "pulse") / rawMag : 0;
        const char *dir = (_calState == 0) ? "West" : (_calState == 1) ? "East"
                          : (_calState == 2) ? "North" : "South";
        const char *evk = (_calState == 0) ? "westcomplete" : (_calState == 1) ? "estcomplete"
                          : (_calState == 2) ? "northcomplete" : "southcomplete";
        if (_calState == 0) { _calPulseW = rawRate; _calMountPointingWest = _mountPointingWest; }
        else if (_calState == 1) _calPulseE = rawRate;
        else if (_calState == 2) _calPulseN = rawRate;
        else                     _calPulseS = rawRate;
        logInfo("%1 pass: increment (ref-cur)=(%2,%3)px  raw %4 ms/px (drift=%5\")",
        {
            QString(dir), QString::number(ddx, 'f', 2), QString::number(ddy, 'f', 2),
            QString::number(rawRate, 'f', 1), QString::number(rawMag * ech, 'f', 2)
        });
        setStateEvent(OST::Busy, "cal", evk, evk);

        _calStep = 0;
        _calState++;
        _dxvector.clear();
        _dyvector.clear();

        if (_calState >= 4)
        {
            // --- combine the 4 passes -------------------------------------------
            double dRAx = (_calPassMean[0][0] - _calPassMean[1][0]) / 2.0;
            double dRAy = (_calPassMean[0][1] - _calPassMean[1][1]) / 2.0;
            double dDEx = (_calPassMean[2][0] - _calPassMean[3][0]) / 2.0;
            double dDEy = (_calPassMean[2][1] - _calPassMean[3][1]) / 2.0;
            double bgx  = (_calPassMean[0][0] + _calPassMean[1][0]) / 2.0;  // background drift estimate
            double bgy  = (_calPassMean[0][1] + _calPassMean[1][1]) / 2.0;

            double magRA = sqrt(square(dRAx) + square(dRAy));
            double magDE = sqrt(square(dDEx) + square(dDEy));
            _calPulseRA  = (magRA > 1e-6) ? getInt("calParams", "pulse") / magRA : 0;
            _calPulseDEC = (magDE > 1e-6) ? getInt("calParams", "pulse") / magDE : 0;

            // CCD orientation from BOTH axes (Ekos-style two-estimate average).
            double angleRA = atan2(dRAy, dRAx);
            double angleDE = atan2(-dDEx, dDEy);   // DEC axis expressed in the RA-axis convention
            if (fabs(atan2(sin(angleDE - angleRA), cos(angleDE - angleRA))) > PI / 2.0)
                angleDE += PI;                     // resolve the 180-deg ambiguity against the RA axis
            _calCcdOrientation = atan2(sin(angleRA) + sin(angleDE), cos(angleRA) + cos(angleDE));
            _ccdOrientation = _calCcdOrientation;

            // Calibration quality: how much of each per-pulse increment was the
            // constant background drift (tracking / polar-alignment error) rather
            // than the pulse itself. High ratio -> the differenced rates rest on
            // a shaky assumption and the mount's polar alignment is likely poor.
            double bgMag = sqrt(square(bgx) + square(bgy));
            double sigMin = qMin(magRA, magDE);
            double driftRatio = (sigMin > 1e-6) ? bgMag / sigMin : 999.0;
            double maxRatio = getFloat("calParams", "calmaxdriftratio");

            logInfo("Calibration: RA %1 ms/px, DEC %2 ms/px, angle %3 deg (RA %4 / DE %5), background drift (%6,%7) px/frame, drift ratio %8",
            {
                QString::number(_calPulseRA, 'f', 1), QString::number(_calPulseDEC, 'f', 1),
                QString::number(_calCcdOrientation * 180.0 / PI, 'f', 1),
                QString::number(angleRA * 180.0 / PI, 'f', 1), QString::number(angleDE * 180.0 / PI, 'f', 1),
                QString::number(bgx, 'f', 2), QString::number(bgy, 'f', 2),
                QString::number(driftRatio, 'f', 2)
            });

            OST::State qual = (driftRatio < 0.30) ? OST::Ok
                              : (maxRatio <= 0 || driftRatio < maxRatio) ? OST::Busy : OST::Error;
            getEltLight("calibrationvalues", "calqual")->setValue(qual, true);

            if (maxRatio > 0 && driftRatio > maxRatio)
            {
                _calRetry++;
                int maxRetry = getInt("calParams", "calmaxretries");
                logWarning("Calibration quality poor: background drift is %1x the pulse signal (limit %2) - check the mount's polar alignment",
                {QString::number(driftRatio, 'f', 2), QString::number(maxRatio, 'f', 2)});

                if (_calRetry <= maxRetry)
                {
                    logWarning("Retrying calibration (%1/%2)", {QString::number(_calRetry), QString::number(maxRetry)});
                    _calState = 0;
                    _calStep = 0;
                    _calAwaitingKick = false;
                    _dxvector.clear();
                    _dyvector.clear();
                    for (int i = 0; i < 4; i++) _calPassMean[i][0] = _calPassMean[i][1] = 0;
                    _dxPrev = 0;
                    _dyPrev = 0;
                    _pulseW = getInt("calParams", "pulse");   // restart the West pass
                    setStateEvent(OST::Busy, "cal", "calretry", "calibration retry");
                    emit ComputeCalDone();                     // -> RequestCalPulses
                    return;
                }

                logError("Calibration failed: still poor after %1 retries - Abort. Fix the mount's polar alignment.",
                {QString::number(maxRetry)});
                setStateEvent(OST::Error, "error", "calbad", "calibration quality too poor");
                emit Abort();
                return;
            }
            _calRetry = 0;

            _calMountDEC = _mountDEC;
            double decCompensation = cos(_calMountDEC * PI / 180.0);
            if (decCompensation > 0.1)  // avoid blow-up near the pole
            {
                _calPulseRA = _calPulseRA / decCompensation;
                logInfo("DEC compensation applied: DEC=%1 deg factor=%2",
                {QString::number(_calMountDEC, 'f', 1), QString::number(decCompensation, 'f', 3)});
            }

            // Persist: RA/DE are the rates used for guiding; N/S/E/W kept as raw
            // per-direction diagnostics (they still carry the background drift).
            getEltFloat("calibrationvalues", "calPulseRA")->setValue(_calPulseRA);
            getEltFloat("calibrationvalues", "calPulseDE")->setValue(_calPulseDEC);
            getEltFloat("calibrationvalues", "calPulseN")->setValue(_calPulseN);
            getEltFloat("calibrationvalues", "calPulseS")->setValue(_calPulseS);
            getEltFloat("calibrationvalues", "calPulseE")->setValue(_calPulseE);
            getEltFloat("calibrationvalues", "calPulseW")->setValue(_calPulseW);
            getEltFloat("calibrationvalues", "ccdOrientation")->setValue(_calCcdOrientation * 180 / PI);
            getEltFloat("calibrationvalues", "calMountDEC")->setValue(_calMountDEC);
            getEltBool("calibrationvalues", "revRA")->setValue(getBool("revCorrections", "revRA"));
            getEltBool("calibrationvalues", "revDE")->setValue(getBool("revCorrections", "revDE"), true);
            logInfo("Calibration completed successfully");
            getProperty("actions")->setState(OST::Ok, true);
            emit CalibrationDone();
            setStateEvent(OST::Busy, "caldone", "calcompleted", "calibration completed");
            // The post-calibration position becomes the guiding reference.
            starsFirst = _solver.stars;
            return;
        }

        // Entering the North or South pass: take up mechanical backlash first,
        // so the timed measurement pulses that follow aren't partially wasted
        // engaging the gears.
        if (_calState == 2 || _calState == 3)
        {
            int kick = getInt("calParams", "decbacklashkick");
            if (kick > 0)
            {
                if (_calState == 2) _pulseN = kick;
                else                _pulseS = kick;
                _calAwaitingKick = true;
                logInfo("Sending %1ms DEC backlash kick before the %2 calibration pass",
                {QString::number(kick), _calState == 2 ? "North" : "South"});
            }
        }
    }
    }

    if (!_calAwaitingKick)
    {
    if (_calState == 0)
    {
        _pulseW = getInt("calParams", "pulse");
    }
    if (_calState == 1)
    {
        _pulseE = getInt("calParams", "pulse");
    }
    if (_calState == 2)
    {
        _pulseN = getInt("calParams", "pulse");
    }
    if (_calState == 3)
    {
        _pulseS = getInt("calParams", "pulse");
    }
    }
    double _driftRA = _dxFirst * cos(_calCcdOrientation) + _dyFirst *  sin(_calCcdOrientation);
    double _driftDE = _dxFirst * sin(_calCcdOrientation) + _dyFirst * -cos(_calCcdOrientation);
    double ech = getSampling();
    getEltFloat("drift", "RA")->setValue(_driftRA * ech);
    getEltFloat("drift", "DEC")->setValue(_driftDE * ech);
    getProperty("drift")->push();


    emit ComputeCalDone();
}
void Guider::SMComputeGuide()
{
    _pulseW = 0;
    _pulseE = 0;
    _pulseN = 0;
    _pulseS = 0;

    // Measure drift vs the fixed reference, seeded with the previous frame's
    // drift for stability. A transient loss of correlation just skips the frame
    // (no pulse); only a run of consecutive failures aborts.
    startracker::MatchResult m = startracker::match(toTrackStars(starsFirst),
                                                   toTrackStars(_solver.stars),
                                                   _dxFirst, _dyFirst, trackParams());
    if (!m.ok)
    {
        _consecutiveMatchFail++;
        int maxFail = getInt("guideParams", "maxmatchfail");
        logWarning("No star correlation with reference (%1/%2) - skipping frame [ref=%3 cur=%4 matched=%5]",
        {
            QString::number(_consecutiveMatchFail), QString::number(maxFail),
            QString::number(m.nRef), QString::number(m.nCur), QString::number(m.nInliers)
        });
        if (_consecutiveMatchFail >= maxFail)
        {
            logError("Lost star correlation for %1 consecutive frames - Abort", {QString::number(_consecutiveMatchFail)});
            emit Abort();
            return;
        }
        emit ComputeGuideDone();   // pulses are all 0 -> this frame sends nothing
        return;
    }
    _consecutiveMatchFail = 0;
    _dxFirst = m.dx;
    _dyFirst = m.dy;
    // Dither requested: compute random displacement pulses then rebuild reference
    if (_doDither)
    {
        double currentDecCompensation = cos(_mountDEC * PI / 180.0);
        int ditherpixel = getInt("guideParams", "ditherpixel");
        double randRA  = (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * ditherpixel;
        double randDEC = (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * ditherpixel;

        // RA: calPulseRA is DEC-normalised, apply current DEC compensation
        if (randRA > 0)
            _pulseW = randRA * _calPulseRA * currentDecCompensation;
        else
            _pulseE = -randRA * _calPulseRA * currentDecCompensation;

        // DEC: calPulseDEC is raw ms/pixel, no compensation needed
        if (randDEC > 0)
            _pulseN = randDEC * _calPulseDEC;
        else
            _pulseS = -randDEC * _calPulseDEC;

        _doDither = false;
        logInfo(QString("Dithering: RA=%1 px DEC=%2 px -> pulseN=%3 pulseS=%4 pulseE=%5 pulseW=%6 ms")
                .arg(randRA, 0, 'f', 1).arg(randDEC, 0, 'f', 1)
                .arg(_pulseN).arg(_pulseS).arg(_pulseE).arg(_pulseW));
        emit DitherNow();
        setStateEvent(OST::Busy, "dithering", "ditherrequest", "dither requested");
        return;
    }

    double _driftRA = _dxFirst * cos(_calCcdOrientation) + _dyFirst *  sin(_calCcdOrientation);
    double _driftDE = _dxFirst * sin(_calCcdOrientation) + _dyFirst * -cos(_calCcdOrientation);

    // Integral term: a proportional-only law cannot null a constant drift
    // (tracking / polar-alignment error) - it settles at error = drift / gain.
    // Accumulate the raw residual and add Ki * accumulator to the correction;
    // a true integrator drives the steady-state residual to zero. The
    // accumulator is clamped (anti-windup) and frozen for an axis whose pulse
    // saturated last frame; it is reset at guide start / after a dither.
    double intMax = getFloat("guideParams", "intmax");
    if (!_intRAsat) _intRA += _driftRA;
    if (!_intDEsat) _intDE += _driftDE;
    if (intMax > 0)
    {
        _intRA = qBound(-intMax, _intRA, intMax);
        _intDE = qBound(-intMax, _intDE, intMax);
    }
    double kiRA = getFloat("guideParams", "raintgain");
    double kiDE = getFloat("guideParams", "deintgain");
    double driftRAeff = _driftRA + kiRA * _intRA;
    double driftDEeff = _driftDE + kiDE * _intDE;

    logInfo("Guide: dxy(ref-cur)=(%1,%2)px  drift RA/DE=(%3,%4)px  I=(%5,%6)  matched=%7",
    {
        QString::number(_dxFirst, 'f', 2), QString::number(_dyFirst, 'f', 2),
        QString::number(_driftRA, 'f', 2), QString::number(_driftDE, 'f', 2),
        QString::number(kiRA * _intRA, 'f', 2), QString::number(kiDE * _intDE, 'f', 2),
        QString::number(m.nInliers)
    });

    // Apply DEC compensation for current position
    // _calPulseRA is stored "equatorial" (DEC=0), adjust for the current DEC
    double currentDecCompensation = cos(_mountDEC * PI / 180.0);
    double calPulseRACompensated = _calPulseRA * currentDecCompensation;

    int  revRA = 1;
    if (getBool("revCorrections", "revRA")) revRA = -1;
    int  revDE = 1;
    if (getBool("revCorrections", "revDE")) revDE = -1;
    bool disRAO = getBool("disCorrections", "disRA+");
    bool disRAE = getBool("disCorrections", "disRA-");
    bool disDEN = getBool("disCorrections", "disDE+");
    bool disDES = getBool("disCorrections", "disDE-");

    if (revRA * driftRAeff > 0 && !disRAO)
    {
        _pulseE = getFloat("guideParams", "raAgr") * revRA * driftRAeff * calPulseRACompensated;
        if (_pulseE > getInt("guideParams", "pulsemax")) _pulseE = getInt("guideParams", "pulsemax");
        if (_pulseE < getInt("guideParams", "pulsemin")) _pulseE = 0;
    }
    else _pulseE = 0;

    if (revRA * driftRAeff < 0 && !disRAE)
    {
        _pulseW = -getFloat("guideParams", "raAgr") * revRA * driftRAeff * calPulseRACompensated;
        if (_pulseW > getInt("guideParams", "pulsemax")) _pulseW = getInt("guideParams", "pulsemax");
        if (_pulseW < getInt("guideParams", "pulsemin")) _pulseW = 0;
    }
    else _pulseW = 0;

    // DEC backlash compensation - step 1: learn from the previous reversal pulse's
    // effect (if one is pending), before deciding this cycle's pulse.
    if (_decBacklashLearning)
    {
        double calPulse = _calPulseDEC;
        double expectedReductionPx = (calPulse > 0) ? (_decBacklashCorrection / calPulse) : 0;
        double actualReductionPx   = qAbs(_decBacklashLastDriftDE) - qAbs(_driftDE);

        int amount = getInt("backlash", "amount");
        int step   = getInt("backlash", "step");
        int minAmt = getInt("backlash", "min");
        int maxAmt = getInt("backlash", "max");

        if (actualReductionPx < 0.8 * expectedReductionPx)
            amount = qMin(amount + step, maxAmt);
        else if (actualReductionPx > 1.2 * expectedReductionPx)
            amount = qMax(amount - step, minAmt);

        getEltInt("backlash", "amount")->setValue(amount, true);
        logInfo("DEC backlash compensation: expected %1px reduction, got %2px -> now %3ms",
        {
            QString::number(expectedReductionPx, 'f', 2),
            QString::number(actualReductionPx, 'f', 2),
            QString::number(amount)
        });

        _decBacklashLearning = false;
    }

    // DEC backlash compensation - step 2: decide this cycle's pulse and, if the
    // direction reversed compared to the last pulse actually sent, add the current
    // compensation estimate on top and remember enough to learn from it next cycle.
    int decDir = 0; // -1 = South needed, +1 = North needed, 0 = none
    if (revDE * driftDEeff > 0 && !disDEN) decDir = -1;
    else if (revDE * driftDEeff < 0 && !disDES) decDir = 1;

    if (decDir == -1)
    {
        _pulseS = getFloat("guideParams", "deAgr")  * revDE * driftDEeff * _calPulseDEC;
        if (_pulseS > getInt("guideParams", "pulsemax")) _pulseS = getInt("guideParams", "pulsemax");
        if (_pulseS < getInt("guideParams", "pulsemin")) _pulseS = 0;
    }
    else _pulseS = 0;

    if (decDir == 1)
    {
        _pulseN = -getFloat("guideParams", "deAgr") * revDE * driftDEeff * _calPulseDEC;
        if (_pulseN > getInt("guideParams", "pulsemax")) _pulseN = getInt("guideParams", "pulsemax");
        if (_pulseN < getInt("guideParams", "pulsemin")) _pulseN = 0;
    }
    else _pulseN = 0;

    if (getBool("backlash", "enable") && decDir != 0 && _lastDecDir != 0 && decDir != _lastDecDir)
    {
        int amount = getInt("backlash", "amount");
        double correctionMs = (decDir == -1) ? _pulseS : _pulseN;

        logInfo("DEC direction reversed to %1 - adding %2ms backlash compensation (current estimate)",
        {decDir == -1 ? "South" : "North", QString::number(amount)});

        if (decDir == -1) _pulseS += amount;
        else               _pulseN += amount;

        _decBacklashLearning    = true;
        _decBacklashDir         = decDir;
        _decBacklashCorrection  = correctionMs;
        _decBacklashLastDriftDE = _driftDE;
    }

    if (decDir != 0) _lastDecDir = decDir;

    // Anti-windup: stop integrating an axis whose correction is already maxed out.
    int pmax = getInt("guideParams", "pulsemax");
    _intRAsat = (_pulseE >= pmax || _pulseW >= pmax);
    _intDEsat = (_pulseN >= pmax || _pulseS >= pmax);

    _itt++;

    // Store drift history for RMS calculation
    _dRAvector.push_back(_driftRA * getSampling());
    _dDEvector.push_back(_driftDE * getSampling());

    // Limit vector size to rmsOver parameter
    int rmsOver = getInt("guideParams", "rmsOver");
    while (_dRAvector.size() > (size_t)rmsOver)
    {
        _dRAvector.erase(_dRAvector.begin());
    }
    while (_dDEvector.size() > (size_t)rmsOver)
    {
        _dDEvector.erase(_dDEvector.begin());
    }

    // Calculate RMS
    double rmsRA = 0;
    double rmsDEC = 0;
    double rmsTotal = 0;

    if (_dRAvector.size() > 0)
    {
        for (size_t i = 0; i < _dRAvector.size(); i++)
        {
            rmsRA += square(_dRAvector[i]);
            rmsDEC += square(_dDEvector[i]);
        }
        rmsRA = sqrt(rmsRA / _dRAvector.size());
        rmsDEC = sqrt(rmsDEC / _dDEvector.size());
        rmsTotal = sqrt(square(rmsRA) + square(rmsDEC));
    }

    getProperty("drift")->setGridLimit(rmsOver);
    getProperty("guiding")->setGridLimit(rmsOver);

    getEltInt("values", "pulseN")->setValue(_pulseN);
    getEltInt("values", "pulseS")->setValue(_pulseS);
    getEltInt("values", "pulseE")->setValue(_pulseE);
    getEltInt("values", "pulseW")->setValue(_pulseW);
    getEltFloat("values", "rmsRA")->setValue(rmsRA);
    getEltFloat("values", "rmsDEC")->setValue(rmsDEC);
    getEltFloat("values", "rmsTotal")->setValue(rmsTotal, true);
    double ech = getSampling();
    getEltFloat("drift", "RA")->setValue(_driftRA * ech);
    getEltFloat("drift", "DEC")->setValue(_driftDE * ech, true);
    getProperty("drift")->push();

    double tt = QDateTime::currentDateTime().toMSecsSinceEpoch();
    getEltFloat("guiding", "time")->setValue(tt);
    getEltFloat("guiding", "RA")->setValue(_driftRA * ech);
    getEltFloat("guiding", "DE")->setValue(_driftDE * ech);
    getEltFloat("guiding", "pDE")->setValue(_pulseN - _pulseS);
    getEltFloat("guiding", "pRA")->setValue( _pulseE - _pulseW);
    getEltFloat("guiding", "SNR")->setValue(_image->getStats().SNR);
    getEltFloat("guiding", "RMS")->setValue(rmsTotal);
    getProperty("guiding")->push();

    setStateEvent(OST::Busy, "guiding", "guideRMS", "guide RMS",
                  rmsTotal);
    setStateEvent(OST::Busy, "guiding", "guideSNR", "guide SNR",
                  qRound(_image->getStats().SNR));

    emit ComputeGuideDone();
}
void Guider::SMRequestPulses()
{

    //logInfo("SMRequestPulses");
    INDI::BaseDevice dp = getDevice(getString("devices", "guider").toStdString().c_str());

    if (_pulseN > 0)
    {
        //qDebug() << "********* Pulse  N " << _pulseN;
        _pulseDECfinished = false;
        INDI::PropertyNumber prop = dp.getNumber("TELESCOPE_TIMED_GUIDE_NS");
        for (std::size_t i = 0; i < prop.size(); i++)
        {
            if (strcmp(prop[i].name, "TIMED_GUIDE_N") == 0)
            {
                prop[i].value = _pulseN;
            }
            else prop[i].value = 0;
        }
        sendNewNumber(prop);
    }

    if (_pulseS > 0)
    {
        _pulseDECfinished = false;
        //qDebug()  << "********* Pulse  S " << _pulseS;
        INDI::PropertyNumber prop = dp.getNumber("TELESCOPE_TIMED_GUIDE_NS");
        for (std::size_t i = 0; i < prop.size(); i++)
        {
            if (strcmp(prop[i].name, "TIMED_GUIDE_S") == 0)
            {
                prop[i].value = _pulseS;
            }
            else prop[i].value = 0;
        }
        sendNewNumber(prop);
    }

    if (_pulseE > 0)
    {
        _pulseRAfinished = false;
        //qDebug()  << "********* Pulse  E " << _pulseE;
        INDI::PropertyNumber prop = dp.getNumber("TELESCOPE_TIMED_GUIDE_WE");
        for (std::size_t i = 0; i < prop.size(); i++)
        {
            if (strcmp(prop[i].name, "TIMED_GUIDE_E") == 0)
            {
                prop[i].value = _pulseE;
            }
            else prop[i].value = 0;
        }
        sendNewNumber(prop);
    }

    if (_pulseW > 0)
    {
        _pulseRAfinished = false;
        //qDebug()  << "********* Pulse  W " << _pulseW;
        INDI::PropertyNumber prop = dp.getNumber("TELESCOPE_TIMED_GUIDE_WE");
        for (std::size_t i = 0; i < prop.size(); i++)
        {
            if (strcmp(prop[i].name, "TIMED_GUIDE_W") == 0)
            {
                prop[i].value = _pulseW;
            }
            else prop[i].value = 0;
        }
        sendNewNumber(prop);
    }

    emit RequestPulsesDone();

    if ((_pulseN == 0) && (_pulseS == 0) && (_pulseE == 0) && (_pulseW == 0))
    {
        emit PulsesDone();   // nothing to send this cycle
    }
    else
    {
        armWatchdog();       // waiting for TELESCOPE_TIMED_GUIDE_* -> IPS_IDLE
    }
}

void Guider::SMFindStars()
{

    //logInfo("SMFindStars");
    stats = _image->getStats();
    _solver.ResetSolver(stats, _image->getImageBuffer());
    connect(&_solver, &Solver::successSEP, this, &Guider::OnSucessSEP);
    _solver.stars.clear();
    armWatchdog();   // waiting for the SEP extraction to finish
    _solver.FindStars(_solver.stellarSolverProfiles[0]);
}

void Guider::OnSucessSEP()
{
    OST::ImgData dta = _image->ImgStats();

    double ech = getSampling();
    dta.HFRavg = _solver.HFRavg * ech;
    dta.starsCount = _solver.stars.size();

    // Draw star circles (radius = 2 × HFR) on the image and refresh the JPEG
    QImage rawImage = _image->getRawQImage();
    QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
    im.setColorTable(rawImage.colorTable());
    {
        QPainter painter(&im);
        painter.setPen(QPen(Qt::red, 2));
        for (const FITSImage::Star &star : _solver.stars)
        {
            double r = star.HFR * 3;
            painter.drawEllipse(QPointF(star.x / 2.0, star.y / 2.0), r, r);
        }
        painter.setPen(QPen(Qt::green, 2));
        for (const FITSImage::Star &star : starsFirst)
        {
            double r = star.HFR * 3;
            painter.drawEllipse(QPointF(star.x / 2.0, star.y / 2.0), r, r);
        }
    }
    atomicSaveJpeg(im, getWebroot() + "/" + getModuleName() + ".jpeg");
    dta.mUrlJpeg = getModuleName() + ".jpeg";

    getEltImg("image", "image")->setValue(dta, true);

    if (_solver.stars.size() < 3 )
    {
        logError("We need at least 3 stars to guide - abort");
        setStateEvent(OST::Error, "error", "nostars", "no stars found");
        emit Abort();
        return;
    }

    //logInfo("SEP finished");
    disconnect(&_solver, &Solver::successSEP, this, &Guider::OnSucessSEP);
    emit FindStarsDone();
}

void Guider::SMAbort()
{
    logInfo("Aborting guiding");
    disarmWatchdog();
    _expectingFrame = false;
    getEltBool("actions", "calibrate")->setValue(false, false);
    getEltBool("actions", "abortguider")->setValue(false, false);
    getEltBool("actions", "guide")->setValue(false, false);
    getEltBool("actions", "resetcalibration")->setValue(false, false);
    getEltBool("actions", "abortguider")->setValue(true, true);
    getProperty("actions")->setState(OST::Ok, true);


    disconnect(&_SMInit,        &QStateMachine::finished, nullptr, nullptr);
    disconnect(&_SMCalibration, &QStateMachine::finished, nullptr, nullptr);
    _SMInit.stop();
    _SMCalibration.stop();
    _SMGuide.stop();

    emit AbortDone();
    // Defer the "ready" notification so the queued Abort→End transition
    // is fully processed before the sequencer can call guide/start again.
    QTimer::singleShot(0, this, [this]()
    {
        setStateEvent(OST::Ok, "ready", "abortguide", "Abort guide");
        logInfo("Guiding aborted");
    });

}

