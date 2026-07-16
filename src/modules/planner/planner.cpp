#include "planner.h"
#include "version.cc"
#include <libnova/julian_day.h>
#include <libnova/transform.h>
#include <libnova/lunar.h>
#include <libnova/angular_separation.h>

/**
 * @brief Required export function for dynamic module loading
 * This function is called by the controller when loading the module
 */
Planner* initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Planner* module = new Planner(name, label, profile, availableModuleLibs);
    return module;
}

Planner::Planner(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs),
      mIsRunning(false)
{
    Q_INIT_RESOURCE(planner);
    giveMeAnActions();
    giveMeALocation();
    loadOstPropertiesFromFile(":planner.json");
    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("description", "Planner module");
    setMetadata("template", "planner");


    // Initialize properties and request INDI devices
    initProperties();

    logInfo("Planner module initialized");
}

Planner::~Planner()
{
}

void Planner::initProperties()
{

    // Request INDI devices
    giveMeADevice("gps", "GPS", INDI::BaseDevice::GPS_INTERFACE);

    // Add start and stop buttons
    OST::PropertyMulti* pm = getProperty("actions");
    OST::ElementBool* b = new OST::ElementBool("start", "Start", "010", "");
    b->setValue(false, false);
    pm->addElt( b);
    b = new OST::ElementBool("stop", "Stop", "020", "");
    b->setValue(false, false);
    pm->addElt( b);


}

void Planner::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        //getEltBool(event.prpkey, event.eltkey)->setValue(false, false);
        if (event.eltkey == "start")
        {
            //getProperty(event.prpkey)->setState(OST::Busy, true);
            startPlanner();
        }
        if (event.eltkey == "stop")
        {
            //getProperty(event.prpkey)->setState(OST::Ok, true);
            abortPlanner();
        }
    }
    if (event.prpkey == "planning")
    {
        //get elements data from event data
        QJsonObject e =
            event.data["m"].toObject()[this->getModuleName()].toObject()["p"].toObject()[event.prpkey].toObject()["e"].toObject();

        if (event.ev == OST::ExtEvType::GC)
        {
            getProperty(event.prpkey)->newLine(e.toVariantMap());
            getEltPrg(event.prpkey, "progress")->setPrgValue(0, false);
            getEltPrg(event.prpkey, "progress")->setDynLabel("Queued", false);
            getProperty(event.prpkey)->updateLine(getProperty(event.prpkey)->getGrid().count() - 1);
        }
        if (event.ev == OST::ExtEvType::GU)
        {
            getStore()[event.prpkey]->updateLine(event.line, e.toVariantMap());
            getEltPrg(event.prpkey, "progress")->setPrgValue(0, false);
            getEltPrg(event.prpkey, "progress")->setDynLabel("Queued", false);
            getStore()[event.prpkey]->updateLine(event.line);
        }
        if (event.ev == OST::ExtEvType::GF)
        {
            getStore()[event.prpkey]->fetchLine(event.line);
        }
        if (event.ev == OST::ExtEvType::GD)
        {
            getStore()[event.prpkey]->deleteLine(event.line);
        }
        if (event.ev == OST::ExtEvType::GH)
        {
            getStore()[event.prpkey]->swapLines(event.line, event.line - 1);
        }
        if (event.ev == OST::ExtEvType::GB)
        {
            getStore()[event.prpkey]->swapLines(event.line, event.line + 1);
        }
        if (event.ev == OST::ExtEvType::GR)
        {
            getStore()[event.prpkey]->clearGrid();
        }
    }
}

