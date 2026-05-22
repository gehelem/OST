#include <QtCore>
#include <basedevice.h>
#include "indimodule.h"
#include "version.cc"
#include <common.h>

IndiModule::IndiModule(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : Basemodule(name, label, profile, availableModuleLibs)
{
    setVerbose(false);
    //_moduletype = "IndiModule";
    loadOstPropertiesFromFile(":indimodule.json");

    setMetadata("baseindiGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("baseindiGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("baseindiGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("baseindiVersion", QString::fromStdString(Version::GIT_TAG));


    OST::LovString* ls = new OST::LovString("DRIVER_INTERFACE-TELESCOPE_INTERFACE");
    createGlobLov("DRIVER_INTERFACE-TELESCOPE_INTERFACE", ls);
    ls = new OST::LovString("DRIVER_INTERFACE-GENERAL_INTERFACE");
    createGlobLov("DRIVER_INTERFACE-GENERAL_INTERFACE", ls);
    ls = new OST::LovString("DRIVER_INTERFACE-CCD_INTERFACE");
    createGlobLov("DRIVER_INTERFACE-CCD_INTERFACE", ls);
    ls = new OST::LovString("DRIVER_INTERFACE-GUIDER_INTERFACE");
    createGlobLov("DRIVER_INTERFACE-GUIDER_INTERFACE", ls);
    ls = new OST::LovString("DRIVER_INTERFACE-FOCUSER_INTERFACE");
    createGlobLov("DRIVER_INTERFACE-FOCUSER_INTERFACE", ls);
    ls = new OST::LovString("DRIVER_INTERFACE-FILTER_INTERFACE");
    createGlobLov("DRIVER_INTERFACE-FILTER_INTERFACE", ls);
    ls = new OST::LovString("DRIVER_INTERFACE-GPS_INTERFACE");
    createGlobLov("DRIVER_INTERFACE-GPS_INTERFACE", ls);
    ls = new OST::LovString("DRIVER_INTERFACE-WEATHER_INTERFACE");
    createGlobLov("DRIVER_INTERFACE-WEATHER_INTERFACE", ls);
    ls = new OST::LovString("DRIVER_INTERFACE-AUX_INTERFACE");
    createGlobLov("DRIVER_INTERFACE-AUX_INTERFACE", ls);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &IndiModule::connectIndiTimer);
    timer->start(2000);

}
/**
 * @brief INDI module event handler (Hook 2/3)
 *
 * Override of Basemodule's hook. Called automatically by the orchestrator
 * after onExternalEventBase() and before onExternalEventCustom().
 *
 * NO need to call parent - orchestration is automatic!
 */
bool IndiModule::onExternalEventIndi(OST::ExtEvent event)
{
    //qDebug() << "IndiModule::onExternalEventIndi event = " << OST::ExtEvToString(event.ev) << " p=" << event.prpkey << " e=" <<
    //         event.eltkey << " l=" << event.lovkey << " i=" << event.line;

    QJsonObject o = event.data["m"].toObject()[this->getModuleName()].toObject();
    QJsonObject p = o["p"].toObject();
    QJsonObject e = p[event.prpkey].toObject()["e"].toObject();

    if (event.ev == OST::ExtEvType::SV)
    {

        QVariantMap eltval;
        eltval["value"] = e.begin().value().toVariant();
        if (event.prpkey == "serveractions" && event.eltkey == "conserv")
        {
            getProperty(event.prpkey)->setState(OST::Busy, true);
            if (connectIndi()) getProperty(event.prpkey)->setState(OST::Ok, true);
            else getProperty(event.prpkey)->setState(OST::Error, true);

            OST::ElementUpdate u;
            bool b = true;
            getStore()[event.prpkey]->getElt(event.eltkey)->accept(&u, eltval, b);
            return true;
        }
        if (event.prpkey == "serveractions" && event.eltkey == "disconserv")
        {
            getProperty(event.prpkey)->setState(OST::Busy, true);
            if (disconnectIndi()) getProperty(event.prpkey)->setState(OST::Ok, true);
            else getProperty(event.prpkey)->setState(OST::Error, true);

            OST::ElementUpdate u;
            bool b = true;
            getStore()[event.prpkey]->getElt(event.eltkey)->accept(&u, eltval, b);
            return true;
        }
        if (event.prpkey == "optic")
        {
            getEltString("optic", "optic")->setValue("", true);
            if (mGlobalDatastore && event.eltkey == "optic")
            {
                QString opticsName = eltval["value"].toString();
                if (!opticsName.isEmpty())
                {
                    double focal    = mGlobalDatastore->getGridFloat("optics", "focal",    "name", opticsName);
                    double diameter = mGlobalDatastore->getGridFloat("optics", "diameter", "name", opticsName);
                    if (focal > 0.0)
                        logInfo("Optics: %1 - focal %2 mm, diameter %3 mm", {opticsName, focal, diameter});
                    else
                        logWarning("Optics '%1' not found in GlobalDatastore", {opticsName});
                    getEltFloat("optic", "fl")->setValue(focal);
                    getEltFloat("optic", "diam")->setValue(diameter);
                    getEltString("optic", "optic")->setValue(eltval["value"].toString());
                }
            }
            setFocalLengthAndDiameter(); // Mandatory for simulators to work
        }

        if (event.prpkey == "devices"  && event.eltkey == "equipments")
        {
            disconnectIndi();
            if (mGlobalDatastore)
            {
                QString equipmentName = eltval["value"].toString();
                if (!equipmentName.isEmpty())
                {
                    if (getProperty("devices")->getElts()->contains("camera")) getEltString("devices", "camera")->setValue("");
                    if (getProperty("devices")->getElts()->contains("mount")) getEltString("devices", "mount")->setValue("");
                    if (getProperty("devices")->getElts()->contains("focuser")) getEltString("devices", "focuser")->setValue("");
                    if (getProperty("devices")->getElts()->contains("filter")) getEltString("devices", "filter")->setValue("");
                    if (getProperty("devices")->getElts()->contains("guidecamera")) getEltString("devices", "guidecamera")->setValue("");
                    if (getProperty("devices")->getElts()->contains("guidest4")) getEltString("devices", "guidest4")->setValue("");
                    if (getProperty("devices")->getElts()->contains("gps")) getEltString("devices", "gps")->setValue("");
                    QString camera = mGlobalDatastore->getGridString("equipments", "camera",    "name", equipmentName);
                    QString mount = mGlobalDatastore->getGridString("equipments", "mount",    "name", equipmentName);
                    QString focuser = mGlobalDatastore->getGridString("equipments", "focuser",    "name", equipmentName);
                    QString filter = mGlobalDatastore->getGridString("equipments", "filter",    "name", equipmentName);
                    QString guidecamera = mGlobalDatastore->getGridString("equipments", "guidecamera",    "name", equipmentName);
                    QString guidest4 = mGlobalDatastore->getGridString("equipments", "guidest4",    "name", equipmentName);
                    QString gps = mGlobalDatastore->getGridString("equipments", "gps",    "name", equipmentName);
                    QString hostport = mGlobalDatastore->getGridString("equipments", "host",    "name", equipmentName);
                    if (camera != "")
                        logInfo("Equipments: %1", {equipmentName});
                    else
                        logWarning("Equipments '%1' not found in GlobalDatastore", {equipmentName});
                    if (getProperty("devices")->getElts()->contains("camera")) getEltString("devices", "camera")->setValue(camera);
                    if (getProperty("devices")->getElts()->contains("mount")) getEltString("devices", "mount")->setValue(mount);
                    if (getProperty("devices")->getElts()->contains("focuser")) getEltString("devices", "focuser")->setValue(focuser);
                    if (getProperty("devices")->getElts()->contains("filter")) getEltString("devices", "filter")->setValue(filter);
                    if (getProperty("devices")->getElts()->contains("guidecamera")) getEltString("devices",
                                "guidecamera")->setValue(guidecamera);
                    if (getProperty("devices")->getElts()->contains("guidest4")) getEltString("devices", "guidest4")->setValue(guidest4);
                    if (getProperty("devices")->getElts()->contains("gps")) getEltString("devices", "gps")->setValue(gps);

                    QString host = mGlobalDatastore->getGridString("servers", "host",    "name", hostport);
                    int port = mGlobalDatastore->getGridInt("servers", "port", "name", hostport);
                    if (host != "")
                        logInfo("Servers: %1 : %2:%3", {hostport, host, port});
                    else
                        logWarning("Servers '%1' not found in GlobalDatastore", {hostport});
                    getEltString("server", "host")->setValue(host);
                    getEltInt("server", "port")->setValue(port);
                }
            }
            connectIndi();
        }


        if (event.prpkey == "devicesactions" && event.eltkey == "condevs")
        {
            if (!isServerConnected())
            {
                logWarning("%1 - Indi server not connected", {QString("onExternalEventIndi")});
                getProperty(event.prpkey)->setState(OST::Error, true);
                return true;
            }

            if (event.eltkey == "condevs")
            {
                getProperty(event.prpkey)->setState(OST::Busy, true);
                if (connectAllDevices()) getProperty(event.prpkey)->setState(OST::Ok, true);
                else getProperty(event.prpkey)->setState(OST::Error, true);

                OST::ElementUpdate u;
                bool b = true;
                getStore()[event.prpkey]->getElt(event.eltkey)->accept(&u, eltval, b);
                return true;
            }
            if (event.eltkey == "discondevs")
            {
                getProperty(event.prpkey)->setState(OST::Busy, true);
                if (disconnectAllDevices()) getProperty(event.prpkey)->setState(OST::Ok, true);
                else getProperty(event.prpkey)->setState(OST::Error, true);

                OST::ElementUpdate u;
                bool b = true;
                getStore()[event.prpkey]->getElt(event.eltkey)->accept(&u, eltval, b);
                return true;
            }
            if (event.eltkey == "loadconfs")
            {
                getProperty(event.prpkey)->setState(OST::Busy, true);
                if (loadDevicesConfs()) getProperty(event.prpkey)->setState(OST::Ok, true);
                else getProperty(event.prpkey)->setState(OST::Error, true);

                OST::ElementUpdate u;
                bool b = true;
                getStore()[event.prpkey]->getElt(event.eltkey)->accept(&u, eltval, b);
                return true;
            }
        }

    }

    if (event.ev == OST::ExtEvType::J2 && event.prpkey == "devices")
    {
        refreshDeviceslovs();
    }
    return true;

}
void IndiModule::connectIndiTimer()
{
    if (isServerConnected())
    {
        return;
    }
    connectIndi();

}

void IndiModule::newDevice(INDI::BaseDevice dp)
{
    //logDebug("IndiModule::newDevice %1", {dp.getDeviceName()});
    refreshDeviceslovs();
    onNewDevice(dp);
}
void IndiModule::removeDevice(INDI::BaseDevice dp)
{
    //logDebug("IndiModule::removeDevice %1", {dp.getDeviceName()});
    refreshDeviceslovs();
    onRemoveDevice(dp);
}
void IndiModule::newProperty(INDI::Property pProperty)
{
    //logDebug("IndiModule::newProperty %1 %2", {pProperty.getDeviceName(), pProperty.getName()});
    onNewProperty(pProperty);
}

void IndiModule::updateProperty (INDI::Property property)
{
    //logDebug("IndiModule::updateProperty %1 %2", {property.getDeviceName(), property.getName()});
    onUpdateProperty(property);

}
void IndiModule::removeProperty(INDI::Property property)
{
    //logDebug("IndiModule::removeProperty %1 %2", {property.getDeviceName(), property.getName()});
    onRemoveProperty(property);
}

void IndiModule::onNewDevice      (INDI::BaseDevice dp)    {} ;
void IndiModule::onRemoveDevice   (INDI::BaseDevice dp)    {} ;
void IndiModule::onNewProperty    (INDI::Property property) {} ;
void IndiModule::onRemoveProperty (INDI::Property property) {} ;
void IndiModule::onUpdateProperty (INDI::Property property) {} ;

/*!
 * Connects to indi server
 * Should we add host/port ??
 * IMHO this would be overkill as headless instance of OST and indiserver should be 99% hosted on the same machine
 * It would be easy to manage 1% remaining with indiserver chaining capabilities
 */
bool IndiModule::connectIndi()
{
    if (isServerConnected())
    {
        //sendWarning("Indi server already connected");
        newUniversalMessage("Indi server already connected");
        return true;
    }
    setServer(getString("server", "host").toStdString().c_str(), getInt("server", "port"));
    if (connectServer())
    {
        newUniversalMessage("Indi server connected");
        logInfo("Indi server connected");
        QTimer::singleShot(500, this, &IndiModule::OnAfterIndiConnectIndiTimer);
        return true;
    }
    logError("%1 - Couldn't connect to Indi server", {QString("connectIndi")});
    return false;
}
bool IndiModule::disconnectIndi(void)
{
    if (!isServerConnected())
    {
        logWarning("%1 - Indi server already disconnected", {QString("disconnectIndi")});
        return true;
    }
    if (!disconnectServer())
    {
        logError("%1 - Couldn't disconnect from Indi server", {QString("disconnectIndi")});
        return false;
    }
    logInfo("%1 - Indi server disconnected", {QString("disconnectIndi")});
    return true;

}
/*!
 * Asks every device to connect
 */
bool IndiModule::connectAllDevices()
{
    int err = 0;
    std::vector<INDI::BaseDevice> devs = getDevices();

    for(std::size_t i = 0; i < devs.size(); i++)
    {
        INDI::PropertySwitch svp = devs[i].getSwitch("CONNECTION");

        if (!svp.isValid())
        {
            logError("%1 - Couldn't find CONNECTION switch for %2", {QString("connectAllDevices"), QString(devs[i].getDeviceName())});
            err++;
        }
        else
        {
            for (std::size_t j = 0; j < svp.size(); j++)
            {
                if (strcmp(svp[j].name, "CONNECT") == 0)
                {
                    svp[j].s = ISS_ON;
                }
                else
                {
                    svp[j].s  = ISS_OFF;
                }
            }

            sendNewSwitch(svp);
            if (devs[i].getDriverInterface() & INDI::BaseDevice::CCD_INTERFACE)
            {
                logDebug("Set blob mode %1", {QString(devs[i].getDeviceName())});
                setBLOBMode(B_ALSO, devs[i].getDeviceName(), nullptr);

            }

        }

    }
    if (err == 0) return true;
    else return false;

}
/*!
 * Asks every device to disconnect
 */
bool IndiModule::disconnectAllDevices()
{
    int err = 0;
    std::vector<INDI::BaseDevice> devs = getDevices();

    for(std::size_t i = 0; i < devs.size(); i++)
    {
        INDI::PropertySwitch svp = devs[i].getSwitch("CONNECTION");

        if (!svp.isValid())
        {
            logError("%1 - Couldn't find CONNECTION switch for %2", {QString("disconnectAllDevices"), QString(devs[i].getDeviceName())});
            err++;
        }
        else
        {
            for (std::size_t j = 0; j < svp.size(); j++)
            {
                if (strcmp(svp[j].name, "DISCONNECT") == 0)
                {
                    svp[j].s = ISS_ON;
                }
                else
                {
                    svp[j].s = ISS_OFF;
                }
            }
            sendNewSwitch(svp);

        }

    }
    if (err == 0) return true;
    else return false;

}
bool IndiModule::connectDevice(QString deviceName)
{
    if (!isServerConnected())
    {
        logWarning("%1 - Indi server is not connected", {QString("connectDevice")});
        return false;
    }
    bool _checkdevice = false;
    foreach (INDI::BaseDevice dd, getDevices())
    {
        if (strcmp(dd.getDeviceName(), deviceName.toStdString().c_str()) == 0) _checkdevice = true;
    }
    if (!_checkdevice)
    {
        logError("%1 - Unable to find %2 device", {QString("connectDevice"), deviceName});
        return false;
    }

    INDI::BaseDevice dev = getDevice(deviceName.toStdString().c_str());

    INDI::PropertySwitch svp = dev.getSwitch("CONNECTION");

    if (!svp.isValid())
    {
        logError("%1 - Couldn't find CONNECTION switch for %2", {QString("connectDevice"), deviceName});
        return false;
    }
    for (std::size_t  j = 0; j < svp.size(); j++)
    {
        if (strcmp(svp[j].name, "CONNECT") == 0)
        {
            svp[j].setState(ISS_ON);
        }
        else
        {
            svp[j].setState(ISS_OFF);
        }
    }
    sendNewSwitch(svp);
    return true;
}
bool IndiModule::disconnectDevice(QString deviceName)
{
    INDI::BaseDevice dev = getDevice(deviceName.toStdString().c_str());

    INDI::PropertySwitch svp = dev.getSwitch("CONNECTION");

    if (!svp.isValid())
    {
        logError("%1 - Couldn't find CONNECTION switch for %2", {QString("disconnectDevice"), deviceName});
        return false;
    }
    for (std::size_t j = 0; j < svp.size(); j++)
    {
        if (strcmp(svp[j].name, "DISCONNECT") == 0)
        {
            svp[j].setState(ISS_ON);
        }
        else
        {
            svp[j].setState(ISS_OFF);
        }
    }
    sendNewSwitch(svp);
    return true;

}
/*!
 * Asks every device to load saved configuration
 */
bool IndiModule::loadDevicesConfs()
{
    int err = 0;
    std::vector<INDI::BaseDevice> devs = getDevices();
    for(std::size_t i = 0; i < devs.size(); i++)
    {
        logInfo("%1 - Loading device conf for %2", {QString("loadDevicesConfs"), QString(devs[i].getDeviceName())});
        if (devs[i].isConnected())
        {
            INDI::PropertySwitch svp = devs[i].getSwitch("CONFIG_PROCESS");

            if (!svp.isValid())
            {
                logError("%1 - Couldn't find CONFIG_PROCESS switch for %2", {QString("loadDevicesConfs"), QString(devs[i].getDeviceName())});
                err++;
            }
            else
            {
                for (std::size_t j = 0; j < svp.size(); j++)
                {
                    if (strcmp(svp[j].name, "CONFIG_LOAD") == 0)
                    {
                        svp[j].setState(ISS_ON);
                    }
                    else
                    {
                        svp[j].setState(ISS_OFF);
                    }
                }
                sendNewSwitch(svp);
            }

        }
    }
    if (err == 0) return true;
    else return false;

}
auto IndiModule::sendModNewNumber(const QString &deviceName, const QString &propertyName, const QString  &elementName,
                                  const double &value) -> bool
{
    //qDebug() << "sendModNewNumber" << " " << deviceName << "-" << propertyName << "-" << elementName << "-" << value;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        logError("%1 - Unable to find %2 device. Aborting.", {QString("sendModNewNumber"), deviceName});
        return false;
    }
    INDI::PropertyNumber prop = dp.getNumber(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        logError("%1 - Unable to find %2/%3 property. Aborting.", {QString("sendModNewNumber"), deviceName, propertyName});
        return false;
    }

    for (std::size_t i = 0; i < prop.size(); i++)
    {
        if (strcmp(prop[i].name, elementName.toStdString().c_str()) == 0)
        {
            prop.findWidgetByName(elementName.toStdString().c_str())->value = value;
            sendNewNumber(prop);
            return true;
        }
    }
    logError("%1 - Unable to find %2/%3/%4 element. Aborting.", {QString("sendModNewNumber"), deviceName, propertyName, elementName});
    return false;

}
bool IndiModule::requestCapture(const QString &deviceName, const double &exposure, const double &gain, const double &offset)
{
    double wgain = gain;
    double woffset = offset;
    double wexpose = exposure;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());
    if (!dp.isValid())
    {
        logError("%1 - Unable to find %2 device. Aborting.", {QString("requestCapture"), deviceName});
        return false;
    }
    if (dp == false)
    {
        logWarning("%1 - %2 not connected, trying to connect", {QString("requestCapture"), deviceName});
        if (!connectDevice(deviceName)) return false;
    }
    setBLOBMode(B_ALSO, deviceName.toStdString().c_str(), nullptr);
    if(dp.getProperty("CCD_CONTROLS"))
    {
        INDI::PropertyNumber prop = dp.getNumber("CCD_CONTROLS");
        if (wgain < prop.findWidgetByName("Gain")->min)
        {
            logWarning("%1 - %2 gain requested too low (%3), setting to %4", {QString("requestCapture"), deviceName, wgain, prop.findWidgetByName("Gain")->min});
            wgain = prop.findWidgetByName("Gain")->min;
        }
        if (wgain > prop.findWidgetByName("Gain")->max)
        {
            logWarning("%1 - %2 gain requested too high (%3), setting to %4", {QString("requestCapture"), deviceName, wgain, prop.findWidgetByName("Gain")->max});
            wgain = prop.findWidgetByName("Gain")->max;
        }
        if (woffset < prop.findWidgetByName("Offset")->min)
        {
            logWarning("%1 - %2 offset requested too low (%3), setting to %4", {QString("requestCapture"), deviceName, wgain, prop.findWidgetByName("Offset")->min});
            woffset = prop.findWidgetByName("Offset")->min;
        }
        if (woffset > prop.findWidgetByName("Offset")->max)
        {
            logWarning("%1 - %2 offset requested too high (%3), setting to %4", {QString("requestCapture"), deviceName, wgain, prop.findWidgetByName("Offset")->max});
            woffset = prop.findWidgetByName("Offset")->max;
        }
        prop.findWidgetByName("Gain")->value = wgain;
        prop.findWidgetByName("Offset")->value = woffset;
        sendNewNumber(prop);
    }
    if(dp.getProperty("CCD_GAIN"))
    {
        INDI::PropertyNumber prop = dp.getNumber("CCD_GAIN");
        if (wgain < prop.findWidgetByName("GAIN")->min)
        {
            logWarning("%1 - %2 gain requested too low (%3), setting to %4", {QString("requestCapture"), deviceName, wgain, prop.findWidgetByName("GAIN")->min});
            wgain = prop.findWidgetByName("GAIN")->min;
        }
        if (wgain > prop.findWidgetByName("GAIN")->max)
        {
            logWarning("%1 - %2 gain requested too high (%3), setting to %4", {QString("requestCapture"), deviceName, wgain, prop.findWidgetByName("GAIN")->max});
            wgain = prop.findWidgetByName("GAIN")->max;
        }
        prop.findWidgetByName("GAIN")->value = wgain;
        sendNewNumber(prop);
    }
    if(dp.getProperty("CCD_OFFSET"))
    {
        INDI::PropertyNumber prop = dp.getNumber("CCD_OFFSET");
        if (woffset < prop.findWidgetByName("OFFSET")->min)
        {
            logWarning("%1 - %2 offset requested too low (%3), setting to %4", {QString("requestCapture"), deviceName, wgain, prop.findWidgetByName("OFFSET")->min});
            woffset = prop.findWidgetByName("OFFSET")->min;
        }
        if (woffset > prop.findWidgetByName("OFFSET")->max)
        {
            logWarning("%1 - %2 offset requested too high (%3), setting to %4", {QString("requestCapture"), deviceName, wgain, prop.findWidgetByName("OFFSET")->max});
            woffset = prop.findWidgetByName("OFFSET")->max;
        }
        prop.findWidgetByName("OFFSET")->value = woffset;
        sendNewNumber(prop);
    }

    if (!sendModNewNumber(deviceName, "CCD_EXPOSURE", "CCD_EXPOSURE_VALUE", wexpose)) return false;
    emit requestCaptureDone();
    return true;


}
bool IndiModule::getModNumber(const QString &deviceName, const QString &propertyName, const QString  &elementName,
                              double &value)
{
    //qDebug() << "getModNumber" << " " << deviceName << " " << propertyName<< " " << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        logError("%1 - Unable to find %2 device. Aborting.", {QString("getModNumber"), deviceName});
        return false;
    }
    INDI::PropertyNumber prop = dp.getNumber(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        logError("%1 - Unable to find %2/%3 property. Aborting.", {QString("getModNumber"), deviceName, propertyName});
        return false;
    }

    for (std::size_t i = 0; i < prop.size(); i++)
    {
        if (strcmp(prop[i].name, elementName.toStdString().c_str()) == 0)
        {
            value = prop.findWidgetByName(elementName.toStdString().c_str())->getValue();
            return true;
        }
    }
    logError("%1 - Unable to find %2/%3/%4 element. Aborting.", {QString("getModNumber"), deviceName, propertyName, elementName});

    return false;
}
bool IndiModule::getModSwitch(const QString &deviceName, const QString &propertyName, const QString  &elementName,
                              bool &value)
{
    //qDebug() << "getModswitch " << "-" << deviceName << "-" << propertyName<< "-" << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        logError("%1 - Unable to find %2 device. Aborting.", {QString("getModSwitch"), deviceName});
        return false;
    }
    INDI::PropertySwitch prop = dp.getSwitch(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        logError("%1 - Unable to find %2/%3 property. Aborting.", {QString("getModSwitch"), deviceName, propertyName});
        return false;
    }

    for (std::size_t i = 0; i < prop.size(); i++)
    {
        if (strcmp(prop[i].name, elementName.toStdString().c_str()) == 0)
        {
            value = prop.findWidgetByName(elementName.toStdString().c_str())->getState();
            return true;
        }
    }
    logError("%1 - Unable to find %2/%3/%4 element. Aborting.", {QString("getModSwitch"), deviceName, propertyName, elementName});
    return false;
}
bool IndiModule::getModText(const QString &deviceName, const QString &propertyName, const QString  &elementName,
                            QString &value)
{
    //qDebug() << "getModText" << "-" << deviceName << "-" << propertyName<< "-" << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        logError("%1 - Unable to find %2 device. Aborting.", {QString("getModText"), deviceName});
        return false;
    }
    INDI::PropertyText prop = dp.getText(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        logError("%1 - Unable to find %2/%3 property. Aborting.", {QString("getModText"), deviceName, propertyName});
        return false;
    }

    for (std::size_t i = 0; i < prop.size(); i++)
    {
        if (strcmp(prop[i].name, elementName.toStdString().c_str()) == 0)
        {
            value = prop.findWidgetByName(elementName.toStdString().c_str())->getText();
            return true;
        }
    }
    logError("%1 - Unable to find %2/%3/%4 element. Aborting.", {QString("getModText"), deviceName, propertyName, elementName});
    return false;
}
bool IndiModule::sendModNewText  (QString deviceName, QString propertyName, QString elementName, QString text)
{
    //qDebug() << "sendModNewText" << "-" << deviceName << "-" << propertyName<< "-" << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        logError("%1 - Unable to find %2 device. Aborting.", {QString("sendModNewText"), deviceName});
        return false;
    }
    INDI::PropertyText prop = dp.getText(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        logError("%1 - Unable to find %2/%3 property. Aborting.", {QString("sendModNewText"), deviceName, propertyName});
        return false;
    }

    for (std::size_t i = 0; i < prop.size(); i++)
    {
        if (strcmp(prop[i].name, elementName.toStdString().c_str()) == 0)
        {
            prop.findWidgetByName(elementName.toStdString().c_str())->setText(text.toStdString());
            sendNewText(prop);
            return true;
        }
    }
    logError("%1 - Unable to find %2/%3/%4 element. Aborting.", {QString("sendModNewText"), deviceName, propertyName, elementName});
    return false;
}
bool IndiModule::sendModNewSwitch(QString deviceName, QString propertyName, QString elementName, ISState sw)
{
    //qDebug() << "sendModNewswitch " << "-" << deviceName << "-" << propertyName<< "-" << elementName;

    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        logError("%1 - Unable to find %2 device. Aborting.", {QString("sendModNewSwitch"), deviceName});
        return false;
    }
    INDI::PropertySwitch prop = dp.getSwitch(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        logError("%1 - Unable to find %2/%3 property. Aborting.", {QString("sendModNewSwitch"), deviceName, propertyName});
        return false;
    }
    for (std::size_t i = 0; i < prop.size(); i++)
    {
        if (prop.getRule() == ISR_1OFMANY) prop[i].s = ISS_OFF;
        if (strcmp(prop[i].name, elementName.toStdString().c_str()) == 0)
        {
            prop.findWidgetByName(elementName.toStdString().c_str())->setState(sw);
            sendNewSwitch(prop);
            return true;
        }
    }
    logError("%1 - Unable to find %2/%3/%4 element. Aborting.", {QString("sendModNewSwitch"), deviceName, propertyName, elementName});
    return false;

}
bool IndiModule::frameSet(QString devicename, double x, double y, double width, double height)
{
    //qDebug() << "frameSet" << "-" << devicename;
    INDI::BaseDevice dp = getDevice(devicename.toStdString().c_str());
    if (!dp.isValid())
    {
        logError("%1 - Unable to find %2 device. Aborting.", {QString("frameSet"), devicename});
        return false;
    }

    INDI::PropertyNumber prop = dp.getNumber("CCD_FRAME");
    if (!prop.isValid())
    {
        logError("%1 - Unable to find %2/%3 property. Aborting.", {QString("frameSet"), devicename, QString("CCD_FRAME")});
        return false;
    }

    for (std::size_t i = 0; i < prop.size(); i++)
    {
        if (strcmp(prop[i].name, "X") == 0)
        {
            prop[i].value = x;
        }
        if (strcmp(prop[i].name, "Y") == 0)
        {
            prop[i].value = y;
        }
        if (strcmp(prop[i].name, "WIDTH") == 0)
        {
            prop[i].value = width;
        }
        if (strcmp(prop[i].name, "HEIGHT") == 0)
        {
            prop[i].value = height;
        }
    }

    sendNewNumber(prop);
    return true;
}
bool IndiModule::frameReset(QString devicename)
{
    //qDebug() << "frameReset" << "-" << devicename;
    INDI::BaseDevice dp = getDevice(devicename.toStdString().c_str());
    if (!dp.isValid())
    {
        logError("%1 - Unable to find %2 device. Aborting.", {QString("frameReset"), devicename});
        return false;
    }

    INDI::PropertySwitch prop = dp.getSwitch("CCD_FRAME_RESET");
    if (!prop.isValid())
    {
        logError("%1 - Unable to find %2/%3 property. Aborting.", {QString("frameReset"), devicename, QString("CCD_FRAME_RESET")});
        return false;
    }

    for (std::size_t i = 0; i < prop.size(); i++)
    {
        if (strcmp(prop[i].name, "RESET") == 0)
        {
            prop[i].s = ISS_ON;
        }
    }

    sendNewSwitch(prop);
    emit askedFrameReset(devicename);
    return true;
}
bool IndiModule::createDeviceProperty(const QString &key, const QString &label, const QString &level1,
                                      const QString &level2, const QString &order, INDI::BaseDevice::DRIVER_INTERFACE interface)
{
    std::vector<INDI::BaseDevice> devs = getDevices();
    OST::PropertyMulti* pm = new OST::PropertyMulti(key, label, OST::ReadWrite, level1, level2, order, true, false);
    OST::ElementString* s = new  OST::ElementString("name", "name", "", "");
    s->setValue("--", false);
    s->setAutoUpdate(true);
    pm->addElt(s);

    for(std::size_t i = 0; i < devs.size(); i++)
    {
        if (devs[i].getDriverInterface() & interface)
        {
            s->lovAdd(devs[i].getDeviceName(), devs[i].getDeviceName());
        }
        /*sendMessage("------------ list devs " + QString(devs[i].getDeviceName()));
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::GENERAL_INTERFACE) sendMessage("GENERAL_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::TELESCOPE_INTERFACE)
            sendMessage("TELESCOPE_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::CCD_INTERFACE) sendMessage("CCD_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::GUIDER_INTERFACE) sendMessage("GUIDER_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::FOCUSER_INTERFACE) sendMessage("FOCUSER_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::FILTER_INTERFACE) sendMessage("FILTER_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::DOME_INTERFACE) sendMessage("DOME_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::GPS_INTERFACE) sendMessage("GPS_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::WEATHER_INTERFACE) sendMessage("WEATHER_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::AO_INTERFACE) sendMessage("AO_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::DUSTCAP_INTERFACE) sendMessage("DUSTCAP_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::LIGHTBOX_INTERFACE)
            sendMessage("LIGHTBOX_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::DETECTOR_INTERFACE)
            sendMessage("DETECTOR_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::ROTATOR_INTERFACE) sendMessage("ROTATOR_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::SPECTROGRAPH_INTERFACE)
            sendMessage("SPECTROGRAPH_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::CORRELATOR_INTERFACE)
            sendMessage("CORRELATOR_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::AUX_INTERFACE) sendMessage("AUX_INTERFACE");
        if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::SENSOR_INTERFACE) sendMessage("SENSOR_INTERFACE");
        sendMessage("----------------------------");*/

    }
    createProperty(pm);
    return true;
}
void IndiModule::OnAfterIndiConnectIndiTimer()
{
    std::vector<INDI::BaseDevice> devs = getDevices();
    for(std::size_t i = 0; i < devs.size(); i++)
    {
        QString d = devs[i].getDeviceName();
        if (!devs[i].isConnected())
        {
            connectDevice(d);
        }
    }
    refreshDeviceslovs();
}
bool IndiModule::refreshDeviceslovs()
{
    getGlovString("DRIVER_INTERFACE-GENERAL_INTERFACE")->lovClear();
    getGlovString("DRIVER_INTERFACE-CCD_INTERFACE")->lovClear();
    getGlovString("DRIVER_INTERFACE-TELESCOPE_INTERFACE")->lovClear();
    getGlovString("DRIVER_INTERFACE-GUIDER_INTERFACE")->lovClear();
    getGlovString("DRIVER_INTERFACE-FOCUSER_INTERFACE")->lovClear();
    getGlovString("DRIVER_INTERFACE-FILTER_INTERFACE")->lovClear();
    getGlovString("DRIVER_INTERFACE-GPS_INTERFACE")->lovClear();
    getGlovString("DRIVER_INTERFACE-WEATHER_INTERFACE")->lovClear();
    getGlovString("DRIVER_INTERFACE-AUX_INTERFACE")->lovClear();


    std::vector<INDI::BaseDevice> devs = getDevices();
    for(std::size_t i = 0; i < devs.size(); i++)
    {
        QString d = devs[i].getDeviceName();
        if (devs[i].isConnected())
        {
            getGlovString("DRIVER_INTERFACE-GENERAL_INTERFACE")->lovAdd(d, d);

            if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::CCD_INTERFACE)
            {
                getGlovString("DRIVER_INTERFACE-CCD_INTERFACE")->lovAdd(d, d);
            }
            if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::TELESCOPE_INTERFACE)
            {
                getGlovString("DRIVER_INTERFACE-TELESCOPE_INTERFACE")->lovAdd(d, d);
            }
            if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::GUIDER_INTERFACE)
            {
                getGlovString("DRIVER_INTERFACE-GUIDER_INTERFACE")->lovAdd(d, d);
            }
            if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::FOCUSER_INTERFACE)
            {
                getGlovString("DRIVER_INTERFACE-FOCUSER_INTERFACE")->lovAdd(d, d);
            }
            if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::FILTER_INTERFACE)
            {
                getGlovString("DRIVER_INTERFACE-FILTER_INTERFACE")->lovAdd(d, d);
            }
            if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::GPS_INTERFACE)
            {
                getGlovString("DRIVER_INTERFACE-GPS_INTERFACE")->lovAdd(d, d);
            }
            if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::WEATHER_INTERFACE)
            {
                getGlovString("DRIVER_INTERFACE-WEATHER_INTERFACE")->lovAdd(d, d);
            }
            if (devs[i].getDriverInterface() & INDI::BaseDevice::DRIVER_INTERFACE::AUX_INTERFACE)
            {
                getGlovString("DRIVER_INTERFACE-AUX_INTERFACE")->lovAdd(d, d);
            }
        }
    }
    return true;
}
bool IndiModule::defineMeAsFocuser()
{
    defineMeAsImager();
    giveMeAnActions();

    OST::PropertyMulti* pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ElementBool* b = new  OST::ElementBool("abortfocus", "Abort focus", "foc99", "");
    b->setValue(false, false);
    b->setPreIcon("stop");
    pm->addElt(b);
    b = new  OST::ElementBool("autofocus", "Autofocus", "foc50", "");
    b->setValue(false, false);
    b->setPreIcon("play_arrow");
    pm->addElt(b);
    b = new  OST::ElementBool("loop", "Loop", "foc70", "");
    b->setValue(false, false);
    b->setPreIcon("forward_media");
    pm->addElt(b);
    mIsFocuser = true;
    return true;

}
bool IndiModule::defineMeAsGuider()
{
    defineMeAsImager();
    giveMeAnActions();

    OST::PropertyMulti* pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ElementBool* b = new  OST::ElementBool("abortguider", "Abort guider", "guid99", "");
    b->setValue(false, false);
    pm->addElt(b);
    b->setPreIcon("stop");
    b = new  OST::ElementBool("guide", "Guide", "guid50", "");
    b->setValue(false, false);
    b->setPreIcon("play_arrow");
    pm->addElt(b);
    b = new  OST::ElementBool("calibrate", "Calibrate", "guid00", "");
    b->setValue(false, false);
    b->setPreIcon("arrows_output");
    pm->addElt(b);
    mIsGuider = true;
    return true;

}
bool IndiModule::defineMeAsSequencer()
{
    defineMeAsImager();
    giveMeAnActions();

    OST::PropertyMulti* pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ElementBool* b = new  OST::ElementBool("abortsequence", "Abort sequence", "seq99", "");
    b->setValue(false, false);
    b->setPreIcon("stop");
    pm->addElt(b);
    b = new  OST::ElementBool("startsequence", "Start sequence", "seq00", "");
    b->setValue(false, false);
    pm->addElt(b);
    b->setPreIcon("play_arrow");
    mIsSequencer = true;
    return true;

}
bool IndiModule::defineMeAsImager()
{
    giveMeAnOptic();
    if (!getStore().contains("image"))
    {
        OST::PropertyMulti* dynprop = new OST::PropertyMulti("image", "Image", OST::Permission::WriteOnly, "Control",
                "", "000Control222", false, false);
        createProperty(dynprop);
    }

    if (getStore().contains("image"))
    {
        if (!(getStore()["image"]->getElts()->contains("image")))
        {
            OST::PropertyMulti * pm = getProperty("image");
            OST::ElementImg* img = new OST::ElementImg("image", "", "00", "");
            pm->addElt(img);
        }

    }

    OST::PropertyMulti * pm = getProperty("parms");
    pm->setRule(OST::SwitchsRule::Any);

    if (!getStore()["parms"]->getElts()->contains("exposure"))
    {
        OST::ElementFloat* f = new  OST::ElementFloat("exposure", "Exposure", "exp000", "");
        f->setValue(0, false);
        f->setDirectEdit(true);
        f->setAutoUpdate(true);
        f->setMinMax(0.00001, 300, false);
        f->setStep(0.001, false);
        f->setSlider(OST::SliderAndValue, false);
        pm->addElt(f);
    }



    if (!getStore()["parms"]->getElts()->contains("gain"))
    {
        OST::ElementInt* i  = new  OST::ElementInt("gain", "Gain", "exp020", "");
        i->setValue(0, false);
        i->setDirectEdit(true);
        i->setAutoUpdate(true);
        i->setMinMax(0, 500, false);
        i->setStep(1, false);
        i->setSlider(OST::SliderAndValue, false);
        pm->addElt(i);
    }

    if (!getStore()["parms"]->getElts()->contains("offset"))
    {
        OST::ElementInt* i  = new  OST::ElementInt("offset", "Offset", "exp030", "");
        i->setValue(0, false);
        i->setDirectEdit(true);
        i->setAutoUpdate(true);
        i->setMinMax(0, 500, false);
        i->setStep(1, false);
        i->setSlider(OST::SliderAndValue, false);
        pm->addElt(i);
    }

    mIsImager = true;
    return true;

}
bool IndiModule::defineMeAsNavigator()
{
    defineMeAsImager();
    giveMeATarget();
    giveMeAnActions();

    OST::PropertyMulti* pm  = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ElementBool* b = new  OST::ElementBool("abortnavigator", "Abort navigator", "nav99", "");
    b->setValue(false, false);
    b->setPreIcon("stop");
    pm->addElt(b);
    b = new  OST::ElementBool("gototarget", "Center target", "nav00", "");
    b->setPreIcon("arrows_input");
    b->setValue(false, false);
    pm->addElt(b);
    b = new  OST::ElementBool("addtoplanner", "Add to planner", "nav20", "");
    b->setPreIcon("event_upcoming");
    b->setValue(false, false);
    pm->addElt(b);

    OST::PropertyMulti* pmtarget  = getProperty("target");
    OST::ElementString* s = new  OST::ElementString("targetname", "Target name", "nav50", "");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pmtarget->addElt(s);
    OST::ElementFloat* f = new  OST::ElementFloat("targetra", "Target RA", "nav51", "");
    f->setDirectEdit(true);
    f->setAutoUpdate(true);
    pmtarget->addElt(f);
    f = new  OST::ElementFloat("targetde", "Target DEC", "nav52", "");
    f->setDirectEdit(true);
    f->setAutoUpdate(true);
    pmtarget->addElt(f);

    pm  = getProperty("parms");
    s = new  OST::ElementString("plannermodule", "Planner module", "nav30", "");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    s->setGlobalLov("loadedModules-planner", OST::LovScope::Controller);
    pm->addElt(s);

    mIsNavigator = true;
    return true;

}
double IndiModule::getPixelSize(const QString &deviceName)
{
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());
    if (!dp.isValid())
    {
        logError("%1 - Unable to find %2 device. Aborting.", {QString("getPixelSize"), deviceName});
        return 0;
    }
    if (!dp.isConnected())
    {
        logWarning("%1 - %2 not connected, trying to connect", {QString("getPixelSize"), deviceName});
        if (!connectDevice(deviceName)) return 0;
    }
    INDI::PropertyNumber prop = dp.getNumber("CCD_INFO");
    float f = prop.findWidgetByName("CCD_PIXEL_SIZE")->getValue();
    //logDebug("getPixelSize result : %1 %2",{deviceName,f});
    return f;

}
double IndiModule::getSampling()
{
    if (!isImager())
    {
        logWarning("%1 - module is not defined as an imager - defaults to 1", {QString("getSampling")});
        return 1;
    }
    return 206 * getPixelSize(getString("devices", "camera")) / (getFloat("optic", "fl") * getFloat("optic", "red"));
}

