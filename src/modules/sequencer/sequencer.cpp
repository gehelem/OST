#include "sequencer.h"
#include <QPainter>
#include <QTimer>
#include <QStorageInfo>
#include "version.cc"

Sequencer *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Sequencer *basemodule = new Sequencer(name, label, profile, availableModuleLibs);
    return basemodule;
}

Sequencer::Sequencer(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{
    setMetadata("thisGithash",    QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate",    QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description",    "Sequencer module");
    setMetadata("thisversion",    QString::fromStdString(Version::GIT_TAG));
    setMetadata("template",       "sequencer");

    loadOstPropertiesFromFile(":sequencer.json");

    giveMeADevice("camera", "Camera",       INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("filter", "Filter wheel", INDI::BaseDevice::FILTER_INTERFACE);
    getEltString("devices", "camera")->setLovConstrained(true);
    getEltString("devices", "filter")->setLovConstrained(true);
    getEltString("devices", "filter")->setNullable(true);
    getEltString("focusparameters", "focusprofile")->setLovConstrained(true);
    getEltString("focusparameters", "focusprofile")->setNullable(true);
    // FILTER_SLOT is a numeric INDI property - "filter" is declared "int"
    // in sequencer.json (issue #240), its LOV key is the real slot number,
    // no string round-trip needed when sending to the driver.
    getEltInt("sequence", "filter")->setLovConstrained(true);
    getEltInt("sequence", "filter")->setNullable(true);
    getEltInt("focusprofiles", "filter")->setLovConstrained(true);
    getEltInt("livesequence", "filter")->setLovConstrained(true);
    defineMeAsSequencer();
    refreshFilterLov();

    mSettleTimer = new QTimer(this);
    mSettleTimer->setSingleShot(true);
    connect(mSettleTimer, &QTimer::timeout, this, &Sequencer::onGuidingSettleTimeout);

    mElapsedTimer = new QTimer(this);
    mElapsedTimer->setInterval(1000);
    connect(mElapsedTimer, &QTimer::timeout, this, &Sequencer::onElapsedTimerTick);

    recomputeSequenceTotals();

    pMachine = QScxmlStateMachine::fromFile(":sequencer.scxml");

    // Pre-sequence
    pMachine->connectToState("WaitCooling",   QScxmlStateMachine::onEntry(this, &Sequencer::SMWaitCooling));
    // SequenceCtrl
    pMachine->connectToState("InitLine",      QScxmlStateMachine::onEntry(this, &Sequencer::SMInitLine));
    pMachine->connectToState("FilterStep",    QScxmlStateMachine::onEntry(this, &Sequencer::SMFilterStep));
    pMachine->connectToState("FocusGate",     QScxmlStateMachine::onEntry(this, &Sequencer::SMFocusGate));
    pMachine->connectToState("GuideGate",     QScxmlStateMachine::onEntry(this, &Sequencer::SMGuideGate));
    pMachine->connectToState("DitherGate",    QScxmlStateMachine::onEntry(this, &Sequencer::SMDitherGate));
    pMachine->connectToState("WaitSettle",    QScxmlStateMachine::onEntry(this, &Sequencer::SMWaitSettle));
    pMachine->connectToState("Exposing",      QScxmlStateMachine::onEntry(this, &Sequencer::SMExposing));
    pMachine->connectToState("FindingStars",  QScxmlStateMachine::onEntry(this, &Sequencer::SMFindStars));
    pMachine->connectToState("EvalShot",      QScxmlStateMachine::onEntry(this, &Sequencer::SMEvalShot));
    // FocusCtrl
    pMachine->connectToState("Focusing",      QScxmlStateMachine::onEntry(this, &Sequencer::SMFocusing));
    // GuideCtrl
    pMachine->connectToState("GuideStarting", QScxmlStateMachine::onEntry(this, &Sequencer::SMGuideStarting));
    pMachine->connectToState("Dithering",     QScxmlStateMachine::onEntry(this, &Sequencer::SMDithering));
    pMachine->connectToState("Recalibrating", QScxmlStateMachine::onEntry(this, &Sequencer::SMRecalibrating));
    // Terminal
    pMachine->connectToState("Done",          QScxmlStateMachine::onEntry(this, &Sequencer::SMDone));
    pMachine->connectToState("Aborted",       QScxmlStateMachine::onEntry(this, &Sequencer::SMAborted));
    pMachine->connectToState("Error",         QScxmlStateMachine::onEntry(this, &Sequencer::SMError));

    // SMError() alone can't tell which of the 9 "target=Error" transitions
    // fired - QScxmlStateMachine::onEntry's connected slot only gets a
    // bool (entering/leaving), never the triggering event. connectToEvent()
    // does receive it (QScxmlEvent, see qscxmlevent.h) - record it here so
    // SMError() can log something actionable instead of just "Sequence
    // error" with no reason (Gilles, 2026-09-26, hitting exactly that).
    for (const QString errorEvent :
            {"CameraAlert", "CoolingTimeout", "FilterTimeout",
             "FocusTimeout", "GuiderTimeout", "GuiderLost", "ExposureTimeout",
             "DitherTimeout", "RecalTimeout"
            })
    {
        pMachine->connectToEvent(errorEvent, this, &Sequencer::SMErrorCause);
    }
}
void Sequencer::SMErrorCause(const QScxmlEvent &event)
{
    mLastErrorCause = event.name();
}

Sequencer::~Sequencer() {}

// =============================================================================
// External events (frontend → sequencer)
// =============================================================================

void Sequencer::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        if (event.eltkey == "startsequence" && getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
        {
            mCurrentLine          = -1;
            mShotCount            = 0;
            mShotsSinceDither     = 0;
            mFilterChanged        = false;
            mGuiderActive         = false;
            mGuiderWasSuspended   = false;
            mLastHFR              = 0.0;
            mMaxRMSDuringExposure = 0.0;
            mPreviousFilter.clear();
            mObjectName = getString("object", "label");
            mDate       = QDateTime::currentDateTime().toString("yyyyMMdd-hh-mm-ss");

            QDir dir;
            dir.mkdir(getWebroot() + "/" + getModuleName());
            dir.mkdir(getWebroot() + "/" + getModuleName() + "/" + mObjectName);

            connectIndi();
            if (!connectDevice(getString("devices", "camera")))
            {
                setStateEvent(OST::Error, "error", "deviceerror", "Device error");
                return;
            }
            setBLOBMode(B_ALSO, getString("devices", "camera").toStdString().c_str(), nullptr);
            frameReset(getString("devices", "camera"));

            if (getString("devices", "camera") == "CCD Simulator")
                sendModNewNumber(getString("devices", "camera"), "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 1);

            getProperty("actions")->setState(OST::Busy, true);

            mShotsCompleted   = 0;
            mSecondsCompleted = 0.0;
            recomputeSequenceTotals();
            buildLiveSequence();

            mSequenceStartTime = QDateTime::currentDateTime();
            getEltString("progress", "realelapsed")->setValue(formatDuration(0), true);
            mElapsedTimer->start();

            setStateEvent(OST::Busy, "running", "startsequence", "Start sequence");
            pMachine->init();
            pMachine->start();
            pMachine->submitEvent("startsequence");
        }

        if (event.eltkey == "abortsequence" && getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
        {
            mSettleTimer->stop();
            if (pMachine->isRunning())
                pMachine->submitEvent("abort");

            otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "abortguider", true);
            otherModuleSetValue(getString("slaves", "focusmodule"), "actions", "abortfocus", true);
        }
    }

    if (event.prpkey == "devices")
        refreshFilterLov();

    // Grid edited (line created/updated/deleted/reordered/reset): the theoretical
    // duration shown in "progress" must reflect the grid even before the sequence runs.
    if (event.prpkey == "sequence" &&
            (event.ev == OST::ExtEvType::GC || event.ev == OST::ExtEvType::GU ||
             event.ev == OST::ExtEvType::GD || event.ev == OST::ExtEvType::GH ||
             event.ev == OST::ExtEvType::GB || event.ev == OST::ExtEvType::GR))
    {
        recomputeSequenceTotals();
    }
}

// =============================================================================
// Pre-sequence entry points
// =============================================================================

void Sequencer::SMWaitCooling()
{
    //qDebug() << "SMWaitCooling";
    if (!getBool("parameters", "enablecooling"))
    {
        pMachine->submitEvent("CoolingReady");
        return;
    }

    QString camera = getString("devices", "camera");
    if (!getDevice(camera.toStdString().c_str()).isValid())
    {
        pMachine->submitEvent("CoolingReady");
        return;
    }

    double target = getFloat("parameters", "targettemperature");
    logInfo("Requesting camera cooling to %1°C", {QString::number(target, 'f', 1)});
    sendModNewNumber(camera, "CCD_TEMPERATURE", "CCD_TEMPERATURE_VALUE", target);
    // CoolingReady submitted by updateProperty() when CCD_TEMPERATURE reaches IPS_OK
}

// =============================================================================
// SequenceCtrl entry points
// =============================================================================

void Sequencer::SMInitLine()
{
    //qDebug() << "SMInitLine";
    mCurrentLine++;

    if (mCurrentLine >= getProperty("livesequence")->getGrid().count())
    {
        // SequenceDoneAll bubbles up through Running to Active → Done.
        // We avoid done.state.Running — Qt SCXML re-enters the parallel state instead of exiting it.
        pMachine->submitEvent("SequenceDoneAll");
        return;
    }

    setStateEvent(OST::Busy, "running", "startline", "Starting line");

    // Reads "livesequence" (the frozen snapshot, issue #241), never
    // "sequence" again once a run has started - "sequence" is only read
    // once, by buildLiveSequence(), so editing it mid-run can never
    // disturb the line currently executing.
    getProperty("livesequence")->fetchLine(mCurrentLine);
    mShotCount            = getInt("livesequence", "count");
    int filterIndex       = getInt("livesequence", "filter");
    const auto &lov       = getEltInt("livesequence", "filter")->getLov();
    mCurrentFilter        = lov.contains(filterIndex) ? lov[filterIndex] : QString();
    mCurrentFilterIndex   = filterIndex;
    mCurrentFrameType     = getString("parameters", "frametype");
    mLastHFR              = 0.0;
    mMaxRMSDuringExposure = 0.0;
    mLineHFRSum           = 0.0;
    mLineHFRSamples       = 0;

    pMachine->submitEvent("LineLoaded");
}

void Sequencer::SMFilterStep()
{
    //qDebug() << "SMFilterStep";
    int filterIndex = getInt("livesequence", "filter");
    mFilterChanged  = !mPreviousFilter.isEmpty() && (mPreviousFilter != mCurrentFilter);

    if (mPreviousFilter.isEmpty() && !mCurrentFilter.isEmpty())
        logInfo("Set filter to %1", {mCurrentFilter});
    else if (mFilterChanged)
        logInfo("Changing filter from %1 to %2", {mPreviousFilter, mCurrentFilter});

    mPreviousFilter = mCurrentFilter;

    setupOutputFolder();

    QString filterDevice = getString("devices", "filter");
    // FILTER_SLOT is 1-based: no device, or no filter selected for this
    // line (e.g. Bias/Dark frames), means nothing to command here.
    if (!getDevice(filterDevice.toStdString().c_str()).isValid() || filterIndex <= 0)
    {
        pMachine->submitEvent("FilterReady");
        return;
    }

    if (!getEltInt("livesequence", "filter")->getLov().contains(filterIndex))
    {
        // Stale slot (e.g. the wheel now reports fewer filters than when
        // this line was configured) - never guess, never send an
        // out-of-range slot to the driver (issue #240). Stay here and let
        // this state's own FilterTimeout watchdog (sequencer.scxml) route
        // to Error instead.
        logWarning("Filter slot %1 is not valid on this wheel - not sending FILTER_SLOT", {QString::number(filterIndex)});
        return;
    }

    // INDI FILTER_SLOT OK → updateProperty() submits FilterReady
    sendModNewNumber(filterDevice, "FILTER_SLOT", "FILTER_SLOT_VALUE", filterIndex);
}

void Sequencer::SMFocusGate()
{
    //qDebug() << "SMFocusGate";
    bool isLightOrFlat = (mCurrentFrameType == "L" || mCurrentFrameType == "F");
    bool needsFocus    = false;

    if (isLightOrFlat && getBool("focusparameters", "autofocusonfilterchange"))
    {
        // mHasFocusedOnce is false until the first focus of this module's
        // lifetime (not reset on sequence start) — that undetermined state
        // always counts as a filter change, so the first line of a sequence
        // gets focused too, without a separate "focus at start" option.
        if (!mHasFocusedOnce || mLastFocusedFilter != mCurrentFilter)
            needsFocus = true;
    }

    // DoFocus: SequenceCtrl → WaitFocus  AND  FocusCtrl.FocusIdle → Focusing
    // SkipFocus: SequenceCtrl → GuideGate, FocusCtrl unchanged
    pMachine->submitEvent(needsFocus ? "DoFocus" : "SkipFocus");
}

void Sequencer::SMGuideGate()
{
    //qDebug() << "SMGuideGate";

    if (!getBool("guideparameters", "useguiding"))
    {
        pMachine->submitEvent("SkipGuide");
        return;
    }

    if (mGuiderActive && !mGuiderWasSuspended)
    {
        // Guider already running (e.g. between shots on the same line)
        pMachine->submitEvent("SkipGuide");
        return;
    }

    mGuiderWasSuspended = false;
    // StartGuide: SequenceCtrl → WaitGuide  AND  GuideCtrl.GuideIdle → GuideStarting
    pMachine->submitEvent("StartGuide");
}

void Sequencer::SMDitherGate()
{
    //qDebug() << "SMDitherGate";
    int ditherevery = getInt("guideparameters", "ditherevery");

    if (ditherevery > 0 && mShotsSinceDither >= ditherevery && mGuiderActive)
    {
        // DoDither: SequenceCtrl → WaitDither  AND  GuideCtrl.GuideIdle → Dithering
        pMachine->submitEvent("DoDither");
    }
    else
    {
        pMachine->submitEvent("SkipDither");
    }
}

void Sequencer::SMWaitSettle()
{
    //qDebug() << "SMWaitSettle";
    int settleTime = getInt("guideparameters", "guidingsettletime");

    if (settleTime <= 0)
    {
        pMachine->submitEvent("SettleDone");
        return;
    }

    logInfo("Waiting %1 s for guiding to settle", {settleTime});
    mSettleTimer->start(settleTime * 1000);
}

void Sequencer::SMExposing()
{
    //qDebug() << "SMExposing";
    mMaxRMSDuringExposure = 0.0;

    double exp    = getFloat("livesequence", "exposure");
    int    gain   = getInt("parameters", "gain");
    int    offset = getInt("parameters", "offset");
    if (getString("devices", "camera") == "CCD Simulator")
    {
        sendModNewNumber(getString("devices", "camera"), "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 1 );
    }
    requestCapture(getString("devices", "camera"), exp, gain, offset);

    // Whole-sequence progress, in shots: this exposure is shot (mShotsCompleted+1) of mTotalShots
    if (mTotalShots > 0)
    {
        int current = mShotsCompleted + 1;
        getEltPrg("progress", "global")->setPrgValue(100.0 * current / mTotalShots, false);
        getEltPrg("progress", "global")->setDynLabel(
            QString::number(current) + "/" + QString::number(mTotalShots), true);
    }
}

void Sequencer::SMEvalShot()
{
    //qDebug() << "SMEvalShot";
    mShotCount--;
    mShotsSinceDither++;
    mShotsCompleted++;
    mSecondsCompleted += getFloat("livesequence", "exposure");
    updateLiveSequenceProgress();

    if (mShotCount > 0)
    {
        double hfrThreshold = getFloat("focusparameters", "hfrthreshold");
        if (hfrThreshold > 0.0 && mLastHFR > hfrThreshold)
        {
            logInfo("HFR %1 exceeds threshold %2 — refocusing",
            {QString::number(mLastHFR, 'f', 2), QString::number(hfrThreshold, 'f', 2)});
            // DoFocus: SequenceCtrl.EvalShot → WaitFocus  AND  FocusCtrl.FocusIdle → Focusing
            pMachine->submitEvent("DoFocus");
            return;
        }

        pMachine->submitEvent("NextShot");
    }
    else
    {
        pMachine->submitEvent("LineDone");
    }
}

// =============================================================================
// FocusCtrl entry points
// =============================================================================

void Sequencer::SMFocusing()
{
    //qDebug() << "SMFocusing";
    logInfo("Requesting autofocus from %1", {getString("slaves", "focusmodule")});
    mFocusTimer.start();
    bumpLiveSequenceCounter("focuscount");

    // Per-filter override (focusprofiles grid) takes priority over the
    // single fallback profile (parameters.focusprofile). findGridRow()
    // (libs/basemodule.cpp) now reads the key column through whichever
    // getter matches its real type, so this generic lookup is safe again
    // for "filter" being an int column (was a hand-written loop here
    // before that fix, issue #240).
    QString focusProfile = getGridString("focusprofiles", "profile", "filter", QString::number(mCurrentFilterIndex));
    if (focusProfile.isEmpty())
        focusProfile = getString("focusparameters", "focusprofile");

    if (!focusProfile.isEmpty())
    {
        logInfo("Requesting focus profile %1 on %2", {focusProfile, getString("slaves", "focusmodule")});
        otherModuleRequestProfileLoad(getString("slaves", "focusmodule"), focusProfile);
    }

    if (mGuiderActive && getBool("focusparameters", "suspendguidingduringfocus"))
    {
        logInfo("Suspending guiding on %1", {getString("slaves", "guidermodule")});
        mGuiderWasSuspended = true;
        mGuiderActive       = false;
        otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "abortguider", true);
        // GuideCtrl stays in GuideIdle — GuiderLost is not submitted for an intentional abort
    }

    otherModuleSetValue(getString("slaves", "focusmodule"), "actions", "autofocus", true);
    // FocusDone submitted by onOtherModuleEvent when focus module signals Ok/"ready"
}

// =============================================================================
// GuideCtrl entry points
// =============================================================================

void Sequencer::SMGuideStarting()
{
    //qDebug() << "SMGuideStarting";
    logInfo("Starting guider on %1", {getString("slaves", "guidermodule")});
    mGuideStartTimer.start();
    otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "guide", true);
    // GuideDone submitted by onOtherModuleEvent when guider signals Busy/"guiding"
}

void Sequencer::SMDithering()
{
    //qDebug() << "SMDithering";
    logInfo("Dithering (shot %1 since last dither)", {mShotsSinceDither});
    mShotsSinceDither = 0;
    bumpLiveSequenceCounter("dithercount");
    otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "dither", true);
    // DitherDone submitted by onOtherModuleEvent when guider returns to Busy/"guiding"
}