void Planner::startPlanner()
{
    if (mIsRunning)
    {
        logWarning("Operation already running");
        return;
    }
    getEltBool("actions", "start")->setValue(true, false);
    getEltBool("actions", "stop")->setValue(false, true);
    // Check INDI server connection
    if (!isServerConnected())
    {
        logError("INDI server not connected");
        getProperty("actions")->setState(OST::Error, true);
        abortPlanner();
        return;
    }

    // Check and connect devices
    QString g = getString("devices", "gps");
    if (g.isEmpty() || g == "--")
    {
        logError("No GPS selected");
        getProperty("actions")->setState(OST::Error, true);
        abortPlanner();
        return;
    }

    // Connect device if not connected
    connectDevice(g);

    // Check slaves modules
    g = getString("slaves", "sequencemodule");
    if (g.isEmpty() || g == "--")
    {
        logError("No Sequence module selected");
        getProperty("actions")->setState(OST::Error, true);
        abortPlanner();
        return;
    }
    g = getString("slaves", "navigatormodule");
    if (g.isEmpty() || g == "--")
    {
        logError("No Sequence module selected");
        getProperty("actions")->setState(OST::Error, true);
        abortPlanner();
        return;
    }
    int lineCount = getProperty("planning")->getGrid().count();

    if (lineCount == 0)
    {
        logError("Planning grid is empty");
        getProperty("actions")->setState(OST::Error, true);
        abortPlanner();
        return;
    }


    mIsRunning = true;

    //getProperty("actions")->setState(OST::Busy, true);
    logInfo("Starting planner...");

    // Disable properties during operation (optional)
    //getProperty("parms")->disable();
    getProperty("devices")->disable();

    // Update progress
    getEltPrg("progress", "global")->setPrgValue(0, true);
    getEltPrg("progress", "global")->setDynLabel("Step 0/" + QString::number(lineCount), true);
    for (int i = 0; i < getProperty("planning")->getGrid().count(); i++)
    {
        getProperty("planning")->fetchLine(i);
        getEltPrg("planning", "progress")->setDynLabel("Queued", false);
        getEltPrg("planning", "progress")->setPrgValue(0, false);
        getProperty("planning")->updateLine(i);
    }

    mCurrentLine = 0;
    mSkipCounts.clear();
    startLine();

}

void Planner::abortPlanner()
{
    getEltBool("actions", "start")->setValue(false, false);
    getEltBool("actions", "stop")->setValue(true, true);

    otherModuleSetValue(getString("slaves", "sequencemodule"), "actions", "abortsequence", true);
    otherModuleSetValue(getString("slaves", "navigatormodule"), "actions", "abortnavigator", true);

    if (!mIsRunning)
        return;

    mIsRunning = false;
    mWaitingSequence = false;
    mWaitingNavigator = false;
    mWaitingDuration = false;

    logInfo("Operation aborted");

    // Re-enable properties
    //getProperty("parms")->enable();
    getProperty("devices")->enable();

    // Reset progress
    getEltPrg("progress", "global")->setPrgValue(0, true);
    getEltPrg("progress", "global")->setDynLabel("Aborted", true);

    getProperty("actions")->setState(OST::Ok, true);
}

void Planner::onOperationComplete()
{
    logInfo("Operation completed successfully");

    mIsRunning = false;

    // Re-enable properties
    //getProperty("parms")->enable();
    getProperty("devices")->enable();

    // Update progress to 100%
    getEltPrg("progress", "global")->setPrgValue(100, true);
    getEltPrg("progress", "global")->setDynLabel("Completed", true);

    getProperty("actions")->setState(OST::Ok, true);
}

void Planner::sequenceComplete()
{
    mWaitingSequence = false;
    mWaitingNavigator = false;


    getProperty("planning")->fetchLine(mCurrentLine);
    getEltPrg("planning", "progress")->setDynLabel("Finished", false);
    getEltPrg("planning", "progress")->setPrgValue(100, false);
    getProperty("planning")->updateLine(mCurrentLine);

    advanceToNextLine();
}
void Planner::navigatorComplete()
{
    mWaitingNavigator = false;

    logInfo("Navigator went to target %1, starting sequence with profile %2", {getString("planning", "object"), getString("planning", "profile")});

    getProperty("planning")->fetchLine(mCurrentLine);
    getEltPrg("planning", "progress")->setDynLabel("Shooting", false);
    getEltPrg("planning", "progress")->setPrgValue(0, false);
    getProperty("planning")->updateLine(mCurrentLine);

    // Ask sequence to start
    QVariantMap eltData;
    eltData["startsequence"] = true;
    QVariantMap propData;
    propData["elements"] = eltData;
    QVariantMap eventData;
    eventData["actions"] = propData;
    otherModuleSetValue( getString("slaves", "sequencemodule"), "actions", "startsequence", true);

    mWaitingSequence = true;

}
void Planner::startLine()
{
    // update global status
    int pg =  100 * mCurrentLine / getProperty("planning")->getGrid().size();
    getEltPrg("progress", "global")->setPrgValue(pg, true);
    getEltPrg("progress", "global")->setDynLabel("Step 0/" + QString::number(getProperty("planning")->getGrid().size()), true);

    // update line status
    getProperty("planning")->fetchLine(mCurrentLine);
    getEltPrg("planning", "progress")->setDynLabel("Checking visibility", false);
    getEltPrg("planning", "progress")->setPrgValue(0, false);
    getProperty("planning")->updateLine(mCurrentLine);

    logInfo("Checking visibility for target %1", {getString("planning", "object")});

    mWaitingSequence  = false;
    mWaitingNavigator = false;
    mWaitingDuration   = true;

    // Ask the sequencer how long the requested profile would theoretically take,
    // so the visibility check covers the whole sequence, not just this instant.
    otherModuleQuery(getString("slaves", "sequencemodule"), "profileduration",
    {{"profile", getString("planning", "profile")}});
}

