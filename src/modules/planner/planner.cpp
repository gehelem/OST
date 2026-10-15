#include "planner.h"
#include "version.cc"
#include <libnova/julian_day.h>
#include <libnova/transform.h>
#include <libnova/lunar.h>
#include <libnova/angular_separation.h>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

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

    // "actions" (start/stop) and "location" - previously created directly
    // in the constructor (giveMeAnActions()/giveMeALocation()) with the
    // start/stop elements added by hand right here; now one call, shared
    // with any other module that wants the same "start/stop" recipe.
    defineMeAsPlanner();
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
        }
        if (event.ev == OST::ExtEvType::GU)
        {
            getStore()[event.prpkey]->updateLine(event.line, e.toVariantMap());
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
        setStateEvent(OST::Error, "error", "noindi", "INDI server not connected");
        abortPlanner();
        return;
    }

    // Check and connect devices
    QString g = getString("devices", "gps");
    if (g.isEmpty() || g == "--")
    {
        logError("No GPS selected");
        getProperty("actions")->setState(OST::Error, true);
        setStateEvent(OST::Error, "error", "nogps", "no GPS selected");
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
        setStateEvent(OST::Error, "error", "noslave", "no sequence module selected");
        abortPlanner();
        return;
    }
    g = getString("slaves", "navigatormodule");
    if (g.isEmpty() || g == "--")
    {
        logError("No Sequence module selected");
        getProperty("actions")->setState(OST::Error, true);
        setStateEvent(OST::Error, "error", "noslave", "no navigator module selected");
        abortPlanner();
        return;
    }
    int lineCount = getProperty("planning")->getGrid().count();

    if (lineCount == 0)
    {
        logError("Planning grid is empty");
        getProperty("actions")->setState(OST::Error, true);
        setStateEvent(OST::Error, "error", "emptygrid", "planning grid is empty");
        abortPlanner();
        return;
    }


    mIsRunning = true;

    //getProperty("actions")->setState(OST::Busy, true);
    setStateEvent(OST::Busy, "running", "start", "planner started");
    logInfo("Starting planner...");

    // Disable properties during operation (optional)
    //getProperty("parms")->disable();
    getProperty("devices")->disable();

    // Update progress
    getEltPrg("progress", "global")->setPrgValue(0, true);
    getEltPrg("progress", "global")->setDynLabel("Step 0/" + QString::number(lineCount), true);
    buildLivePlanning();

    mCurrentLine = 0;
    mSkipCounts.clear();
    startLine();

}

// Snapshots the "planning" config grid into "liveplanning" - not saved to
// the profile, this is what's read/updated while a run is in progress, so
// editing "planning" mid-run never disturbs it. "progress" lives only on
// "liveplanning" now (removed from "planning" entirely) - every line
// starts queued at 0%.
void Planner::buildLivePlanning()
{
    getProperty("liveplanning")->clearGrid();
    for (int i = 0; i < getProperty("planning")->getGrid().count(); i++)
    {
        getProperty("planning")->fetchLine(i);
        QVariantMap values;
        values["object"]  = getString("planning", "object");
        values["ra"]      = getFloat("planning", "ra");
        values["dec"]     = getFloat("planning", "dec");
        values["profile"] = getString("planning", "profile");
        getProperty("liveplanning")->newLine(values);

        getEltPrg("liveplanning", "progress")->setPrgValue(0, false);
        getEltPrg("liveplanning", "progress")->setDynLabel("Queued", false);
        getEltLight("liveplanning", "status")->setValue(OST::Idle, false);
        getProperty("liveplanning")->updateLine(getProperty("liveplanning")->getGrid().count() - 1);
    }
}

