#include "parkmanager.h"
#include <QPainter>
#include <QTime>
#include <libnova/solar.h>
#include <libnova/julian_day.h>
#include <libnova/rise_set.h>
#include <libnova/transform.h>
#include "version.cc"

Parkmanager *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Parkmanager *basemodule = new Parkmanager(name, label, profile, availableModuleLibs);
    return basemodule;
}

Parkmanager::Parkmanager(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{
    loadOstPropertiesFromFile(":parkmanager.json");
    giveMeAnActions();
    giveMeALocation();

    OST::PropertyMulti* pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::OneOfMany);

    OST::ElementBool* b = new OST::ElementBool("open", "Open all", "10", "");
    b->setPreIcon("play_arrow");
    pm->addElt(b);

    b = new OST::ElementBool("close", "Close all", "20", "");
    b->setPreIcon("stop");
    pm->addElt(b);

    b = new OST::ElementBool("auto", "Automate", "30", "");
    b->setPreIcon("hdr_auto");
    pm->addElt(b);

    b = new OST::ElementBool("abort", "Abort motion", "40", "");
    b->setPreIcon("close");
    pm->addElt(b);

    b = new OST::ElementBool("idle", "Idle", "50", "");
    b->setPreIcon("mode_standby");
    pm->addElt(b);

    getEltBool("actions", "idle")->setValue(true, true);
    getEltBool("weatherrules", "disabled")->setValue(true, true);

    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Park manager module");
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("template", "parkmanager");

    giveMeADevice("dome", "Dome", INDI::BaseDevice::DOME_INTERFACE);
    giveMeADevice("mount", "Mount", INDI::BaseDevice::TELESCOPE_INTERFACE);
    giveMeADevice("weather", "Weather", INDI::BaseDevice::WEATHER_INTERFACE);

    connectIndi();
    connectAllDevices();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Parkmanager::onTimer);
    timer->start(2000);
}

Parkmanager::~Parkmanager() {}

void Parkmanager::onAfterInit(void) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

bool Parkmanager::hasDome(void)
{
    return !getString("devices", "dome").isEmpty();
}

bool Parkmanager::hasMount(void)
{
    return !getString("devices", "mount").isEmpty();
}

bool Parkmanager::isWeatherOk(void)
{
    return getEltLight("weatherstate", "global")->value() == OST::Ok;
}

bool Parkmanager::isTimeInWindow(QTime now, QTime start, QTime end)
{
    if (start <= end)
        return now >= start && now < end;
    // Overnight window (e.g. 22:00 → 05:00)
    return now >= start || now < end;
}

void Parkmanager::goIdle(void)
{
    mMode  = Mode::Idle;
    mPhase = Phase::Idle;
    getEltBool("actions", "idle")->setValue(true, true);
}

// ---------------------------------------------------------------------------
// User actions
// ---------------------------------------------------------------------------

void Parkmanager::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        bool b = event.data["m"][this->getModuleName()]["p"][event.prpkey]["e"][event.eltkey].toBool();
        if (!b) return;

        if (event.eltkey == "open")
        {
            mMode = Mode::Open;
            startOpenSequence();
        }
        else if (event.eltkey == "close")
        {
            mMode = Mode::Close;
            startCloseSequence();
        }
        else if (event.eltkey == "auto")
        {
            mMode  = Mode::Auto;
            mPhase = Phase::Idle;
            logInfo("Auto mode engaged");
        }
        else if (event.eltkey == "abort")
        {
            mMode  = Mode::Abort;
            mPhase = Phase::EmergencyStopping;
            logWarning("Emergency abort requested");
        }
        else if (event.eltkey == "idle")
        {
            logInfo("Going idle");
            goIdle();
        }
    }
}

// ---------------------------------------------------------------------------
// Sequences
// ---------------------------------------------------------------------------

void Parkmanager::startOpenSequence(void)
{
    if (getEltBool("weatherrules", "mustbeok")->value() && !isWeatherOk())
    {
        logWarning("Weather is not OK, open sequence aborted");
        goIdle();
        return;
    }

    if (hasDome())
        mPhase = Phase::OpeningDome;
    else if (hasMount())
        mPhase = Phase::UnparkingMount;
    else
    {
        logWarning("No dome and no mount configured");
        goIdle();
    }
}

void Parkmanager::startCloseSequence(void)
{
    if (hasMount())
        mPhase = Phase::StoppingTracking;
    else if (hasDome())
        mPhase = Phase::ClosingDome;
    else
    {
        logWarning("No dome and no mount configured");
        goIdle();
    }
}

// ---------------------------------------------------------------------------
// State machine — driven by the 2s timer
// ---------------------------------------------------------------------------