void Sequencer::SMRecalibrating()
{
    //qDebug() << "SMRecalibrating";
    logInfo("RMS-triggered recalibration on %1", {getString("slaves", "guidermodule")});
    bumpLiveSequenceCounter("recalcount");
    otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "resetcalibration", true);
    otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "guide", true);
    // RecalDone submitted by onOtherModuleEvent when guider returns to Busy/"guiding"
}

// =============================================================================
// Terminal states
// =============================================================================

void Sequencer::SMDone()
{
    //qDebug() << "SMDone";
    logInfo("Sequence completed");
    mElapsedTimer->stop();
    getProperty("actions")->setState(OST::Ok, true);
    getEltBool("actions", "startsequence")->setValue(false, true);
    getEltBool("actions", "abortsequence")->setValue(false, true);
    setStateEvent(OST::Ok, "ready", "sequencedone", "Sequence done");
    pMachine->stop();
}

void Sequencer::SMAborted()
{
    //qDebug() << "SMAborted";
    logInfo("Sequence aborted");
    mSettleTimer->stop();
    mElapsedTimer->stop();
    getProperty("actions")->setState(OST::Ok, true);
    getEltBool("actions", "startsequence")->setValue(false, true);
    getEltBool("actions", "abortsequence")->setValue(false, true);
    setStateEvent(OST::Ok, "ready", "abortdone", "Sequence aborted");
    pMachine->stop();
}