void Planner::proceedToSlew()
{
    getProperty("planning")->fetchLine(mCurrentLine);
    getEltPrg("planning", "progress")->setDynLabel("Slewing", false);
    getEltPrg("planning", "progress")->setPrgValue(0, false);
    getProperty("planning")->updateLine(mCurrentLine);

    logInfo("Navigator is slewing to target %1", {getString("planning", "object")});

    mWaitingSequence = false;
    mWaitingNavigator = true;

    otherModuleRequestPropertyDump(getString("slaves", "navigatormodule"), "actions");

    // Set navigator's target
    otherModuleSetValue( getString("slaves", "navigatormodule"), "target", "targetra", getFloat("planning", "ra"));
    otherModuleSetValue( getString("slaves", "navigatormodule"), "target", "targetde", getFloat("planning", "dec"));
    otherModuleSetValue( getString("slaves", "navigatormodule"), "target", "targetname", getString("planning", "object"));

    // Set sequencer's object data
    otherModuleRequestProfileLoad(getString("slaves", "sequencemodule"), getString("planning", "profile"));
    otherModuleSetValue( getString("slaves", "sequencemodule"), "object", "label", getString("planning", "object"));
    otherModuleSetValue( getString("slaves", "sequencemodule"), "object", "ra", getFloat("planning", "ra"));
    otherModuleSetValue( getString("slaves", "sequencemodule"), "object", "de", getFloat("planning", "dec"));

    // Ask navigator to slew to target
    otherModuleSetValue( getString("slaves", "navigatormodule"), "actions", "gototarget", true);
}

void Planner::skipLine(const QString &reason)
{
    int attempts    = ++mSkipCounts[mCurrentLine];
    int maxRetries  = getInt("visibility", "maxretries");

    if (attempts >= maxRetries)
    {
        logWarning("Giving up on target %1 after %2 attempts: %3", {getString("planning", "object"), QString::number(attempts), reason});
        getProperty("planning")->fetchLine(mCurrentLine);
        getEltPrg("planning", "progress")->setDynLabel("Failed: " + reason, false);
        getEltPrg("planning", "progress")->setPrgValue(0, false);
        getProperty("planning")->updateLine(mCurrentLine);
    }
    else
    {
        logWarning("Skipping target %1 (attempt %2/%3): %4", {getString("planning", "object"), QString::number(attempts), QString::number(maxRetries), reason});
        getProperty("planning")->fetchLine(mCurrentLine);
        getEltPrg("planning", "progress")->setDynLabel("Skipped: " + reason, false);
        getEltPrg("planning", "progress")->setPrgValue(0, false);
        getProperty("planning")->updateLine(mCurrentLine);
    }

    advanceToNextLine();
}

void Planner::advanceToNextLine()
{
    int count = getProperty("planning")->getGrid().count();
    for (int i = 1; i <= count; i++)
    {
        int idx = (mCurrentLine + i) % count;
        getProperty("planning")->fetchLine(idx);
        QString label = getEltPrg("planning", "progress")->dynLabel();
        if (label != "Finished" && !label.startsWith("Failed"))
        {
            mCurrentLine = idx;
            startLine();
            return;
        }
    }

    logInfo("Planning completed");
    getEltPrg("progress", "global")->setPrgValue(100, true);
    getEltPrg("progress", "global")->setDynLabel("Finished", true);
    mIsRunning = false;
}

