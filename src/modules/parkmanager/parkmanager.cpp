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
        if (event.eltkey == "xxx")
        {
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

    if (
        (property.getDeviceName() == getString("devices", "mount"))
        &&  (property.getName()   == std::string("EQUATORIAL_EOD_COORD"))
    )
    {
        // Update mount position
        INDI::PropertyNumber prop = property;
        getEltFloat("mountposition", "RA")->setValue(prop.findWidgetByName("RA")->value, false);
        getEltFloat("mountposition", "DEC")->setValue(prop.findWidgetByName("DEC")->value, true);
    }
    if (
        (property.getDeviceName() == getString("devices", "gps"))
        &&  (property.getName()   == std::string("GEOGRAPHIC_COORD"))
    )
    {
        // Update GPS Coords
        INDI::PropertyNumber prop = property;
        getEltFloat("gpslocation", "alt")->setValue(prop.findWidgetByName("ELEV")->value, false);
        getEltFloat("gpslocation", "lat")->setValue(prop.findWidgetByName("LAT")->value, false);
        getEltFloat("gpslocation", "lon")->setValue(prop.findWidgetByName("LONG")->value, true);
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
        getEltDate("gpstime", "date")->setValue(dt.date(), false);
        getEltTime("gpstime", "time")->setValue(dt.time(), false);
        getEltFloat("gpstime", "offset")->setValue(offset.toFloat(), true);
    }
    if (
        (property.getDeviceName() == getString("devices", "mount"))
        &&  (property.getName()   == std::string("EQUATORIAL_EOD_COORD"))
        &&  (property.getState() == IPS_OK)
        && mState == "running"
    )
    {
    }
}
void Parkmanager::initIndi()
{
    connectIndi();
    connectDevice(getString("devices", "camera"));
    connectDevice(getString("devices", "mount"));
    setBLOBMode(B_ALSO, getString("devices", "camera").toStdString().c_str(), nullptr);
    if (getString("devices", "camera") == "CCD Simulator")
    {
        //sendModNewNumber(getString("devices", "camera"), "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 0.01 );
    }
    enableDirectBlobAccess(getString("devices", "camera").toStdString().c_str(), nullptr);

    // Update GPS Coords
    INDI::PropertyNumber pn = getDevice(getString("devices", "gps").toStdString().c_str()).getProperty("GEOGRAPHIC_COORD",
                              INDI_NUMBER);
    getEltFloat("gpslocation", "alt")->setValue(pn.findWidgetByName("ELEV")->value, false);
    getEltFloat("gpslocation", "lat")->setValue(pn.findWidgetByName("LAT")->value, false);
    getEltFloat("gpslocation", "lon")->setValue(pn.findWidgetByName("LONG")->value, true);
    // Update GPS Time and date
    INDI::PropertyText pt = getDevice(getString("devices", "gps").toStdString().c_str()).getProperty("TIME_UTC", INDI_TEXT);
    QDateTime dt;
    QString strdt = pt.findWidgetByName("UTC")->text;
    QString offset = pt.findWidgetByName("OFFSET")->text;
    dt = dt.fromString(strdt, Qt::ISODate);
    getEltDate("gpstime", "date")->setValue(dt.date(), false);
    getEltTime("gpstime", "time")->setValue(dt.time(), false);
    getEltFloat("gpstime", "offset")->setValue(offset.toFloat(), true);



}