void Sequencer::SMError()
{
    //qDebug() << "SMError";
    logInfo("Sequence error - cause: %1", {mLastErrorCause.isEmpty() ? "unknown" : mLastErrorCause});
    mLastErrorCause.clear();
    mSettleTimer->stop();
    mElapsedTimer->stop();
    getProperty("actions")->setState(OST::Error, true);
    getEltBool("actions", "startsequence")->setValue(false, true);
    getEltBool("actions", "abortsequence")->setValue(false, true);
    setStateEvent(OST::Error, "error", "sequenceerror", "Sequence error");
    pMachine->stop();
}

// =============================================================================
// Timer
// =============================================================================

void Sequencer::onGuidingSettleTimeout()
{
    logInfo("Guiding settle complete");
    pMachine->submitEvent("SettleDone");
}

void Sequencer::onElapsedTimerTick()
{
    double elapsed = mSequenceStartTime.msecsTo(QDateTime::currentDateTime()) / 1000.0;
    getEltString("progress", "realelapsed")->setValue(formatDuration(elapsed), true);
}

// =============================================================================
// Inter-module events (focus / guider → sequencer)
// =============================================================================

void Sequencer::onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line)
{
    bool isFocuser = (!mod.isEmpty() && mod == getString("slaves", "focusmodule"));
    bool isGuider  = (!mod.isEmpty() && mod == getString("slaves", "guidermodule"));

    if (!isFocuser && !isGuider)
        return;

    // ── Focus module ──────────────────────────────────────────────────────────
    if (isFocuser && ev == OST::EvType::ea && prp == "signals")
    {
        QJsonObject o  = data.toJsonValue().toObject()["e"].toObject();
        int         s  = o["state"].toInt();
        QString     sd = o["statedescription"].toString();

        if (OST::IntToState(s) == OST::Ok && sd == "ready" && !pMachine->isActive("Focusing"))
        {
            // The focus module really did report done - but this sequencer
            // isn't waiting on it right now, so FocusDone is deliberately
            // NOT submitted (submitting an unexpected event would just be
            // ignored by the current scxml state anyway). Logged because a
            // silent drop here previously showed up 180s later as a bare,
            // unexplained "Sequence error" (FocusTimeout) - Gilles,
            // 2026-09-26: "comme si le sequenceur ne recevait pas le focus
            // done".
            logInfo("Focus module reported done, but sequencer isn't in Focusing (ignored)");
        }
        else if (OST::IntToState(s) == OST::Ok && sd == "ready" && pMachine->isActive("Focusing"))
        {
            logInfo("Focus completed");
            mHasFocusedOnce    = true;
            mLastFocusedFilter = mCurrentFilter;

            mFocusDurationSum += mFocusTimer.elapsed() / 1000.0;
            mFocusSamples++;
            getEltString("progress", "avgfocusduration")->setValue(
                formatDuration(mFocusDurationSum / mFocusSamples), true);

            // FocusDone: FocusCtrl.Focusing → FocusIdle  AND  SequenceCtrl.WaitFocus → GuideGate
            pMachine->submitEvent("FocusDone");
        }
        return;
    }

    // ── Guider module — state signals ─────────────────────────────────────────
    // Early-out when guiding integration is disabled: mGuiderActive stays false,
    // which silently disables dithering, RMS checks, and focus suspension.
    if (isGuider && !getBool("guideparameters", "useguiding"))
        return;

    if (isGuider && ev == OST::EvType::ea && prp == "signals")
    {
        QJsonObject o  = data.toJsonValue().toObject()["e"].toObject();
        int         s  = o["state"].toInt();
        QString     sd = o["statedescription"].toString();

        if (OST::IntToState(s) == OST::Busy && sd == "guiding")
        {
            mGuiderActive = true;

            // Only notify the SM when it is actively waiting for one of these
            if (pMachine->isActive("GuideStarting"))
            {
                logInfo("Guider resumed after focus");

                mGuideDurationSum += mGuideStartTimer.elapsed() / 1000.0;
                mGuideSamples++;
                getEltString("progress", "avgguideduration")->setValue(
                    formatDuration(mGuideDurationSum / mGuideSamples), true);

                // GuideDone: GuideCtrl.GuideStarting→GuideIdle AND SequenceCtrl.WaitGuide→DitherGate
                pMachine->submitEvent("GuideDone");
            }
            else if (pMachine->isActive("Dithering"))
            {
                logInfo("Dither complete");
                // DitherDone: GuideCtrl.Dithering→GuideIdle AND SequenceCtrl.WaitDither→WaitSettle
                pMachine->submitEvent("DitherDone");
            }
            else if (pMachine->isActive("Recalibrating"))
            {
                logInfo("Recalibration complete");
                // RecalDone: GuideCtrl.Recalibrating→GuideIdle AND SequenceCtrl.WaitRecal→WaitSettle
                pMachine->submitEvent("RecalDone");
            }
            // else: guider running externally — just track via mGuiderActive
        }
        else if (OST::IntToState(s) == OST::Ok && sd == "ready")
        {
            if (pMachine->isActive("WaitGuide"))
            {
                // Guider confirmed abort after RMS alert — reset calibration and restart
                logInfo("Guider stopped — resetting calibration and restarting");
                otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "resetcalibration", true);
                otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "guide", true);
            }
            else if (mGuiderActive && !mGuiderWasSuspended)
            {
                // Unexpected guider stop
                mGuiderActive = false;
                logInfo("Guider stopped unexpectedly");
                if (pMachine->isRunning())
                    pMachine->submitEvent("GuiderLost");
            }
            else
            {
                mGuiderActive = false;
            }
        }
        return;
    }

    // ── Guider module — live RMS ──────────────────────────────────────────────
    if (isGuider && ev == OST::EvType::ea && prp == "values" && elt == "rmsTotal")
    {
        // rmsTotal is ground truth: guiding is definitely running.
        mGuiderActive = true;

        // Confirm guider started (both from GuideStarting and from WaitGuide after RMSAlert).
        if (pMachine->isActive("GuideStarting") || pMachine->isActive("WaitGuide"))
        {
            pMachine->submitEvent("GuideDone");
            return;
        }

        if (pMachine->isActive("Exposing"))
        {
            QJsonObject o  = data.toJsonValue().toObject()["e"].toObject();
            double rms = o["rmsTotal"].toDouble();
            mMaxRMSDuringExposure = qMax(mMaxRMSDuringExposure, rms);

            double rmsThreshold = getFloat("guideparameters", "rmsthreshold");
            if (rmsThreshold > 0.0 && rms > rmsThreshold)
            {
                logInfo("RMS %1 px exceeded threshold %2 px — aborting exposure and guider",
                {QString::number(rms, 'f', 2), QString::number(rmsThreshold, 'f', 2)});
                mMaxRMSDuringExposure = 0.0;
                mGuiderActive = false;
                sendModNewSwitch(getString("devices", "camera"), "CCD_ABORT_EXPOSURE", "ABORT", ISS_ON);
                otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "abortguider", true);
                pMachine->submitEvent("RMSAlert");
            }
        }
        return;
    }
}

