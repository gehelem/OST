#include "parkmanager.h"
#include <QPainter>
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

    OST::PropertyMulti* pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::OneOfMany);
    OST::ElementBool* b = new  OST::ElementBool("open", "Open all", "10", "");
    b->setPreIcon("play_arrow");
    pm->addElt(b);

    b = new  OST::ElementBool("close", "Close all", "20", "");
    b->setPreIcon("stop");
    pm->addElt(b);

    b = new  OST::ElementBool("auto", "Automate", "30", "");
    b->setPreIcon("hdr_auto");
    pm->addElt(b);

    b = new  OST::ElementBool("abort", "Abort motion", "40", "");
    b->setPreIcon("close");
    pm->addElt(b);
    b->setValue(false, true);


    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Park manager module");
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("template", "parkmanager");


    giveMeADevice("dome", "Dome", INDI::BaseDevice::DOME_INTERFACE);
    giveMeADevice("gps", "GPS", INDI::BaseDevice::GPS_INTERFACE);
    giveMeADevice("mount", "Mount", INDI::BaseDevice::TELESCOPE_INTERFACE);
    giveMeADevice("weather", "Weather", INDI::BaseDevice::WEATHER_INTERFACE);

    connectIndi();
    connectAllDevices();

}

Parkmanager::~Parkmanager()
{

}

void Parkmanager::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev == OST::ExtEvType::DP)
    {
        //logDebug("Message DP %1", {event.prpkey});
    }

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        bool b = event.data["m"][this->getModuleName()]["p"][event.prpkey]["e"][event.eltkey].toBool();
        if (b && event.eltkey == "open")
        {
            logInfo("Open request");
        }
        if (b && event.eltkey == "close")
        {
            logInfo("Close request");
        }
        if (b && event.eltkey == "auto")
        {
            logInfo("Auto request");
        }
        if (b && event.eltkey == "abort")
        {
            logInfo("Abort request");
        }
    }

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "zzz")
    {
        if (event.eltkey == "yyy")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
            }

        }
    }


}

void Parkmanager::onUpdateProperty(INDI::Property property)
{
    //if (mState == "idle") return;

    if (property.getDeviceName() == getString("devices", "mount"))
    {
        if (property.getName()   == std::string("EQUATORIAL_EOD_COORD"))
        {
            // Update mount position
            INDI::PropertyNumber prop = property;
            getEltFloat("mountstate", "RA")->setValue(prop.findWidgetByName("RA")->value, false);
            getEltFloat("mountstate", "DEC")->setValue(prop.findWidgetByName("DEC")->value, true);
        }
        if (property.getName()   == std::string("TELESCOPE_TRACK_STATE"))
        {
            // Update mount position
            INDI::PropertySwitch prop = property;
            getEltBool("mountstate", "tracking")->setValue(prop.findWidgetByName("TRACK_ON")->s, false);
        }
        if (property.getName()   == std::string("TELESCOPE_PARK"))
        {
            // Update mount position
            INDI::PropertySwitch prop = property;
            getEltBool("mountstate", "parked")->setValue(prop.findWidgetByName("PARK")->s, false);
        }
    }

    if (property.getDeviceName() == getString("devices", "dome"))
    {
        if (property.getName()   == std::string("DOME_SHUTTER"))
        {
            INDI::PropertySwitch prop = property;
            getEltBool("domestate", "shutterclosed")->setValue(prop.findWidgetByName("SHUTTER_CLOSE")->s, true);
        }
        if (property.getName()   == std::string("DOME_PARK"))
        {
            INDI::PropertySwitch prop = property;
            getEltBool("domestate", "parked")->setValue(prop.findWidgetByName("PARK")->s, true);
        }
    }

    if (property.getDeviceName() == getString("devices", "weather"))
    {
        if (property.getName()   == std::string("WEATHER_STATUS"))
        {
            INDI::PropertyLight prop = property;
            getEltLight("weatherstate", "global")->setValue(OST::IntToState(prop.findWidgetByName("WEATHER_FORECAST")->s), true);
        }
    }

    if (
        (property.getDeviceName() == getString("devices", "gps"))
        &&  (property.getName()   == std::string("GEOGRAPHIC_COORD"))
    )
    {
        // Update GPS Coords
        INDI::PropertyNumber prop = property;
        getEltFloat("gpsstate", "alt")->setValue(prop.findWidgetByName("ELEV")->value, false);
        getEltFloat("gpsstate", "lat")->setValue(prop.findWidgetByName("LAT")->value, false);
        getEltFloat("gpsstate", "lon")->setValue(prop.findWidgetByName("LONG")->value, true);
    }
    if (
        (property.getDeviceName() == getString("devices", "gps"))
        &&  (property.getName()   == std::string("TIME_UTC"))
    )
    {
        // Update GPS Time and date
        INDI::PropertyText prop = property;
        QDateTime dt;
        QString strdt = prop.findWidgetByName("UTC")->text;
        QString offset = prop.findWidgetByName("OFFSET")->text;
        dt = dt.fromString(strdt, Qt::ISODate);
        getEltDate("gpsstate", "date")->setValue(dt.date(), false);
        getEltTime("gpsstate", "time")->setValue(dt.time(), false);
        getEltFloat("gpsstate", "offset")->setValue(offset.toFloat(), true);
    }
}
void Parkmanager::initIndi()
{
    connectIndi();
    connectDevice(getString("devices", "gps"));
    connectDevice(getString("devices", "mount"));
    connectDevice(getString("devices", "weather"));
    connectDevice(getString("devices", "dome"));

    // Update GPS Coords
    INDI::PropertyNumber pn = getDevice(getString("devices", "gps").toStdString().c_str()).getProperty("GEOGRAPHIC_COORD",
                              INDI_NUMBER);
    getEltFloat("gpsstate", "alt")->setValue(pn.findWidgetByName("ELEV")->value, false);
    getEltFloat("gpsstate", "lat")->setValue(pn.findWidgetByName("LAT")->value, false);
    getEltFloat("gpsstate", "lon")->setValue(pn.findWidgetByName("LONG")->value, true);
    // Update GPS Time and date
    INDI::PropertyText pt = getDevice(getString("devices", "gps").toStdString().c_str()).getProperty("TIME_UTC", INDI_TEXT);
    QDateTime dt;
    QString strdt = pt.findWidgetByName("UTC")->text;
    QString offset = pt.findWidgetByName("OFFSET")->text;
    dt = dt.fromString(strdt, Qt::ISODate);
    getEltDate("gpsstate", "date")->setValue(dt.date(), false);
    getEltTime("gpsstate", "time")->setValue(dt.time(), false);
    getEltFloat("gpsstate", "offset")->setValue(offset.toFloat(), true);



}
