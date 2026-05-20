#include "sequencer.h"
#include <QPainter>
#include <QTimer>
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
    setMetadata("template",       "sequence");

    loadOstPropertiesFromFile(":sequencer.json");

    giveMeADevice("camera", "Camera",       INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("filter", "Filter wheel", INDI::BaseDevice::FILTER_INTERFACE);
    defineMeAsSequencer();
    refreshFilterLov();

    mSettleTimer = new QTimer(this);
    mSettleTimer->setSingleShot(true);
    connect(mSettleTimer, &QTimer::timeout, this, &Sequencer::onGuidingSettleTimeout);

    pMachine = QScxmlStateMachine::fromFile(":sequencer.scxml");

    // SequenceCtrl
    pMachine->connectToState("InitLine",      QScxmlStateMachine::onEntry(this, &Sequencer::SMInitLine));
    pMachine->connectToState("FilterStep",    QScxmlStateMachine::onEntry(this, &Sequencer::SMFilterStep));
    pMachine->connectToState("FocusGate",     QScxmlStateMachine::onEntry(this, &Sequencer::SMFocusGate));
    pMachine->connectToState("GuideGate",     QScxmlStateMachine::onEntry(this, &Sequencer::SMGuideGate));
    pMachine->connectToState("DitherGate",    QScxmlStateMachine::onEntry(this, &Sequencer::SMDitherGate));
    pMachine->connectToState("WaitSettle",    QScxmlStateMachine::onEntry(this, &Sequencer::SMWaitSettle));
    pMachine->connectToState("Exposing",      QScxmlStateMachine::onEntry(this, &Sequencer::SMExposing));
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

            for (int i = 0; i < getProperty("sequence")->getGrid().count(); i++)
            {
                getProperty("sequence")->fetchLine(i);
                getEltPrg("sequence", "progress")->setDynLabel("Queued", false);
                getEltPrg("sequence", "progress")->setPrgValue(0, false);
                getProperty("sequence")->updateLine(i);
            }

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
        }
    }

    if (event.prpkey == "devices")
        refreshFilterLov();
}

// =============================================================================
// SequenceCtrl entry points
// =============================================================================

void Sequencer::SMInitLine()
{
    //qDebug() << "SMInitLine";
    mCurrentLine++;

    if (mCurrentLine >= getProperty("sequence")->getGrid().count())
    {
        // SequenceDoneAll bubbles up through Running to Active → Done.
        // We avoid done.state.Running — Qt SCXML re-enters the parallel state instead of exiting it.
        pMachine->submitEvent("SequenceDoneAll");
        return;
    }

    setStateEvent(OST::Busy, "running", "startline", "Starting line");

    getProperty("sequence")->fetchLine(mCurrentLine);
    mShotCount            = getInt("sequence", "count");
    int filterIndex       = getInt("sequence", "filter");
    const auto &lov       = getEltInt("sequence", "filter")->getLov();
    mCurrentFilter        = lov.contains(filterIndex) ? lov[filterIndex] : QString();
    mCurrentFrameType     = getString("sequence", "frametype");
    mLastHFR              = 0.0;
    mMaxRMSDuringExposure = 0.0;

    pMachine->submitEvent("LineLoaded");
}