// =============================================================================
// INDI callbacks
// =============================================================================

void Sequencer::newBLOB(INDI::PropertyBlob pblob)
{
    if (QString(pblob.getDeviceName()) != getString("devices", "camera"))
        return;
    if (!pMachine->isRunning() || !pMachine->isActive("Exposing"))
        return;

    delete _image;
    _image = new fileio();
    _image->loadBlob(pblob, 64);
    stats = _image->getStats();

    QImage rawImage = _image->getRawQImage();
    QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
    im.setColorTable(rawImage.colorTable());
    int quality = getInt("parms", "jpegquality");
    atomicSaveJpeg(im, getWebroot() + "/" + getModuleName() + ".jpeg", quality);

    OST::ImgData dta  = _image->ImgStats();
    dta.mUrlJpeg      = getModuleName() + ".jpeg";
    getEltImg("image", "image")->setValue(dta, true);
    QString tt = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    QStorageInfo storage(mCurrentFolder);
    qint64 freeBytes  = storage.bytesFree();
    qint64 totalBytes = storage.bytesTotal();
    if (totalBytes > 0 && freeBytes * 100 / totalBytes < getMinFreePercent())
    {
        logWarning("Sequencer: disk too full (%1 MB free, threshold %2%) — FITS not saved: %3",
        {freeBytes / (1024 * 1024), getMinFreePercent(), mCurrentFolder});
    }
    else
    {
        // Skip empty segments (object name, filter) instead of leaving a bare "--"
        QStringList nameParts;
        if (!mObjectName.isEmpty()) nameParts << mObjectName;
        nameParts << mCurrentFrameType;
        if (!mCurrentFilter.isEmpty()) nameParts << mCurrentFilter;
        nameParts << tt;
        _image->saveAsFITSSimple(mCurrentFolder + "/" + nameParts.join("-") + ".FITS");
    }

    // newBLOB runs in the INDI thread — only do I/O here.
    // The state machine submits ExposureDone which triggers FindingStars on the main thread.
    if (pMachine->isRunning())
        pMachine->submitEvent("ExposureDone");
}