void Parkmanager::processPhase(void)
{
    QString mount = getString("devices", "mount");
    QString dome  = getString("devices", "dome");

    switch (mPhase)
    {
        case Phase::Idle:
            break;

        // ---- Opening sequence ------------------------------------------------

        case Phase::OpeningDome:
            logInfo("Opening dome shutter");
            sendModNewSwitch(dome.toStdString().c_str(), "DOME_SHUTTER", "SHUTTER_OPEN", ISS_ON);
            mPhase = Phase::WaitingDomeOpen;
            break;

        case Phase::WaitingDomeOpen:
            if (!getEltBool("domestate", "shutterclosed")->value())
            {
                logInfo("Dome open — unparking mount");
                mPhase = hasMount() ? Phase::UnparkingMount : Phase::OpenMonitoring;
            }
            break;

        case Phase::UnparkingMount:
            logInfo("Unparking mount");
            sendModNewSwitch(mount.toStdString().c_str(), "TELESCOPE_PARK", "UNPARK", ISS_ON);
            mPhase = Phase::WaitingUnpark;
            break;

        case Phase::WaitingUnpark:
            if (!getEltBool("mountstate", "parked")->value())
            {
                logInfo("Mount unparked — starting tracking");
                mPhase = Phase::StartingTracking;
            }
            break;

        case Phase::StartingTracking:
            logInfo("Starting mount tracking");
            sendModNewSwitch(mount.toStdString().c_str(), "TELESCOPE_TRACK_STATE", "TRACK_ON", ISS_ON);
            if (!getBool("mountstate", "home")) getEltBool("mountstate", "home")->setValue(true, true);
            mPhase = Phase::OpenMonitoring;
            logInfo("Open sequence complete");
            break;

        case Phase::OpenMonitoring:
            if (getEltBool("weatherrules", "mustbeok")->value() && !isWeatherOk())
            {
                logWarning("Weather degraded — triggering close sequence");
                mMode = Mode::Close;
                startCloseSequence();
            }
            break;

        // ---- Closing sequence ------------------------------------------------

        case Phase::StoppingTracking:
            logInfo("Stopping mount tracking");
            sendModNewSwitch(mount.toStdString().c_str(), "TELESCOPE_TRACK_STATE", "TRACK_OFF", ISS_ON);
            mPhase = Phase::GoingHome;
            break;

        case Phase::GoingHome:
            getEltBool("mountstate", "home")->setValue(false, false);
            logInfo("Sending mount to home position");
            sendModNewSwitch(mount.toStdString().c_str(), "TELESCOPE_HOME", "GO", ISS_ON);
            mPhase = Phase::WaitingHome;
            break;

        case Phase::WaitingHome:
            if (getEltBool("mountstate", "home")->value())
            {
                logInfo("Mount at home — parking");
                mPhase = Phase::ParkingMount;
            }
            break;

        case Phase::ParkingMount:
            logInfo("Parking mount");
            sendModNewSwitch(mount.toStdString().c_str(), "TELESCOPE_PARK", "PARK", ISS_ON);
            mPhase = Phase::WaitingPark;
            break;

        case Phase::WaitingPark:
            if (getEltBool("mountstate", "parked")->value())
            {
                if (hasDome())
                {
                    logInfo("Mount parked — closing dome");
                    mPhase = Phase::ClosingDome;
                }
                else
                {
                    logInfo("Close sequence complete");
                    goIdle();
                }
            }
            break;

        case Phase::ClosingDome:
            logInfo("Closing dome shutter");
            sendModNewSwitch(dome.toStdString().c_str(), "DOME_SHUTTER", "SHUTTER_CLOSE", ISS_ON);
            mPhase = Phase::WaitingDomeClose;
            break;

        case Phase::WaitingDomeClose:
            if (getEltBool("domestate", "shutterclosed")->value())
            {
                logInfo("Close sequence complete");
                goIdle();
            }
            break;

        // ---- Emergency -------------------------------------------------------

        case Phase::EmergencyStopping:
            logWarning("Emergency stop — aborting all motion");
            if (hasMount())
                sendModNewSwitch(mount.toStdString().c_str(), "TELESCOPE_ABORT_MOTION", "ABORT", ISS_ON);
            if (hasDome())
                sendModNewSwitch(dome.toStdString().c_str(), "DOME_ABORT_MOTION", "ABORT", ISS_ON);
            goIdle();
            break;
    }
}

// ---------------------------------------------------------------------------
// Auto mode — check time rules and trigger sequences
// ---------------------------------------------------------------------------