bool IndiModule::giveMeADevice(QString name, QString label, INDI::BaseDevice::DRIVER_INTERFACE interface)
{
    giveMeAnEquipment();

    OST::PropertyMulti* pm = getProperty("devices");
    OST::ElementString* s = new  OST::ElementString(name, label, label, "");
    s->setPostIcon("refresh");
    switch (interface)
    {
        case INDI::BaseDevice::DRIVER_INTERFACE::GENERAL_INTERFACE:
            s->setGlobalLov("DRIVER_INTERFACE-GENERAL_INTERFACE");
            break;
        case INDI::BaseDevice::DRIVER_INTERFACE::CCD_INTERFACE:
            s->setGlobalLov("DRIVER_INTERFACE-CCD_INTERFACE");
            break;
        case INDI::BaseDevice::DRIVER_INTERFACE::TELESCOPE_INTERFACE:
            s->setGlobalLov("DRIVER_INTERFACE-TELESCOPE_INTERFACE");
            break;
        case INDI::BaseDevice::DRIVER_INTERFACE::GUIDER_INTERFACE:
            s->setGlobalLov("DRIVER_INTERFACE-GUIDER_INTERFACE");
            break;
        case INDI::BaseDevice::DRIVER_INTERFACE::FOCUSER_INTERFACE:
            s->setGlobalLov("DRIVER_INTERFACE-FOCUSER_INTERFACE");
            break;
        case INDI::BaseDevice::DRIVER_INTERFACE::FILTER_INTERFACE:
            s->setGlobalLov("DRIVER_INTERFACE-FILTER_INTERFACE");
            break;
        case INDI::BaseDevice::DRIVER_INTERFACE::GPS_INTERFACE:
            s->setGlobalLov("DRIVER_INTERFACE-GPS_INTERFACE");
            break;
        case INDI::BaseDevice::DRIVER_INTERFACE::WEATHER_INTERFACE:
            s->setGlobalLov("DRIVER_INTERFACE-WEATHER_INTERFACE");
            break;
        case INDI::BaseDevice::DRIVER_INTERFACE::AUX_INTERFACE:
            s->setGlobalLov("DRIVER_INTERFACE-AUX_INTERFACE");
            break;
        default:
            logWarning("%1 - unhandled interface type", {QString("giveMeADevice")});
            break;
    }

    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addElt(s);
    return true;

}
bool IndiModule::giveMeAnOptic()
{
    if (getStore().contains("optic")) return false;

    OST::PropertyMulti* pm = new OST::PropertyMulti("optic", "Optic", OST::ReadWrite, "Parameters", "General", "222Parms333",
            true,
            false);
    createProperty(pm);

    OST::ElementFloat* f = new  OST::ElementFloat("fl", "Focal length", "1", "");
    f->setDirectEdit(true);
    f->setMinMax(1, 4000, false);
    f->setAutoUpdate(true);
    pm->addElt(f);
    OST::ElementFloat* d = new  OST::ElementFloat("diam", "Diameter", "2", "");
    d->setDirectEdit(true);
    d->setMinMax(1, 4000, false);
    d->setAutoUpdate(true);
    pm->addElt(d);
    OST::ElementFloat* r = new  OST::ElementFloat("red", "Reducer", "3", "");
    r->setDirectEdit(true);
    r->setMinMax(0.1, 10, false);
    r->setAutoUpdate(true);
    r->setValue(1);
    pm->addElt(r);
    OST::ElementString* o = new  OST::ElementString("optic", "Global optic", "0", "");
    o->setDirectEdit(true);
    o->setAutoUpdate(true);
    o->setValue("");
    o->setGlobalLov("optics", OST::LovScope::Controller);
    pm->addElt(o);
    mIsOptic = true;
    return true;

}
bool IndiModule::setFocalLengthAndDiameter()
{
    double fl = getFloat("optic", "fl");
    double diam = getFloat("optic", "diam");
    QString dev = getString("devices", "camera");
    return setFocalLengthAndDiameter(dev, fl, diam);
}