void Sequencer::SMFindStars()
{
    // Runs on the main thread (state machine entry callback).
    _solver.ResetSolver(stats, _image->getImageBuffer());
    connect(&_solver, &Solver::successSEP, this, &Sequencer::OnSucessSEP);
    _solver.FindStars(_solver.stellarSolverProfiles[0]);
}

void Sequencer::OnSucessSEP()
{
    disconnect(&_solver, &Solver::successSEP, this, &Sequencer::OnSucessSEP);

    OST::ImgData dta = getEltImg("image", "image")->value();
    double ech       = getSampling();
    dta.HFRavg       = _solver.HFRavg * ech;
    dta.starsCount   = _solver.stars.size();
    getEltImg("image", "image")->setValue(dta, true);

    mLastHFR = dta.HFRavg;
    setStateEvent(OST::Busy, "running", "imagehfr", "image HFR", dta.HFRavg);

    // Running average HFR for the current "livesequence" row (issue #241).
    if (mCurrentLine >= 0 && mCurrentLine < getProperty("livesequence")->getGrid().count())
    {
        mLineHFRSum += dta.HFRavg;
        mLineHFRSamples++;
        getEltFloat("livesequence", "avghfr")->setValue(mLineHFRSum / mLineHFRSamples, false);
        getProperty("livesequence")->updateLine(mCurrentLine);
    }

    pMachine->submitEvent("FindStarsDone");
}

