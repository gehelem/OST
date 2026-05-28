/**
 * @file guider.cpp
 * @brief Autoguiding module for OST - Corrects telescope tracking drift
 *
 * This module implements a complete autoguiding system with three phases:
 *   1. INITIALIZATION: Connect devices, capture reference star field
 *   2. CALIBRATION: Measure how many pixels correspond to 1ms pulse in each direction
 *   3. GUIDING: Continuous loop - detect drift, send correcting pulses
 *
 * Algorithm: Trigonometric matching uses triangle indices from star triangles.
 * Triangles are invariant under translation/rotation/scaling, making them robust.
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
//#include "polynomialfit.h"
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

                // Check if calibration data exists
                double calN = getFloat("calibrationvalues", "calPulseN");
                double calS = getFloat("calibrationvalues", "calPulseS");
                double calE = getFloat("calibrationvalues", "calPulseE");
                double calW = getFloat("calibrationvalues", "calPulseW");

                // If no calibration, must do it first
                if (calN == 0 || calS == 0 || calE == 0 || calW == 0)
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
        if (_pulseRAfinished && _pulseDECfinished) emit PulsesDone();
    }


}

void Guider::newBLOB(INDI::PropertyBlob pblob)
{
    if (
        (QString(pblob.getDeviceName()) == getString("devices", "camera"))
    )
    {
        delete _image;
        _image = new fileio();
        _image->loadBlob(pblob, 64);
        stats = _image->getStats();
        QImage rawImage = _image->getRawQImage();
        QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
        im.setColorTable(rawImage.colorTable());

        emit ExposureDone();
    }

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

    // Get pier side (West or East of pier)
    // Affects CCD orientation if optical tube is flipped
    if (!getModSwitch(getString("devices", "guider"), "TELESCOPE_PIER_SIDE", "PIER_WEST", _mountPointingWest))
    {
        logError("Failed to read mount pier side");
        setStateEvent(OST::Error, "error", "devicefailed", "mount failed");
        ;
        return;
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
 * This handler initializes counters and loads the reference star field (_trigFirst)
 * from the initialization phase.
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

    // Clear previous calibration results
    _calPulseN = 0;  // Will be filled by calibration
    _calPulseS = 0;
    _calPulseE = 0;
    _calPulseW = 0;

    // Update UI with current (empty) calibration values
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

    // Prepare triangle indices for matching
    _trigCurrent.clear();  // Will be filled as we take exposures
    _trigPrev = _trigFirst; // Use reference from initialization phase

    // Clear polynomial fitting data (for CCD orientation calculation - currently unused)
    _dxvector.clear();
    _dyvector.clear();
    _coefficients.clear();

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

    // Set grid limits to match RMS buffer size for consistent visualization
    int rmsOver = getInt("guideParams", "rmsOver");
    getProperty("drift")->setGridLimit(rmsOver);
    getProperty("guiding")->setGridLimit(rmsOver);
    logInfo("Grid limits set to %1 frames (rmsOver parameter)", {QString::number(rmsOver)});

    // Load calibration results from database
    _calPulseN = getFloat("calibrationvalues", "calPulseN");
    _calPulseS = getFloat("calibrationvalues", "calPulseS");
    _calPulseE = getFloat("calibrationvalues", "calPulseE");
    _calPulseW = getFloat("calibrationvalues", "calPulseW");
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

    // Show what the compensation does
    double calPulseECompensated = _calPulseE * currentDecCompensation;
    double calPulseWCompensated = _calPulseW * currentDecCompensation;

    logInfo("RA compensation factor: %1(cos(%2°))", {QString::number(currentDecCompensation, 'f', 3), QString::number(_mountDEC, 'f', 1)});
    logInfo("Adjusted calibration: E=%1 W=%2 N=%3 S=%4 (pixels/sec)", {QString::number(calPulseECompensated, 'f', 2),
            QString::number(calPulseWCompensated, 'f', 2),
            QString::number(_calPulseN, 'f', 2),
            QString::number(_calPulseS, 'f', 2)
                                                                      });

    // Clear RMS drift history from previous guiding sessions
    // These will accumulate as new measurements come in
    _dRAvector.clear();
    _dDEvector.clear();

    _doDither = false;

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
    emit RequestExposureDone();
}
void Guider::SMComputeFirst()
{
    _trigFirst.clear();
    buildIndexes(_solver, _trigFirst);


    emit ComputeFirstDone();
}
void Guider::SMComputeCal()
{
    //qDebug()  << "SMComputeCal" << _calStep << _calState;
    buildIndexes(_solver, _trigCurrent);
    _ccdOrientation = 0;

    double coeff[2];
    Q_UNUSED(coeff);
    if (_trigCurrent.size() > 0)
    {
        matchIndexes(_trigPrev, _trigCurrent, _matchedCurPrev, _dxPrev, _dyPrev);
        matchIndexes(_trigFirst, _trigCurrent, _matchedCurFirst, _dxFirst, _dyFirst);
        //_grid->append(_dxFirst,_dyFirst);
        //_propertyStore.update(_grid);
        //emit propertyAppended(_grid,&_modulename,0,_dxFirst,_dyFirst,0,0);
        _dxvector.push_back(_dxPrev);
        _dyvector.push_back(_dyPrev);
        /*if (_dxvector.size() > 1)
        {
            polynomialfit(_dxvector.size(), 2, _dxvector.data(), _dyvector.data(), coeff);
            BOOST_LOG_TRIVIAL(debug) << "Coeffs " << coeff[0] << "-" <<  coeff[1] << " CCD Orientation = " << atan(coeff[1])*180/PI;
        }*/


    }
    else
    {
        logError("No stars, can't calibrate");
        setStateEvent(OST::Error, "error", "nostars", "no stars");
        emit Abort();
        return;
    }
    _trigPrev = _trigCurrent;

    /*if (_calState==0) {
        BOOST_LOG_TRIVIAL(debug) << "RA drift " << sqrt(square(_avdx)+square(_avdy)) << " drift / ms = " << 1000*sqrt(square(_avdx)+square(_avdy))/_pulseWTot;
    }
    if (_calState==2) {
        BOOST_LOG_TRIVIAL(debug) << "DEC drift " << sqrt(square(_avdx)+square(_avdy)) << " drift / ms = " << 1000*sqrt(square(_avdx)+square(_avdy))/_pulseNTot;
    }*/
    _pulseN = 0;
    _pulseS = 0;
    _pulseE = 0;
    _pulseW = 0;
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
        double a = atan2(ddy, ddx);
        //ddy));
        if (_calState == 0)
        {
            _calPulseW = getInt("calParams", "pulse") / sqrt(square(ddx) + square(ddy));
            _ccdOrientation = a;
            _calMountPointingWest = _mountPointingWest;
            _calCcdOrientation = _ccdOrientation;
            double ech = getSampling();
            double drift_arcsec = sqrt(square(ddx) + square(ddy)) * ech;
            logInfo("West calibration complete: %1 ms/px (%2 ms/arcsec, drift=%3\")", {QString::number(_calPulseW, 'f', 2), QString::number(_calPulseW / ech, 'f', 2), QString::number(drift_arcsec, 'f', 2)});
            setStateEvent(OST::Busy, "cal", "westcomplete", "westcomplete");
        }
        if (_calState == 1)
        {
            _calPulseE = getInt("calParams", "pulse") / sqrt(square(ddx) + square(ddy));
            double ech = getSampling();
            double drift_arcsec = sqrt(square(ddx) + square(ddy)) * ech;
            logInfo("East calibration complete: %1 ms/px (%2 ms/arcsec, drift=%3\")", {QString::number(_calPulseE, 'f', 2), QString::number(_calPulseE / ech, 'f', 2), QString::number(drift_arcsec, 'f', 2)});
            setStateEvent(OST::Busy, "cal", "estcomplete", "estcomplete");
        }
        if (_calState == 2)
        {
            _calPulseN = getInt("calParams", "pulse") / sqrt(square(ddx) + square(ddy));
            double ech = getSampling();
            double drift_arcsec = sqrt(square(ddx) + square(ddy)) * ech;
            logInfo("North calibration complete: %1 ms/px (%2 ms/arcsec, drift=%3\")", {QString::number(_calPulseN, 'f', 2), QString::number(_calPulseN / ech, 'f', 2), QString::number(drift_arcsec, 'f', 2)});
            setStateEvent(OST::Busy, "cal", "northcomplete", "northcomplete");
        }
        if (_calState == 3)
        {
            _calPulseS = getInt("calParams", "pulse") / sqrt(square(ddx) + square(ddy));
            double ech = getSampling();
            double drift_arcsec = sqrt(square(ddx) + square(ddy)) * ech;
            logInfo("South calibration complete: %1 ms/px (%2 ms/arcsec, drift=%3\")", {QString::number(_calPulseS, 'f', 2), QString::number(_calPulseS / ech, 'f', 2), QString::number(drift_arcsec, 'f', 2)});
            setStateEvent(OST::Busy, "cal", "southcomplete", "southcomplete");
        }

        _calStep = 0;
        _calState++;
        //if (_calState==2) {
        _dxvector.clear();
        _dyvector.clear();
        _coefficients.clear();
        //}
        if (_calState >= 4)
        {

            // Store calibration DEC for later compensation
            _calMountDEC = _mountDEC;

            // Compensate RA calibration values for declination
            // Store as "equatorial" values (compensated to DEC=0)
            double decCompensation = cos(_calMountDEC * PI / 180.0);
            if (decCompensation > 0.1)  // Avoid division by zero near poles
            {
                _calPulseE = _calPulseE / decCompensation;
                _calPulseW = _calPulseW / decCompensation;
                logInfo("DEC compensation applied: DEC=%1° factor=%2", {QString::number(_calMountDEC, 'f', 1), QString::number(decCompensation, 'f', 3)});
            }

            // Store all calibration values for persistent reuse
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
            _trigFirst = _trigCurrent;
            return;
        }
    }
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
    buildIndexes(_solver, _trigCurrent);

    if (_trigCurrent.size() > 0)
    {
        matchIndexes(_trigFirst, _trigCurrent, _matchedCurFirst, _dxFirst, _dyFirst);
    }
    else
    {
        logWarning("No stars detected in current frame - skipping corrections");
        emit ComputeGuideDone();
        return;
    }

    // Dither requested: compute random displacement pulses then rebuild reference
    if (_doDither)
    {
        double currentDecCompensation = cos(_mountDEC * PI / 180.0);
        int ditherpixel = getInt("guideParams", "ditherpixel");
        double randRA  = (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * ditherpixel;
        double randDEC = (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * ditherpixel;

        // RA: calPulseE/W are equatorial (DEC-normalized), apply current DEC compensation
        if (randRA > 0)
            _pulseW = randRA * _calPulseW * currentDecCompensation;
        else
            _pulseE = -randRA * _calPulseE * currentDecCompensation;

        // DEC: calPulseN/S are raw ms/pixel, no compensation needed
        if (randDEC > 0)
            _pulseN = randDEC * _calPulseN;
        else
            _pulseS = -randDEC * _calPulseS;

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

    // Apply DEC compensation for current position
    // calPulseE/W are stored as "equatorial" (DEC=0), need to adjust for current DEC
    double currentDecCompensation = cos(_mountDEC * PI / 180.0);
    double calPulseECompensated = _calPulseE * currentDecCompensation;
    double calPulseWCompensated = _calPulseW * currentDecCompensation;

    int  revRA = 1;
    if (getBool("revCorrections", "revRA")) revRA = -1;
    int  revDE = 1;
    if (getBool("revCorrections", "revDE")) revDE = -1;
    bool disRAO = getBool("disCorrections", "disRA+");
    bool disRAE = getBool("disCorrections", "disRA-");
    bool disDEN = getBool("disCorrections", "disDE+");
    bool disDES = getBool("disCorrections", "disDE-");

    if (revRA * _driftRA > 0 && !disRAO)
    {
        _pulseE = getFloat("guideParams", "raAgr") * revRA * _driftRA * calPulseECompensated;
        if (_pulseE > getInt("guideParams", "pulsemax")) _pulseE = getInt("guideParams", "pulsemax");
        if (_pulseE < getInt("guideParams", "pulsemin")) _pulseE = 0;
    }
    else _pulseE = 0;

    if (revRA * _driftRA < 0 && !disRAE)
    {
        _pulseW = -getFloat("guideParams", "raAgr") * revRA * _driftRA * calPulseWCompensated;
        if (_pulseW > getInt("guideParams", "pulsemax")) _pulseW = getInt("guideParams", "pulsemax");
        if (_pulseW < getInt("guideParams", "pulsemin")) _pulseW = 0;
    }
    else _pulseW = 0;

    if (revDE * _driftDE > 0 && !disDEN)
    {
        _pulseS = getFloat("guideParams", "deAgr")  * revDE * _driftDE * _calPulseS;
        if (_pulseS > getInt("guideParams", "pulsemax")) _pulseS = getInt("guideParams", "pulsemax");
        if (_pulseS < getInt("guideParams", "pulsemin")) _pulseS = 0;
    }
    else _pulseS = 0;

    if (revDE * _driftDE < 0 && !disDES)
    {
        _pulseN = -getFloat("guideParams", "deAgr") * revDE * _driftDE * _calPulseN;
        if (_pulseN > getInt("guideParams", "pulsemax")) _pulseN = getInt("guideParams", "pulsemax");
        if (_pulseN < getInt("guideParams", "pulsemin")) _pulseN = 0;
    }
    else _pulseN = 0;

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
        emit PulsesDone();
    }

}

void Guider::SMFindStars()
{

    //logInfo("SMFindStars");
    stats = _image->getStats();
    _solver.ResetSolver(stats, _image->getImageBuffer());
    connect(&_solver, &Solver::successSEP, this, &Guider::OnSucessSEP);
    _solver.stars.clear();
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
        painter.setPen(QPen(Qt::green, 2));
        for (const FITSImage::Star &star : _solver.stars)
        {
            double r = star.HFR * 3;
            painter.drawEllipse(QPointF(star.x / 2.0, star.y / 2.0), r, r);
        }
    }
    im.save(getWebroot() + "/" + getModuleName() + ".jpeg", "JPG", 100);
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

void Guider::matchIndexes(QVector<Trig> ref, QVector<Trig> act, QVector<MatchedPair> &pairs, double &dx, double &dy)
{
    pairs.clear();

    foreach (Trig r, ref)
    {
        foreach (Trig a, act)
        {
            if (
                (r.s < a.s * 1.001 ) && (r.s > a.s * 0.999 ) && (r.p < a.p * 1.001 ) && (r.p > a.p * 0.999 )
                && (r.d12 < a.d12 * 1.001) && (r.d12 > a.d12 * 0.999)
                && (r.d13 < a.d13 * 1.001) && (r.d13 > a.d13 * 0.999)
                && (r.d23 < a.d23 * 1.001) && (r.d23 > a.d23 * 0.999)
            )
            {
                bool found;
                found = false;
                foreach (MatchedPair pair, pairs)
                {
                    if ( (pair.xr == r.x1) && (pair.yr == r.y1) ) found = true;
                }
                if (!found) pairs.append({r.x1, r.y1, a.x1, a.y1, r.x1 - a.x1, r.y1 - a.y1});
                found = false;
                foreach (MatchedPair pair, pairs)
                {
                    if ( (pair.xr == r.x2) && (pair.yr == r.y2) ) found = true;
                }
                if (!found) pairs.append({r.x2, r.y2, a.x2, a.y2, r.x2 - a.x2, r.y2 - a.y2});
                found = false;
                foreach (MatchedPair pair, pairs)
                {
                    if ( (pair.xr == r.x3) && (pair.yr == r.y3) ) found = true;
                }
                if (!found) pairs.append({r.x3, r.y3, a.x3, a.y3, r.x3 - a.x3, r.y3 - a.y3});
            }
        }
    }
    dx = 0;
    dy = 0;
    for (int i = 0 ; i < pairs.size(); i++ )
    {
        dx = dx + pairs[i].dx;
        dy = dy + pairs[i].dy;
    }
    if (pairs.isEmpty())
    {
        dx = 0;
        dy = 0;
        return;
    }
    dx = dx / pairs.size();
    dy = dy / pairs.size();



    /*foreach (MatchedPair pair, _matchedPairs) {
            BOOST_LOG_TRIVIAL(debug) << "Matched pair =  " << pair.dx << "-" << pair.dy;
    }*/

}
void Guider::buildIndexes(Solver &solver, QVector<Trig> &trig)
{
    int nb = solver.stars.size();
    if (nb > 10) nb = 10;
    trig.clear();

    for (int i = 0; i < nb; i++)
    {
        for (int j = i + 1; j < nb; j++)
        {
            for (int k = j + 1; k < nb; k++)
            {
                double dij, dik, djk, p, s;
                dij = sqrt(square(solver.stars[i].x - solver.stars[j].x) + square(solver.stars[i].y - solver.stars[j].y));
                dik = sqrt(square(solver.stars[i].x - solver.stars[k].x) + square(solver.stars[i].y - solver.stars[k].y));
                djk = sqrt(square(solver.stars[j].x - solver.stars[k].x) + square(solver.stars[j].y - solver.stars[k].y));
                p = dij + dik + djk;
                s = sqrt(p * (p - dij) * (p - dik) * (p - djk));
                trig.append(
                {
                    solver.stars[i].x,
                    solver.stars[i].y,
                    solver.stars[j].x,
                    solver.stars[j].y,
                    solver.stars[k].x,
                    solver.stars[k].y,
                    dij, dik, djk,
                    p, s, s / p
                });

            }
        }

    }

}