void Sequencer::SMFilterStep()
{
    //qDebug() << "SMFilterStep";
    int filterIndex = getInt("sequence", "filter");
    mFilterChanged  = !mPreviousFilter.isEmpty() && (mPreviousFilter != mCurrentFilter);
    mPreviousFilter = mCurrentFilter;

    setupOutputFolder();

    if (mFilterChanged)
        logInfo("Changing filter from %1 to %2", {mPreviousFilter, mCurrentFilter});

    QString filterDevice = getString("devices", "filter");
    if (!getDevice(filterDevice.toStdString().c_str()).isValid())
    {
        pMachine->submitEvent("FilterReady");
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

    if (isLightOrFlat)
    {
        if (mCurrentLine == 0 && getBool("parameters", "autofocusatstart"))
            needsFocus = true;
        else if (mFilterChanged && getBool("parameters", "autofocusonfilterchange"))
            needsFocus = true;
    }

    // DoFocus: SequenceCtrl → WaitFocus  AND  FocusCtrl.FocusIdle → Focusing
    // SkipFocus: SequenceCtrl → GuideGate, FocusCtrl unchanged
    pMachine->submitEvent(needsFocus ? "DoFocus" : "SkipFocus");
}

void Sequencer::SMGuideGate()
{
    //qDebug() << "SMGuideGate";

    if (!getBool("parameters", "useguiding"))
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
    int ditherevery = getInt("parameters", "ditherevery");

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
    int settleTime = getInt("parameters", "guidingsettletime");

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

    double exp    = getFloat("sequence", "exposure");
    int    gain   = getInt("sequence", "gain");
    int    offset = getInt("sequence", "offset");
    requestCapture(getString("devices", "camera"), exp, gain, offset);

    int total = getInt("sequence", "count");
    int done  = total - mShotCount + 1;
    getEltPrg("sequence", "progress")->setPrgValue(100.0 * done / total, false);
    getEltPrg("sequence", "progress")->setDynLabel(QString::number(done) + "/" + QString::number(total), false);
    getProperty("sequence")->updateLine(mCurrentLine);

    int lineTotal = getProperty("sequence")->getGrid().size();
    getEltPrg("progress", "global")->setPrgValue(100.0 * (mCurrentLine + 1) / lineTotal, false);
    getEltPrg("progress", "global")->setDynLabel(
        QString::number(mCurrentLine + 1) + "/" + QString::number(lineTotal), true);
}

void Sequencer::SMEvalShot()
{
    //qDebug() << "SMEvalShot";
    mShotCount--;
    mShotsSinceDither++;

    if (mShotCount > 0)
    {
        double hfrThreshold = getFloat("parameters", "hfrthreshold");
        if (hfrThreshold > 0.0 && mLastHFR > hfrThreshold)
        {
            logInfo("HFR %1 exceeds threshold %2 — refocusing",
            {QString::number(mLastHFR, 'f', 2), QString::number(hfrThreshold, 'f', 2)});
            // DoFocus: SequenceCtrl.EvalShot → WaitFocus  AND  FocusCtrl.FocusIdle → Focusing
            pMachine->submitEvent("DoFocus");
            return;
        }

        double rmsThreshold = getFloat("parameters", "rmsthreshold");
        if (rmsThreshold > 0.0 && mMaxRMSDuringExposure > rmsThreshold && mGuiderActive)
        {
            logInfo("Peak RMS %1 px exceeded threshold %2 px — recalibrating",
            {QString::number(mMaxRMSDuringExposure, 'f', 2), QString::number(rmsThreshold, 'f', 2)});
            // DoRecal: SequenceCtrl.EvalShot → WaitRecal  AND  GuideCtrl.GuideActive → Recalibrating
            pMachine->submitEvent("DoRecal");
            return;
        }

        pMachine->submitEvent("NextShot");
    }
    else
    {
        getEltPrg("sequence", "progress")->setDynLabel("Done", false);
        getEltPrg("sequence", "progress")->setPrgValue(100, false);
        getProperty("sequence")->updateLine(mCurrentLine);
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

    if (mGuiderActive && getBool("parameters", "suspendguidingduringfocus"))
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
    otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "guide", true);
    // GuideDone submitted by onOtherModuleEvent when guider signals Busy/"guiding"
}

void Sequencer::SMDithering()
{
    //qDebug() << "SMDithering";
    logInfo("Dithering (shot %1 since last dither)", {mShotsSinceDither});
    mShotsSinceDither = 0;
    otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "dither", true);
    // DitherDone submitted by onOtherModuleEvent when guider returns to Busy/"guiding"
}

void Sequencer::SMRecalibrating()
{
    //qDebug() << "SMRecalibrating";
    logInfo("RMS-triggered recalibration on %1", {getString("slaves", "guidermodule")});
    otherModuleSetValue(getString("slaves", "guidermodule"), "actions", "calguide", true);
    // RecalDone submitted by onOtherModuleEvent when guider returns to Busy/"guiding"
}

// =============================================================================
// Terminal states
// =============================================================================