void Sequencer::newProperty(INDI::Property property)
{
    if (property.getDeviceName() == getString("devices", "filter") &&
            QString(property.getName()) == "FILTER_NAME")
    {
        refreshFilterLov();
    }
}

void Sequencer::updateProperty(INDI::Property property)
{
    // Chain up: IndiModule::updateProperty() refreshes devices.*'s own LOV
    // on a CONNECTION change (issue #231) - overriding this method
    // entirely, as this one does, otherwise silently skips that. Kept
    // ahead of the early returns below.
    IndiModule::updateProperty(property);

    if (strcmp(property.getName(), "CCD1") == 0)
    {
        newBLOB(property);
        return;
    }

    if (property.getDeviceName() == getString("devices", "camera") &&
            property.getState() == IPS_ALERT)
    {
        logInfo("Camera alert — aborting sequence");
        emit cameraAlert();
        if (pMachine->isRunning())
            pMachine->submitEvent("CameraAlert");
        return;
    }

    if (property.getDeviceName() == getString("devices", "camera") &&
            QString(property.getName()) == "CCD_TEMPERATURE" &&
            property.getState() == IPS_OK &&
            pMachine->isActive("WaitCooling"))
    {
        logInfo("Camera temperature reached");
        pMachine->submitEvent("CoolingReady");
        return;
    }

    if (property.getDeviceName() == getString("devices", "filter") &&
            QString(property.getName()) == "FILTER_SLOT" &&
            property.getState() == IPS_OK &&
            pMachine->isActive("FilterStep"))
    {
        pMachine->submitEvent("FilterReady");
        return;
    }

    if (property.getDeviceName() == getString("devices", "camera") &&
            QString(property.getName()) == "CCD_EXPOSURE" &&
            pMachine->isActive("Exposing"))
    {
        newExp(property);
    }
}