bool IndiModule::setFocalLengthAndDiameter(QString device, double fl, double diam)
{
    //logDebug("setFocalLengthAndDiameter %1 %2 %3", {device, fl, diam});

    if (fl < 1)
    {
        logWarning("SetFocalLength failed, invalid focal length %1", {fl});
        return false;
    }
    if (diam < 1)
    {
        logWarning("SetFocalLength failed, invalid diameter %1", {diam});
        return false;
    }
    if (!sendModNewNumber(device, "SCOPE_INFO", "APERTURE", diam))
    {
        logWarning("SetFocalLength failed %1 %2 %3", {"SCOPE_INFO", "APERTURE", diam});
        return false;
    }
    if (!sendModNewNumber(device, "SCOPE_INFO", "FOCAL_LENGTH", fl))
    {
        logWarning("SetFocalLength failed %1 %2 %3", {"SCOPE_INFO", "FOCAL_LENGTH", fl});
        return false;
    }
    return true;

}

bool IndiModule::giveMeAnEquipment()
{

    if (!getStore().contains("devices"))
    {
        OST::PropertyMulti* pm = new OST::PropertyMulti("devices", "Devices", OST::ReadWrite, "Devices", "", "222Parms333",
                true,
                false);
        createProperty(pm);
    }
    if (!getProperty("devices")->getElts()->contains("equipments"))
    {
        OST::ElementString* e = new  OST::ElementString("equipments", "Global equipment set", "01", "");
        e->setGlobalLov("equipments", OST::LovScope::Controller);
        e->setDirectEdit(true);
        e->setAutoUpdate(true);
        getProperty("devices")->addElt(e);

    }
    return true;
}
bool IndiModule::giveMeAnActions()
{
    if (!getStore().contains("actions"))
    {
        OST::PropertyMulti* pm = new OST::PropertyMulti("actions", "Actions", OST::ReadWrite, "Actions", "", "0000",
                false,
                false);
        createProperty(pm);
    }

}
bool IndiModule::giveMeATarget()
{
    if (!getStore().contains("target"))
    {
        OST::PropertyMulti* pm = new OST::PropertyMulti("target", "Target", OST::ReadWrite, "Target", "", "0500",
                true,
                false);
        createProperty(pm);
    }

};