void Sequencer::SMDone()
{
    //qDebug() << "SMDone";
    logInfo("Sequence completed");
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
    getProperty("actions")->setState(OST::Ok, true);
    getEltBool("actions", "startsequence")->setValue(false, true);
    getEltBool("actions", "abortsequence")->setValue(false, true);
    setStateEvent(OST::Ok, "ready", "abortdone", "Sequence aborted");
    pMachine->stop();
}

void Sequencer::SMError()
{
    //qDebug() << "SMError";
    logInfo("Sequence error");
    mSettleTimer->stop();
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

        if (OST::IntToState(s) == OST::Ok && sd == "ready" && pMachine->isActive("Focusing"))
        {
            logInfo("Focus completed");
            // FocusDone: FocusCtrl.Focusing → FocusIdle  AND  SequenceCtrl.WaitFocus → GuideGate
            pMachine->submitEvent("FocusDone");
        }
        return;
    }

    // ── Guider module — state signals ─────────────────────────────────────────
    // Early-out when guiding integration is disabled: mGuiderActive stays false,
    // which silently disables dithering, RMS checks, and focus suspension.
    if (isGuider && !getBool("parameters", "useguiding"))
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
        else if (OST::IntToState(s) == OST::Ok && mGuiderActive && !mGuiderWasSuspended)
        {
            // Unexpected guider stop (not caused by us for focus)
            mGuiderActive = false;
            logInfo("Guider stopped unexpectedly");
            if (pMachine->isRunning())
                pMachine->submitEvent("GuiderLost");
        }
        return;
    }

    // ── Guider module — live RMS (tracked during exposure only) ──────────────
    if (isGuider && ev == OST::EvType::ea && prp == "values" && elt == "rmsTotal")
    {
        // rmsTotal is ground truth: guiding is definitely running.
        mGuiderActive = true;
        // If we were waiting for the guider to start, confirm it now.
        if (pMachine->isActive("GuideStarting"))
            pMachine->submitEvent("GuideDone");
        if (pMachine->isActive("Exposing"))
            mMaxRMSDuringExposure = qMax(mMaxRMSDuringExposure, data.toDouble());
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
    im.save(getWebroot() + "/" + getModuleName() + ".jpeg", "JPG", 100);

    QString tt = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    _image->saveAsFITSSimple(
        mCurrentFolder + "/" + mObjectName + "-" + mCurrentFrameType + "-" + mCurrentFilter + "-" + tt + ".FITS");

    OST::ImgData dta = _image->ImgStats();
    mLastHFR         = dta.HFRavg;
    dta.mUrlJpeg     = getModuleName() + ".jpeg";
    getEltImg("image", "image")->setValue(dta, true);

    pMachine->submitEvent("ExposureDone");
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
    double etot = getFloat("sequence", "exposure");
    double ex   = exp.findWidgetByName("CCD_EXPOSURE_VALUE")->value;
    getEltPrg("progress", "exposure")->setPrgValue(100.0 * (etot - ex) / etot, true);
}

// =============================================================================
// Helpers
// =============================================================================

void Sequencer::setupOutputFolder()
{
    QDir dir;
    QString base = getWebroot() + "/" + getModuleName() + "/" + mObjectName;

    if (mCurrentFrameType == "L")
    {
        sendModNewSwitch(getString("devices", "camera"), "CCD_FRAME_TYPE", "FRAME_LIGHT", ISS_ON);
        mCurrentFolder = base + "/LIGHT/" + mCurrentFilter;
        dir.mkpath(mCurrentFolder);
    }
    else if (mCurrentFrameType == "F")
    {
        sendModNewSwitch(getString("devices", "camera"), "CCD_FRAME_TYPE", "FRAME_FLAT", ISS_ON);
        mCurrentFolder = base + "/FLAT/" + mCurrentFilter;
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
    INDI::BaseDevice dp = getDevice(getString("devices", "filter").toStdString().c_str());
    if (!dp.isValid())
        return;

    INDI::PropertyText txt = dp.getText("FILTER_NAME");
    if (!txt.isValid())
        return;

    getEltInt("sequence", "filter")->lovClear();
    for (unsigned int i = 0; i < txt.count(); i++)
        getEltInt("sequence", "filter")->lovAdd(i + 1, txt[i].getText());
}