void Sequencer::newExp(INDI::PropertyNumber exp)
{
    // Issue #237 - findWidgetByName() can return nullptr even on a
    // property matched by name, if the actual driver update doesn't
    // carry this widget (non-compliant/transient driver state) - same
    // class of bug already fixed in parkmanager.cpp (issue #227) and
    // navigator.cpp.
    auto *expW = exp.findWidgetByName("CCD_EXPOSURE_VALUE");
    if (!expW)
    {
        logWarning("CCD_EXPOSURE_VALUE widget missing, skipping exposure progress update");
        return;
    }
    double etot = getFloat("livesequence", "exposure");
    double ex   = expW->value;
    double elapsedInShot = etot - ex;
    getEltPrg("progress", "exposure")->setPrgValue(100.0 * elapsedInShot / etot, true);

    // Whole-sequence progress, in time: reuses the same CCD_EXPOSURE_VALUE
    // ticks as above, no extra polling — smooth within the current shot,
    // then mSecondsCompleted jumps at each shot's completion (SMEvalShot).
    if (mTotalEstimatedSeconds > 0.0)
    {
        double liveSeconds = mSecondsCompleted + elapsedInShot;
        getEltPrg("progress", "totaltime")->setPrgValue(100.0 * liveSeconds / mTotalEstimatedSeconds, false);
        getEltPrg("progress", "totaltime")->setDynLabel(
            formatDuration(liveSeconds) + " / " + formatDuration(mTotalEstimatedSeconds), true);

        double remaining = qMax(0.0, mTotalEstimatedSeconds - liveSeconds);
        getEltString("progress", "theoreticalremaining")->setValue(formatDuration(remaining), true);
    }
}

// =============================================================================
// Helpers
// =============================================================================

void Sequencer::recomputeSequenceTotals()
{
    int    totalShots   = 0;
    double totalSeconds = 0.0;
    for (int i = 0; i < getProperty("sequence")->getGrid().count(); i++)
    {
        getProperty("sequence")->fetchLine(i);
        int    c = getInt("sequence", "count");
        double e = getFloat("sequence", "exposure");
        totalShots   += c;
        totalSeconds += c * e;
    }
    mTotalShots            = totalShots;
    mTotalEstimatedSeconds = totalSeconds;

    getEltString("progress", "theoreticaltotal")->setValue(formatDuration(mTotalEstimatedSeconds), true);

    double remaining = qMax(0.0, mTotalEstimatedSeconds - mSecondsCompleted);
    getEltString("progress", "theoreticalremaining")->setValue(formatDuration(remaining), true);
}

// Snapshots the "sequence" config grid into "livesequence" (issue #241) -
// not saved to the profile, this is what's shown/updated while a run is
// in progress, so editing "sequence" mid-run never disturbs it. Each line
// starts its two progress columns at 0 - see updateLiveSequenceProgress()
// for how they advance.
void Sequencer::buildLiveSequence()
{
    getProperty("livesequence")->clearGrid();
    for (int i = 0; i < getProperty("sequence")->getGrid().count(); i++)
    {
        getProperty("sequence")->fetchLine(i);
        int    filter = getInt("sequence", "filter");
        double exp    = getFloat("sequence", "exposure");
        int    count  = getInt("sequence", "count");

        QVariantMap values;
        values["filter"]       = filter;
        values["exposure"]     = exp;
        values["count"]        = count;
        values["dithercount"]  = 0;
        values["focuscount"]   = 0;
        values["recalcount"]   = 0;
        values["avghfr"]       = 0.0;
        getProperty("livesequence")->newLine(values);

        // newLine() snapshots whatever the "progress*" elements currently
        // hold (leftover from the previous line) - set them to a clean
        // starting point, then re-sync this one row (same pattern as
        // planner.cpp's own prg-in-grid column).
        getEltPrg("livesequence", "progresscount")->setPrgValue(0, false);
        getEltPrg("livesequence", "progresscount")->setDynLabel("0/" + QString::number(count), false);
        getEltPrg("livesequence", "progressduration")->setPrgValue(0, false);
        getEltPrg("livesequence", "progressduration")->setDynLabel(formatDuration(0) + "/" + formatDuration(count * exp), false);
        getProperty("livesequence")->updateLine(getProperty("livesequence")->getGrid().count() - 1);
    }
}

// Advances the current line's two progress columns in "livesequence"
// (issue #241) - called once per completed shot (SMEvalShot()). Reads
// "sequence"'s own live count/exposure, which still reflect mCurrentLine
// (fetchLine() for the next line only happens in SMInitLine(), once this
// line is entirely done).
void Sequencer::updateLiveSequenceProgress()
{
    if (mCurrentLine < 0 || mCurrentLine >= getProperty("livesequence")->getGrid().count())
        return;

    int    lineCount = getInt("livesequence", "count");
    double lineExp   = getFloat("livesequence", "exposure");
    int    done      = lineCount - mShotCount;
    double doneSeconds  = done * lineExp;
    double totalSeconds = lineCount * lineExp;

    getEltPrg("livesequence", "progresscount")->setPrgValue(lineCount > 0 ? 100.0 * done / lineCount : 0.0, false);
    getEltPrg("livesequence", "progresscount")->setDynLabel(QString::number(done) + "/" + QString::number(lineCount), false);
    getEltPrg("livesequence", "progressduration")->setPrgValue(totalSeconds > 0 ? 100.0 * doneSeconds / totalSeconds : 0.0, false);
    getEltPrg("livesequence", "progressduration")->setDynLabel(formatDuration(doneSeconds) + "/" + formatDuration(totalSeconds), false);
    getProperty("livesequence")->updateLine(mCurrentLine);
}