void Parkmanager::checkAutoMode(void)
{
    if (mMode != Mode::Auto) return;

    // Only act when idle (waiting to open) or monitoring (waiting to close)
    if (mPhase != Phase::Idle && mPhase != Phase::OpenMonitoring) return;

    QTime now = QTime::currentTime();
    bool shouldBeOpen = false;

    if (getEltBool("timerules", "anytime")->value())
    {
        shouldBeOpen = true;
    }
    else if (getEltBool("timerules", "duskdawn")->value())
    {
        QTime sunset  = getEltTime("coming", "sunset")->value();
        QTime sunrise = getEltTime("coming", "sunrise")->value();
        shouldBeOpen = isTimeInWindow(now, sunset, sunrise);
    }
    else if (getEltBool("timerules", "fixed")->value())
    {
        QTime start = getEltTime("timerules", "fixedstart")->value();
        QTime end   = getEltTime("timerules", "fixedend")->value();
        shouldBeOpen = isTimeInWindow(now, start, end);
    }

    if (shouldBeOpen && mPhase == Phase::Idle)
    {
        logInfo("Auto mode: opening window started");
        startOpenSequence();
    }
    else if (!shouldBeOpen && mPhase == Phase::OpenMonitoring)
    {
        logInfo("Auto mode: opening window ended, closing");
        startCloseSequence();
    }
}

// ---------------------------------------------------------------------------
// Timer
// ---------------------------------------------------------------------------

void Parkmanager::onTimer(void)
{
    refreshDriversData();
    calculateSunset();
    checkAutoMode();
    processPhase();
}

// ---------------------------------------------------------------------------
// INDI data refresh
// ---------------------------------------------------------------------------

void Parkmanager::refreshDriversData(void)
{
    if (!getString("devices", "weather").isEmpty())
    {
        sendModNewSwitch(getString("devices", "weather").toStdString().c_str(), "WEATHER_REFRESH", "REFRESH", ISS_ON);
    }

    if (getString("devices", "mount").isEmpty())
    {
        logWarning("Mount device missing");
    }
    else
    {
        INDI::PropertyNumber pn = getDevice(getString("devices",
                                            "mount").toStdString().c_str()).getProperty("EQUATORIAL_EOD_COORD", INDI_NUMBER);
        getEltFloat("mountstate", "RA")->setValue(pn.findWidgetByName("RA")->value, false);
        getEltFloat("mountstate", "DEC")->setValue(pn.findWidgetByName("DEC")->value, true);

        INDI::PropertySwitch ps = getDevice(getString("devices",
                                            "mount").toStdString().c_str()).getProperty("TELESCOPE_TRACK_STATE", INDI_SWITCH);
        getEltBool("mountstate", "tracking")->setValue(ps.findWidgetByName("TRACK_ON")->s, false);

        ps = getDevice(getString("devices", "mount").toStdString().c_str()).getProperty("TELESCOPE_PARK", INDI_SWITCH);
        getEltBool("mountstate", "parked")->setValue(ps.findWidgetByName("PARK")->s, false);
    }

    if (getString("devices", "dome").isEmpty())
    {
        logWarning("Dome device missing");
    }
    else
    {
        INDI::PropertySwitch ps = getDevice(getString("devices", "dome").toStdString().c_str()).getProperty("DOME_SHUTTER",
                                  INDI_SWITCH);
        getEltBool("domestate", "shutterclosed")->setValue(ps.findWidgetByName("SHUTTER_CLOSE")->s, true);

        ps = getDevice(getString("devices", "dome").toStdString().c_str()).getProperty("DOME_PARK", INDI_SWITCH);
        getEltBool("domestate", "parked")->setValue(ps.findWidgetByName("PARK")->s, true);
    }

    if (!getString("devices", "weather").isEmpty())
    {
        INDI::PropertyLight pl = getDevice(getString("devices",
                                           "weather").toStdString().c_str()).getProperty("WEATHER_STATUS", INDI_LIGHT);
        getEltLight("weatherstate", "global")->setValue(OST::IntToState(pl.findWidgetByName("WEATHER_FORECAST")->s), true);
    }
}

void Parkmanager::calculateSunset(void)
{
    double JD = ln_get_julian_from_sys();

    ln_rst_time rst;
    ln_zonedate rise, set, transit;
    ln_lnlat_posn observer;
    observer.lat = getFloat("location", "lat");
    observer.lng = getFloat("location", "lon");
    if (ln_get_solar_rst(JD, &observer, &rst) != 0)
    {
        logError("Sun is circumpolar");
        return;
    }

    ln_get_local_date(rst.rise, &rise);
    ln_get_local_date(rst.transit, &transit);
    ln_get_local_date(rst.set, &set);

    QTime t;
    t.setHMS(rise.hours, rise.minutes, rise.seconds);
    getEltTime("coming", "sunrise")->setValue(t, false);
    t.setHMS(set.hours, set.minutes, set.seconds);
    getEltTime("coming", "sunset")->setValue(t, true);
}

void Parkmanager::onUpdateProperty(INDI::Property property)
{
    if (
        (property.getDeviceName() == getString("devices", "mount"))
        &&  (property.getName()   == std::string("TELESCOPE_HOME"))
        &&  (property.getState() == IPS_OK)
    )
    {
        if (!getBool("mountstate", "home")) getEltBool("mountstate", "home")->setValue(true, true);
    }
}

void Parkmanager::initIndi()
{
    connectIndi();
    connectDevice(getString("devices", "mount"));
    connectDevice(getString("devices", "weather"));
    connectDevice(getString("devices", "dome"));
}