// Writes two files at the module's webroot, same creation/update behavior
// as Sequencer's writeLiveSequenceStats() - "liveplanning" is never saved
// to the profile and gets rebuilt empty by the next buildLivePlanning().
// Called from both terminal paths (onOperationComplete()/abortPlanner(),
// past its early "never actually started" return) - even a run cut short
// has real data worth keeping:
// - planning-report-<timestamp>.txt: one human-readable file per run - a
//   plain-text table of every "liveplanning" line (object, coordinates,
//   profile, final status).
// - planning-history.json: one file per module, read-modify-written on
//   every run. A single run covers several objects, not one, so unlike
//   Sequencer's object-name-keyed history this is just an array of past
//   runs, each with its own timestamp and lines.
void Planner::writeLivePlanningStats()
{
    // Unlike Sequencer (its own "startsequence" handler mkdir()s its
    // webroot folder already, for the FITS/JPEG files it writes there),
    // Planner never writes anything to its own webroot - this folder has
    // never had to exist before now.
    QDir().mkpath(getWebroot() + "/" + getModuleName());

    const QStringList headers = {"Object", "RA", "DEC", "Profile", "Status"};
    QList<QStringList> tableRows;
    QJsonArray lines;

    int rows = getProperty("liveplanning")->getGrid().count();
    for (int i = 0; i < rows; i++)
    {
        getProperty("liveplanning")->fetchLine(i);
        QString object  = getString("liveplanning", "object");
        double  ra      = getFloat("liveplanning", "ra");
        double  dec     = getFloat("liveplanning", "dec");
        QString profile = getString("liveplanning", "profile");
        QString status  = getEltPrg("liveplanning", "progress")->value().dynlabel;

        tableRows.append(QStringList{
            object, QString::number(ra, 'f', 5), QString::number(dec, 'f', 5), profile, status
        });

        QJsonObject line;
        line["object"]  = object;
        line["ra"]      = ra;
        line["dec"]     = dec;
        line["profile"] = profile;
        line["status"]  = status;
        lines.append(line);
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

    QJsonObject runEntry;
    runEntry["timestamp"] = timestamp;
    runEntry["lines"]     = lines;

    // ---- One human-readable report per run ----
    QList<int> widths(headers.size(), 0);
    auto measure = [&](const QStringList &row)
    {
        for (int c = 0; c < row.size(); c++) widths[c] = qMax(widths[c], row[c].length());
    };
    measure(headers);
    for (const QStringList &r : tableRows) measure(r);

    auto formatRow = [&](const QStringList &row)
    {
        QString s;
        for (int c = 0; c < row.size(); c++) s += row[c].leftJustified(widths[c] + 2);
        return s;
    };

    QStringList textLines;
    textLines << QString("Planner run: %1").arg(timestamp);
    textLines << "";
    textLines << formatRow(headers);
    for (const QStringList &r : tableRows) textLines << formatRow(r);

    QString textPath = getWebroot() + "/" + getModuleName() + "/planning-report-" + timestamp + ".txt";
    QFile textFile(textPath);
    if (textFile.open(QIODevice::WriteOnly))
    {
        textFile.write(textLines.join("\n").toUtf8());
        textFile.close();
        logInfo("Wrote planning report to %1", {textPath});
    }
    else
    {
        logWarning("Could not write planning report to %1", {textPath});
    }

    // ---- One JSON file per module, appended to on every run ----
    QString historyPath = getWebroot() + "/" + getModuleName() + "/planning-history.json";
    QJsonArray history;
    QFile historyFile(historyPath);
    if (historyFile.open(QIODevice::ReadOnly))
    {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(historyFile.readAll(), &err);
        historyFile.close();
        if (err.error == QJsonParseError::NoError && doc.isArray())
            history = doc.array();
    }

    history.append(runEntry);

    if (historyFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        historyFile.write(QJsonDocument(history).toJson());
        historyFile.close();
        logInfo("Appended planning run to %1", {historyPath});
    }
    else
    {
        logWarning("Could not write planning history to %1", {historyPath});
    }
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

    // The line in progress never goes through sequenceComplete()/
    // sequenceCancelled()/skipLine() on a manual stop - mark it Error so
    // it doesn't stay stuck on Busy.
    if (mCurrentLine >= 0 && mCurrentLine < getProperty("liveplanning")->getGrid().count())
    {
        getProperty("liveplanning")->fetchLine(mCurrentLine);
        getEltLight("liveplanning", "status")->setValue(OST::Error, false);
        getProperty("liveplanning")->updateLine(mCurrentLine);
    }

    writeLivePlanningStats();

    // Re-enable properties
    //getProperty("parms")->enable();
    getProperty("devices")->enable();

    // Reset progress
    getEltPrg("progress", "global")->setPrgValue(0, true);
    getEltPrg("progress", "global")->setDynLabel("Aborted", true);

    getProperty("actions")->setState(OST::Ok, true);
    setStateEvent(OST::Ok, "idle", "abort", "planner aborted");
}

void Planner::onOperationComplete()
{
    logInfo("Operation completed successfully");
    writeLivePlanningStats();

    mIsRunning = false;

    // Issue reported by Gilles, 2026-10-13: "start" was never reset here
    // (only abortPlanner() did it) - stuck true, same symptom class as
    // the devices/actions reset just below.
    getEltBool("actions", "start")->setValue(false, true);

    // Re-enable properties
    //getProperty("parms")->enable();
    getProperty("devices")->enable();

    // Update progress to 100%
    getEltPrg("progress", "global")->setPrgValue(100, true);
    getEltPrg("progress", "global")->setDynLabel("Completed", true);

    getProperty("actions")->setState(OST::Ok, true);
    setStateEvent(OST::Ok, "ready", "done", "planning completed");
}

void Planner::sequenceComplete()
{
    mWaitingSequence = false;
    mWaitingNavigator = false;


    getProperty("liveplanning")->fetchLine(mCurrentLine);
    getEltPrg("liveplanning", "progress")->setDynLabel("Finished", false);
    getEltPrg("liveplanning", "progress")->setPrgValue(100, false);
    getEltLight("liveplanning", "status")->setValue(OST::Ok, false);
    getProperty("liveplanning")->updateLine(mCurrentLine);

    advanceToNextLine();
}

void Planner::sequenceCancelled()
{
    mWaitingSequence = false;
    mWaitingNavigator = false;

    logWarning("Sequence for target %1 was cancelled - moving on", {getString("liveplanning", "object")});

    getProperty("liveplanning")->fetchLine(mCurrentLine);
    getEltPrg("liveplanning", "progress")->setDynLabel("Cancelled", false);
    getEltPrg("liveplanning", "progress")->setPrgValue(0, false);
    // Cancelled is terminal (excluded from advanceToNextLine()'s retry
    // scan, same as "Finished"/"Failed*") and it didn't complete - Error,
    // not Ok.
    getEltLight("liveplanning", "status")->setValue(OST::Error, false);
    getProperty("liveplanning")->updateLine(mCurrentLine);

    advanceToNextLine();
}
void Planner::navigatorComplete()
{
    mWaitingNavigator = false;

    logInfo("Navigator went to target %1, starting sequence with profile %2", {getString("liveplanning", "object"), getString("liveplanning", "profile")});

    getProperty("liveplanning")->fetchLine(mCurrentLine);
    getEltPrg("liveplanning", "progress")->setDynLabel("Shooting", false);
    getEltPrg("liveplanning", "progress")->setPrgValue(0, false);
    getProperty("liveplanning")->updateLine(mCurrentLine);

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
    int pg =  100 * mCurrentLine / getProperty("liveplanning")->getGrid().size();
    getEltPrg("progress", "global")->setPrgValue(pg, true);
    getEltPrg("progress", "global")->setDynLabel("Step 0/" + QString::number(getProperty("liveplanning")->getGrid().size()), true);

    // update line status
    getProperty("liveplanning")->fetchLine(mCurrentLine);
    getEltPrg("liveplanning", "progress")->setDynLabel("Checking visibility", false);
    getEltPrg("liveplanning", "progress")->setPrgValue(0, false);
    getEltLight("liveplanning", "status")->setValue(OST::Busy, false);
    getProperty("liveplanning")->updateLine(mCurrentLine);

    logInfo("Checking visibility for target %1", {getString("liveplanning", "object")});

    mWaitingSequence  = false;
    mWaitingNavigator = false;
    mWaitingDuration   = true;

    // Ask the sequencer how long the requested profile would theoretically take,
    // so the visibility check covers the whole sequence, not just this instant.
    otherModuleQuery(getString("slaves", "sequencemodule"), "profileduration",
    {{"profile", getString("liveplanning", "profile")}});
}

void Planner::proceedToSlew()
{
    getProperty("liveplanning")->fetchLine(mCurrentLine);
    getEltPrg("liveplanning", "progress")->setDynLabel("Slewing", false);
    getEltPrg("liveplanning", "progress")->setPrgValue(0, false);
    getProperty("liveplanning")->updateLine(mCurrentLine);

    logInfo("Navigator is slewing to target %1", {getString("liveplanning", "object")});

    mWaitingSequence = false;
    mWaitingNavigator = true;

    otherModuleRequestPropertyDump(getString("slaves", "navigatormodule"), "actions");

    // Set navigator's target
    otherModuleSetValue( getString("slaves", "navigatormodule"), "target", "targetra", getFloat("liveplanning", "ra"));
    otherModuleSetValue( getString("slaves", "navigatormodule"), "target", "targetde", getFloat("liveplanning", "dec"));
    otherModuleSetValue( getString("slaves", "navigatormodule"), "target", "targetname", getString("liveplanning", "object"));

    // Set sequencer's object data
    otherModuleRequestProfileLoad(getString("slaves", "sequencemodule"), getString("liveplanning", "profile"));
    otherModuleSetValue( getString("slaves", "sequencemodule"), "object", "label", getString("liveplanning", "object"));
    otherModuleSetValue( getString("slaves", "sequencemodule"), "object", "ra", getFloat("liveplanning", "ra"));
    otherModuleSetValue( getString("slaves", "sequencemodule"), "object", "de", getFloat("liveplanning", "dec"));

    // Ask navigator to slew to target
    otherModuleSetValue( getString("slaves", "navigatormodule"), "actions", "gototarget", true);
}

void Planner::skipLine(const QString &reason)
{
    int attempts    = ++mSkipCounts[mCurrentLine];
    int maxRetries  = getInt("visibility", "maxretries");

    if (attempts >= maxRetries)
    {
        logWarning("Giving up on target %1 after %2 attempts: %3", {getString("liveplanning", "object"), QString::number(attempts), reason});
        getProperty("liveplanning")->fetchLine(mCurrentLine);
        getEltPrg("liveplanning", "progress")->setDynLabel("Failed: " + reason, false);
        getEltPrg("liveplanning", "progress")->setPrgValue(0, false);
        getEltLight("liveplanning", "status")->setValue(OST::Error, false);
        getProperty("liveplanning")->updateLine(mCurrentLine);
    }
    else
    {
        logWarning("Skipping target %1 (attempt %2/%3): %4", {getString("liveplanning", "object"), QString::number(attempts), QString::number(maxRetries), reason});
        getProperty("liveplanning")->fetchLine(mCurrentLine);
        getEltPrg("liveplanning", "progress")->setDynLabel("Skipped: " + reason, false);
        getEltPrg("liveplanning", "progress")->setPrgValue(0, false);
        // Still eligible for a retry (advanceToNextLine()'s scan doesn't
        // exclude "Skipped*") - back to Idle/queued, not Error.
        getEltLight("liveplanning", "status")->setValue(OST::Idle, false);
        getProperty("liveplanning")->updateLine(mCurrentLine);
    }

    advanceToNextLine();
}

void Planner::advanceToNextLine()
{
    int count = getProperty("liveplanning")->getGrid().count();
    for (int i = 1; i <= count; i++)
    {
        int idx = (mCurrentLine + i) % count;
        getProperty("liveplanning")->fetchLine(idx);
        QString label = getEltPrg("liveplanning", "progress")->dynLabel();
        if (label != "Finished" && label != "Cancelled" && !label.startsWith("Failed"))
        {
            mCurrentLine = idx;
            startLine();
            return;
        }
    }

    // Bug reported by Gilles, 2026-10-13: every line skipped/failed (e.g.
    // a visibility problem affecting every object) landed here, which
    // used to just set mIsRunning = false inline - duplicating
    // onOperationComplete() below, but without its devices->enable()/
    // actions->setState(Ok)/setStateEvent() - "devices" properties stayed
    // disabled and the status stuck on Busy forever, not even recoverable
    // by clicking "stop" afterwards (abortPlanner() early-returns once
    // mIsRunning is already false). onOperationComplete() was already
    // written to do all of this correctly, it just wasn't called from
    // anywhere.
    onOperationComplete();
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
            if (checkVisibility(getFloat("liveplanning", "ra"), getFloat("liveplanning", "dec"), seconds, reason))
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
        getEltPrg("liveplanning", "progress")->setPrgValue(i, false);
        getProperty("liveplanning")->updateLine(mCurrentLine);
    }

    if (mod == getString("slaves", "sequencemodule") && ev == OST::EvType::ea && prp == "signals"  && mWaitingSequence)
    {
        QJsonObject o = data.toJsonValue().toObject()["e"].toObject();
        int s = o["state"].toInt();
        QString sd = o["statedescription"].toString();
        QString e = o["event"].toString();
        QString ed = o["eventdescription"].toString();
        //logDebug("catching signals event from %6 : %1 %2 %3 %4 %5", {OST::EvToString(ev), s, sd, e, ed, mod});
        if (OST::IntToState(s) == OST::Ok && sd == "ready" && e == "abortdone")
        {
            sequenceCancelled();
            return;
        }
        if (OST::IntToState(s) == OST::Ok && sd == "ready")
        {
            sequenceComplete();
            return;
        }
    }

}
