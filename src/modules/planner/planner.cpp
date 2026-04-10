#include "planner.h"
#include "version.cc"

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
    setMetadata("description", "Planner module");
    setMetadata("thisversion", "0.1");
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
    // Check if this is a sequence completion event
    //if (e.type == "sequencedone" && mWaitingSequence && getString("parms", "sequencemodule") == e.module )
    //{
    //    sequenceComplete();
    //    return;
    //}

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


    // Report sequence progress
    //if (e.type == "se" && mWaitingSequence && getString("parms", "sequencemodule") == e.module
    //        && e.property == "progress" )
    //{
    //    int i = e.data["elements"].toMap()["global"].toMap()["value"].toInt();
    //    QString s = e.data["elements"].toMap()["global"].toMap()["dynlabel"].toString();
    //    getEltPrg("planning", "progress")->setPrgValue(i, false);
    //    getEltPrg("planning", "progress")->setDynLabel("Seq " + s, false);
    //    getProperty("planning")->updateLine(mCurrentLine);
    //}

    //// Check if this is a navigator completion event
    //if (e.type == "navigatordone" && mIsRunning && mWaitingNavigator
    //        && getString("parms", "navigatormodule") == e.module )
    //{
    //    navigatorComplete();
    //    return;
    //}

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
    g = getString("parms", "sequencemodule");
    if (g.isEmpty() || g == "--")
    {
        logError("No Sequence module selected");
        getProperty("actions")->setState(OST::Error, true);
        abortPlanner();
        return;
    }
    g = getString("parms", "navigatormodule");
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
    getProperty("parms")->disable();
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
    startLine();

}

void Planner::abortPlanner()
{
    getEltBool("actions", "start")->setValue(false, false);
    getEltBool("actions", "stop")->setValue(true, true);

    if (!mIsRunning)
        return;

    mIsRunning = false;
    mWaitingSequence = false;
    mWaitingNavigator = false;

    logInfo("Operation aborted");

    // Re-enable properties
    getProperty("parms")->enable();
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
    getProperty("parms")->enable();
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


    mCurrentLine++;
    if (getProperty("planning")->getGrid().count() <= mCurrentLine)
    {
        logInfo("Planning completed");
        mIsRunning = false;
        return;
    }

    startLine();

}
void Planner::navigatorComplete()
{
    mWaitingNavigator = false;

    logInfo("Navigator went to target " + getString("planning",
            "object") + ", starting sequence with profile " + getString("planning",
                    "profile") );

    // Ask sequence to start
    QVariantMap eltData;
    eltData["startsequence"] = true;
    QVariantMap propData;
    propData["elements"] = eltData;
    QVariantMap eventData;
    eventData["actions"] = propData;
    //emit moduleEvent("Fsetproperty", getString("parms", "sequencemodule"), "", eventData);

    mWaitingSequence = true;

}
void Planner::startLine()
{
    // update line status
    getProperty("planning")->fetchLine(mCurrentLine);
    getEltPrg("planning", "progress")->setDynLabel("Slewing", false);
    getEltPrg("planning", "progress")->setPrgValue(0, false);
    getProperty("planning")->updateLine(mCurrentLine);

    logInfo("Navigator is slewing to target " + getString("planning", "object"));

    mWaitingSequence = false;
    mWaitingNavigator = true;

    // Set navigator's target
    QVariantMap eltData;
    QVariantMap propData;
    QVariantMap eventData;
    eltData["targetname"] = getString("planning", "object");
    propData["elements"] = eltData;
    eventData["actions"] = propData;

    OST::ExtEvent event;
    event.ev = OST::ExtEvType::DP;
    event.mod = getString("parms", "navigatormodule");
    event.prpkey = "actions";
    emit interModuleRequest(event);

    //emit moduleEvent("Fsetproperty", getString("parms", "navigatormodule"), "", eventData);
    eltData = QVariantMap();
    eltData["targetra"] = getFloat("planning", "ra");
    propData["elements"] = eltData;
    eventData["actions"] = propData;
    //emit moduleEvent("Fsetproperty", getString("parms", "navigatormodule"), "", eventData);
    eltData = QVariantMap();
    eltData["targetde"] = getFloat("planning", "dec");
    propData["elements"] = eltData;
    eventData["actions"] = propData;
    //emit moduleEvent("Fsetproperty", getString("parms", "navigatormodule"), "", eventData);

    // Set sequencer's profile
    eltData = QVariantMap();
    eltData["name"] = getString("planning", "profile");
    propData = QVariantMap();
    propData["elements"] = eltData;
    eventData = QVariantMap();
    eventData["loadprofile"] = propData;
    //emit moduleEvent("Fsetproperty", getString("parms", "sequencemodule"), "", eventData); // set profile to load
    //emit moduleEvent("Fposticon", getString("parms", "sequencemodule"), "", eventData); // load it

    // Set sequencer's object data
    eltData = QVariantMap();
    eltData["label"] = getString("planning", "object");
    propData = QVariantMap();
    propData["elements"] = eltData;
    eventData = QVariantMap();
    eventData["object"] = propData;
    //emit moduleEvent("Fsetproperty", getString("parms", "sequencemodule"), "", eventData);
    eventData = QVariantMap();
    eltData["ra"] = getFloat("planning", "ra");
    propData["elements"] = eltData;
    eventData["object"] = propData;
    //emit moduleEvent("Fsetproperty", getString("parms", "sequencemodule"), "", eventData);
    eventData = QVariantMap();
    eltData["de"] = getFloat("planning", "dec");
    propData["elements"] = eltData;
    eventData["object"] = propData;
    //emit moduleEvent("Fsetproperty", getString("parms", "sequencemodule"), "", eventData);

    // Ask navigator to slew to target
    eltData = QVariantMap();
    eltData["gototarget"] = true;
    propData = QVariantMap();
    propData["elements"] = eltData;
    eventData = QVariantMap();
    eventData["actions"] = propData;
    //emit moduleEvent("Fsetproperty", getString("parms", "navigatormodule"), "", eventData);

}
void Planner::onOtherModuleEvent(OST::EvType ev, QVariant data, OST::ElementBase*elt, OST::PropertyBase*prop,
                                 OST::LovBase*lov,
                                 Basemodule*mod)
{
    if (mod->getModuleName() != getString("parms", "navigatormodule")
            && mod->getModuleName() != getString("parms", "sequencemodule") ) return;

    if (mod->getModuleName() == getString("parms", "navigatormodule") && ev == OST::EvType::ap && prop->key() == "actions")
    {
        logDebug("Planner::onOthermoduleEvent mod=%1 ev=%2 prop=%3 status=%4", {mod->getModuleName(), OST::EvToString(ev), prop->key(), prop->state()});
    }
}