// Bumps one of the three per-line activity counters on the current
// "livesequence" row (issue #241) - called from the three state entry
// hooks below, each firing exactly once per actual occurrence of that
// phase (SMFocusing() also covers the very first focus, before any
// shot - it is still "this line"'s own FocusGate decision, see the
// "always counts as a filter change" comment on the first line above).
void Sequencer::bumpLiveSequenceCounter(const QString &column)
{
    if (mCurrentLine < 0 || mCurrentLine >= getProperty("livesequence")->getGrid().count())
        return;

    int count = getGridInt("livesequence", column, mCurrentLine) + 1;
    getEltInt("livesequence", column)->setValue(count, false);
    getProperty("livesequence")->updateLine(mCurrentLine);
}

QVariantMap Sequencer::onModuleQuery(const QString &queryName, const QVariantMap &params)
{
    if (queryName == "profileduration")
    {
        // Peek at a stored profile's "sequence" grid without loading it,
        // so this can be asked about any profile regardless of what is
        // currently active (e.g. by the planner, before starting a line).
        double totalSeconds = 0.0;
        QJsonObject obj;
        if (getDbProfile(getClassName(), params["profile"].toString(), obj))
        {
            QJsonObject seqProp = obj["p"].toObject()["sequence"].toObject();
            QJsonArray  headers = seqProp["gridheaders"].toArray();
            QJsonArray  grid    = seqProp["grid"].toArray();

            int ci = -1, ei = -1;
            for (int i = 0; i < headers.size(); i++)
            {
                if (headers[i].toString() == "count")    ci = i;
                if (headers[i].toString() == "exposure") ei = i;
            }

            if (ci != -1 && ei != -1)
            {
                for (const QJsonValue &vline : grid)
                {
                    QJsonArray line = vline.toArray();
                    totalSeconds += line[ci].toInt() * line[ei].toDouble();
                }
            }
        }
        return { {"seconds", totalSeconds} };
    }
    return Basemodule::onModuleQuery(queryName, params);
}

void Sequencer::setupOutputFolder()
{
    QDir dir;
    QString base = getWebroot() + "/" + getModuleName();
    if (!mObjectName.isEmpty())
        base += "/" + mObjectName;

    if (mCurrentFrameType == "L")
    {
        sendModNewSwitch(getString("devices", "camera"), "CCD_FRAME_TYPE", "FRAME_LIGHT", ISS_ON);
        mCurrentFolder = base + "/LIGHT";
        if (!mCurrentFilter.isEmpty())
            mCurrentFolder += "/" + mCurrentFilter;
        dir.mkpath(mCurrentFolder);
    }
    else if (mCurrentFrameType == "F")
    {
        sendModNewSwitch(getString("devices", "camera"), "CCD_FRAME_TYPE", "FRAME_FLAT", ISS_ON);
        mCurrentFolder = base + "/FLAT";
        if (!mCurrentFilter.isEmpty())
            mCurrentFolder += "/" + mCurrentFilter;
        dir.mkpath(mCurrentFolder);
    }
    else if (mCurrentFrameType == "B")
    {
        sendModNewSwitch(getString("devices", "camera"), "CCD_FRAME_TYPE", "FRAME_BIAS", ISS_ON);
        mCurrentFolder = base + "/BIAS";
        dir.mkpath(mCurrentFolder);
    }
    else if (mCurrentFrameType == "D")
    {
        sendModNewSwitch(getString("devices", "camera"), "CCD_FRAME_TYPE", "FRAME_DARK", ISS_ON);
        mCurrentFolder = base + "/DARK";
        dir.mkpath(mCurrentFolder);
    }
}

void Sequencer::refreshFilterLov()
{
    // Always clear first: a filter device that was unassigned or went
    // invalid must not leave stale filter names in the LOV (mCurrentFilter
    // must fall back to empty, not resolve to a phantom stale entry).
    // Shared by "sequence" (per-line filter), "focusprofiles" (per-filter
    // focus profile override) and "livesequence" (read-only run-time copy
    // of "sequence", issue #241) — same physical filter wheel, same LOV.
    getEltInt("sequence", "filter")->lovClear();
    getEltInt("focusprofiles", "filter")->lovClear();
    getEltInt("livesequence", "filter")->lovClear();

    INDI::BaseDevice dp = getDevice(getString("devices", "filter").toStdString().c_str());
    if (!dp.isValid())
        return;

    INDI::PropertyText txt = dp.getText("FILTER_NAME");
    if (!txt.isValid())
        return;

    for (unsigned int i = 0; i < txt.count(); i++)
    {
        // INDI FILTER_SLOT is 1-based: slot i+1 is named by FILTER_NAME's
        // i-th (0-based) text element.
        getEltInt("sequence", "filter")->lovAdd((int)(i + 1), txt[i].getText());
        getEltInt("focusprofiles", "filter")->lovAdd((int)(i + 1), txt[i].getText());
        getEltInt("livesequence", "filter")->lovAdd((int)(i + 1), txt[i].getText());
    }
}

QString Sequencer::formatDuration(double seconds)
{
    int total = qRound(seconds);
    int h     = total / 3600;
    int m     = (total % 3600) / 60;
    int s     = total % 60;

    if (h > 0)
        return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    return QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));
}
