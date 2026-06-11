#include "globaldatastore.h"

GlobalDatastore::GlobalDatastore(const QString &name, const QString &label,
                                 const QString &profile, const QVariantMap &params)
    : IndiModule(name, label, profile, params)
{
    Q_INIT_RESOURCE(globaldatastore);
    loadOstPropertiesFromFile(":globaldatastore.json");
    giveMeADevice("gps", "GPS", INDI::BaseDevice::GPS_INTERFACE);
}

void GlobalDatastore::onAfterInit()
{
}

void GlobalDatastore::registerLov(const QString &propName, const QString &keyCol,
                                  const QString &labelCol, OST::LovBase* lov)
{
    mManagedLovs[propName] = { keyCol, labelCol, lov };
    rebuildLov(propName);
}

void GlobalDatastore::rebuildLov(const QString &propName)
{
    if (!mManagedLovs.contains(propName)) return;
    auto &ml = mManagedLovs[propName];
    OST::LovString* ls = static_cast<OST::LovString*>(ml.lov);
    ls->lovClear();
    int size = getGridSize(propName);
    for (int i = 0; i < size; i++)
    {
        QString key   = getGridString(propName, ml.keyCol,   i);
        QString label = getGridString(propName, ml.labelCol, i);
        if (!key.isEmpty()) ls->lovAdd(key, label);
    }
}

void GlobalDatastore::onExternalEvent(OST::ExtEvent event)
{
    /* add current location to stored list */
    if (event.ev == OST::ExtEvType::I4)
    {
        getProperty("locations")->push();
    }

    // After any grid modification, keep managed LOVs in sync
    if (event.ev == OST::ExtEvType::GC || event.ev == OST::ExtEvType::GU ||
            event.ev == OST::ExtEvType::GD || event.ev == OST::ExtEvType::GH ||
            event.ev == OST::ExtEvType::GB || event.ev == OST::ExtEvType::GR || event.ev == OST::ExtEvType::I4)
    {
        if (mManagedLovs.contains(event.prpkey))
            rebuildLov(event.prpkey);
    }

    /* retrieve current location from GPS if available */
    if (event.ev == OST::ExtEvType::I3)
    {
        getGPSData();
    }
}
void GlobalDatastore::getGPSData(void)
{
    logInfo("Retrieve GPS data");
    QString gpsDevice = getString("devices", "gps");
    if (gpsDevice.isEmpty())
    {
        logWarning("Please select a GPS device");
        return;
    }

    // Update GPS Coords
    INDI::PropertyNumber pn = getDevice(gpsDevice.toStdString().c_str()).getProperty("GEOGRAPHIC_COORD", INDI_NUMBER);
    QString name = "GPS - " + QDateTime::currentDateTime().toString("[yyyy-MM-dd]");

    getEltString("locations", "name")->setValue(name, false);
    getEltFloat("locations", "alt")->setValue(pn.findWidgetByName("ELEV")->value, false);
    getEltFloat("locations", "lat")->setValue(pn.findWidgetByName("LAT")->value, false);
    getEltFloat("locations", "lon")->setValue(pn.findWidgetByName("LONG")->value, true);

}