bool Planner::checkVisibility(double ra, double dec, double durationSeconds, QString &reason)
{
    ln_lnlat_posn observer;
    observer.lat = getFloat("location", "lat");
    observer.lng = getFloat("location", "lon");

    ln_equ_posn target;
    target.ra  = ra * 15.0; // stored in hours, libnova expects degrees
    target.dec = dec;

    double jdNow = ln_get_julian_from_sys();
    double jdEnd = jdNow + (durationSeconds / 86400.0);

    ln_hrz_posn hrzNow, hrzEnd;
    ln_get_hrz_from_equ(&target, &observer, jdNow, &hrzNow);
    ln_get_hrz_from_equ(&target, &observer, jdEnd,  &hrzEnd);

    double minElevation = getFloat("visibility", "minelevation");
    double worstAlt = qMin(hrzNow.alt, hrzEnd.alt);
    if (worstAlt < minElevation)
    {
        reason = QString("elevation %1° below minimum %2°")
                 .arg(worstAlt, 0, 'f', 1)
                 .arg(minElevation, 0, 'f', 1);
        return false;
    }

    ln_equ_posn moonEqu;
    ln_get_lunar_equ_coords(jdNow, &moonEqu);
    double illumination = ln_get_lunar_disk(jdNow) * 100.0;
    double separation    = ln_get_angular_separation(&target, &moonEqu);

    double illumThreshold = getFloat("visibility", "moonilluminationthreshold");
    double sepThreshold   = getFloat("visibility", "moonseparationthreshold");
    if (illumination > illumThreshold && separation < sepThreshold)
    {
        reason = QString("moon %1% illuminated, only %2° away")
                 .arg(illumination, 0, 'f', 0)
                 .arg(separation, 0, 'f', 1);
        return false;
    }

    return true;
}
void Planner::onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line)
{
    if (mod != getString("slaves", "navigatormodule") && mod != getString("slaves", "sequencemodule") ) return;

    //logDebug("Planner::onOtherModuleEvent2 mod=%1 ev=%2 prop=%3 elt=%4 data=%5", {mod, OST::EvToString(ev), prp, elt, data});

    if (mod == getString("slaves", "sequencemodule") && ev == OST::EvType::qa && mWaitingDuration)
    {
        QJsonObject q = data.toJsonValue().toObject()["q"].toObject();
        if (q["query"].toString() == "profileduration")
        {
            mWaitingDuration = false;
            double seconds = q["result"].toObject()["seconds"].toDouble();

            QString reason;
            if (checkVisibility(getFloat("planning", "ra"), getFloat("planning", "dec"), seconds, reason))
            {
                proceedToSlew();
            }
            else
            {
                skipLine(reason);
            }
        }
        return;
    }

    if (mod == getString("slaves", "navigatormodule") && ev == OST::EvType::ea && prp == "signals"  && mWaitingNavigator)
    {
        QJsonObject o = data.toJsonValue().toObject()["e"].toObject();
        int s = o["state"].toInt();
        QString sd = o["statedescription"].toString();
        QString e = o["event"].toString();
        QString ed = o["eventdescription"].toString();
        //logDebug("catching signals event from %6 : %1 %2 %3 %4 %5", {OST::EvToString(ev), s, sd, e, ed, mod});
        if (OST::IntToState(s) == OST::Ok && sd == "ready")
        {
            navigatorComplete();
            return;
        }
    }

    // Report sequence progress
    if (mod == getString("slaves", "sequencemodule") && ev == OST::EvType::ea && prp == "progress" && mWaitingSequence)
    {
        int i = data.toMap()["e"].toMap()["global"].toInt();
        getEltPrg("planning", "progress")->setPrgValue(i, false);
        getProperty("planning")->updateLine(mCurrentLine);
    }

    if (mod == getString("slaves", "sequencemodule") && ev == OST::EvType::ea && prp == "signals"  && mWaitingSequence)
    {
        QJsonObject o = data.toJsonValue().toObject()["e"].toObject();
        int s = o["state"].toInt();
        QString sd = o["statedescription"].toString();
        QString e = o["event"].toString();
        QString ed = o["eventdescription"].toString();
        //logDebug("catching signals event from %6 : %1 %2 %3 %4 %5", {OST::EvToString(ev), s, sd, e, ed, mod});
        if (OST::IntToState(s) == OST::Ok && sd == "ready")
        {
            sequenceComplete();
            return;
        }
    }

}
