#include <QtCore>
#include <basedevice.h>
#include "indimodule.h"
#include "version.cc"

IndiModule::IndiModule(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : Basemodule(name, label, profile, availableModuleLibs)
{
    setVerbose(false);
    //_moduletype = "IndiModule";
    loadOstPropertiesFromFile(":indimodule.json");
    getEltString("indiGit", "hash")->setValue(QString::fromStdString(Version::GIT_SHA1), false);
    getEltString("indiGit", "date")->setValue(QString::fromStdString(Version::GIT_DATE), false);
    getEltString("indiGit", "message")->setValue(QString::fromStdString(Version::GIT_COMMIT_SUBJECT), false);

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

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &IndiModule::connectIndiTimer);
    timer->start(2000);

}
void IndiModule::OnDispatchToIndiExternalEvent(const QString &eventType, const QString  &eventModule,
        const QString  &eventKey, const QVariantMap &eventData)

{
    Q_UNUSED(eventType);
    Q_UNUSED(eventKey);

    if (!(getModuleName() == eventModule))
    {
        return;
    }
    //sendMessage("OnIndiExternalEvent - recv : " + getModuleName()+ "-" + eventType + "-" + eventKey);
    foreach(const QString &keyprop, eventData.keys())
    {
        foreach(const QString &keyelt, eventData[keyprop].toMap()["elements"].toMap().keys())
        {
            //setOstElementValue(keyprop, keyelt, eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"], true);
            if (keyprop == "serveractions")
            {
                getEltBool(keyprop, keyelt)->setValue(false, false);
                if (keyelt == "conserv")
                {
                    getProperty(keyprop)->setState(OST::Busy);
                    if (connectIndi()) getProperty(keyprop)->setState(OST::Ok);
                    else getProperty(keyprop)->setState(OST::Error);
                }
                if (keyelt == "disconserv")
                {
                    getProperty(keyprop)->setState(OST::Busy);
                    if (disconnectIndi()) getProperty(keyprop)->setState(OST::Ok);
                    else getProperty(keyprop)->setState(OST::Error);
                }
            }
            if (keyprop == "devicesactions")
            {
                getEltBool(keyprop, keyelt)->setValue(false, false);
                if (!isServerConnected())
                {
                    sendWarning("Indi server not connected");
                    getProperty(keyprop)->setState(OST::Error);
                    break;
                }
                if (keyelt == "condevs")
                {
                    getProperty(keyprop)->setState(OST::Busy);
                    if (connectAllDevices()) getProperty(keyprop)->setState(OST::Ok);
                    else getProperty(keyprop)->setState(OST::Error);
                }
                if (keyelt == "discondevs")
                {
                    getProperty(keyprop)->setState(OST::Busy);
                    if (disconnectAllDevices()) getProperty(keyprop)->setState(OST::Ok);
                    else getProperty(keyprop)->setState(OST::Error);
                }
                if (keyelt == "loadconfs")
                {
                    getProperty(keyprop)->setState(OST::Busy);
                    if (loadDevicesConfs()) getProperty(keyprop)->setState(OST::Ok);
                    else getProperty(keyprop)->setState(OST::Error);
                }
            }
        }
    }
}
void IndiModule::connectIndiTimer()
{
    if (isServerConnected())
    {
        return;
    }
    connectIndi();

}
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
        sendMessage("Indi server connected");
        QTimer::singleShot(500, this, &IndiModule::OnAfterIndiConnectIndiTimer);
        return true;
    }
    sendError("Couldn't connect to Indi server");
    return false;
}
bool IndiModule::disconnectIndi(void)
{
    if (!isServerConnected())
    {
        sendWarning("Indi server already disconnected");
        return true;
    }
    if (!disconnectServer())
    {
        sendError("Couldn't disconnect from Indi server");
        return false;
    }
    sendMessage("Indi server disconnected");
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
            sendError("Couldn't find CONNECTION switch " + QString(devs[i].getDeviceName()));
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
                sendMessage("Set blob mode " + QString(devs[i].getDeviceName()));
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
            sendError("Couldn't find CONNECTION switch " + QString(devs[i].getDeviceName()));
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
        sendWarning("Indi server is not connected");
        return false;
    }
    bool _checkdevice = false;
    foreach (INDI::BaseDevice dd, getDevices())
    {
        if (strcmp(dd.getDeviceName(), deviceName.toStdString().c_str()) == 0) _checkdevice = true;
    }
    if (!_checkdevice)
    {
        sendError("Device " + deviceName + " not found");
        return false;
    }

    INDI::BaseDevice dev = getDevice(deviceName.toStdString().c_str());

    INDI::PropertySwitch svp = dev.getSwitch("CONNECTION");

    if (!svp.isValid())
    {
        sendError("Couldn't find CONNECTION switch " + deviceName);
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
        sendError("Couldn't find CONNECTION switch " + deviceName);
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
        sendMessage("Loading device conf " + QString(devs[i].getDeviceName()));
        if (devs[i].isConnected())
        {
            INDI::PropertySwitch svp = devs[i].getSwitch("CONFIG_PROCESS");

            if (!svp.isValid())
            {
                sendError("Couldn't find CONFIG_PROCESS switch " + QString(devs[i].getDeviceName()));
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
        sendError("Unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertyNumber prop = dp.getNumber(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
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
        sendError("Capture - device " + deviceName + " not found. Aborting.");
        return false;
    }
    if (dp == false)
    {
        sendWarning("Capture - " + deviceName + " not connected, trying to connect");
        if (!connectDevice(deviceName)) return false;
    }
    setBLOBMode(B_ALSO, deviceName.toStdString().c_str(), nullptr);
    if(dp.getProperty("CCD_CONTROLS"))
    {
        INDI::PropertyNumber prop = dp.getNumber("CCD_CONTROLS");
        if (wgain < prop.findWidgetByName("Gain")->min)
        {
            sendWarning("Capture - " + deviceName + " gain requested too low (" + QString::number(wgain) + "), setting to " +
                        QString::number(prop.findWidgetByName("Gain")->min));
            wgain = prop.findWidgetByName("Gain")->min;
        }
        if (wgain > prop.findWidgetByName("Gain")->max)
        {
            sendWarning("Capture - " + deviceName + " gain requested too high (" + QString::number(wgain) + "), setting to " +
                        QString::number(prop.findWidgetByName("Gain")->max));
            wgain = prop.findWidgetByName("Gain")->max;
        }
        if (woffset < prop.findWidgetByName("Offset")->min)
        {
            sendWarning("Capture - " + deviceName + " gain requested too low (" + QString::number(woffset) + "), setting to " +
                        QString::number(prop.findWidgetByName("Offset")->min));
            woffset = prop.findWidgetByName("Offset")->min;
        }
        if (woffset > prop.findWidgetByName("Offset")->max)
        {
            sendWarning("Capture - " + deviceName + " gain requested too high (" + QString::number(woffset) + "), setting to " +
                        QString::number(prop.findWidgetByName("Offset")->max));
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
            sendWarning("Capture - " + deviceName + " gain requested too low (" + QString::number(wgain) + "), setting to " +
                        QString::number(prop.findWidgetByName("GAIN")->min));
            wgain = prop.findWidgetByName("GAIN")->min;
        }
        if (wgain > prop.findWidgetByName("GAIN")->max)
        {
            sendWarning("Capture - " + deviceName + " gain requested too high (" + QString::number(wgain) + "), setting to " +
                        QString::number(prop.findWidgetByName("GAIN")->max));
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
            sendWarning("Capture - " + deviceName + " offset requested too low (" + QString::number(woffset) + "), setting to " +
                        QString::number(prop.findWidgetByName("OFFSET")->min));
            woffset = prop.findWidgetByName("OFFSET")->min;
        }
        if (woffset > prop.findWidgetByName("OFFSET")->max)
        {
            sendWarning("Capture - " + deviceName + " offset requested too high (" + QString::number(woffset) + "), setting to " +
                        QString::number(prop.findWidgetByName("OFFSET")->max));
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
        sendError("Unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertyNumber prop = dp.getNumber(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool IndiModule::getModSwitch(const QString &deviceName, const QString &propertyName, const QString  &elementName,
                              bool &value)
{
    //qDebug() << "getModswitch " << "-" << deviceName << "-" << propertyName<< "-" << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        sendError("Unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertySwitch prop = dp.getSwitch(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool IndiModule::getModText(const QString &deviceName, const QString &propertyName, const QString  &elementName,
                            QString &value)
{
    //qDebug() << "getModText" << "-" << deviceName << "-" << propertyName<< "-" << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        sendError("Unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertyText prop = dp.getText(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool IndiModule::sendModNewText  (QString deviceName, QString propertyName, QString elementName, QString text)
{
    //qDebug() << "sendModNewText" << "-" << deviceName << "-" << propertyName<< "-" << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        sendError("Unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertyText prop = dp.getText(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool IndiModule::sendModNewSwitch(QString deviceName, QString propertyName, QString elementName, ISState sw)
{
    //qDebug() << "sendModNewswitch " << "-" << deviceName << "-" << propertyName<< "-" << elementName;

    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        sendError("Unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertySwitch prop = dp.getSwitch(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;

}
bool IndiModule::frameSet(QString devicename, double x, double y, double width, double height)
{
    //qDebug() << "frameSet" << "-" << devicename;
    INDI::BaseDevice dp = getDevice(devicename.toStdString().c_str());
    if (!dp.isValid())
    {
        sendError("Unable to find " + devicename + " device. Aborting.");
        return false;
    }

    INDI::PropertyNumber prop = dp.getNumber("CCD_FRAME");
    if (!prop.isValid())
    {
        sendError("Unable to find " + devicename + "/" + "CCD_FRAME" + " property. Aborting.");
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
        sendError("Unable to find " + devicename + " device. Aborting.");
        return false;
    }

    INDI::PropertySwitch prop = dp.getSwitch("CCD_FRAME_RESET");
    if (!prop.isValid())
    {
        sendError("Unable to find " + devicename + "/" + "CCD_FRAME_RESET" + " property. Aborting.");
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
    OST::ElementString* s = new  OST::ElementString("name", "", "");
    s->setValue("--", false);
    s->setAutoUpdate(true);
    pm->addElt("name", s);

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
    createProperty(key, pm);
    return true;
}
void IndiModule::OnAfterIndiConnectIndiTimer()
{
    std::vector<INDI::BaseDevice> devs = getDevices();
    for(std::size_t i = 0; i < devs.size(); i++)
    {
        refreshDeviceslovs(devs[i].getDeviceName());
    }

}
bool IndiModule::refreshDeviceslovs(QString deviceName)
{
    if (getDevice(deviceName.toStdString().c_str()).getDriverInterface() &
            INDI::BaseDevice::DRIVER_INTERFACE::GENERAL_INTERFACE)
    {
        QString d = getDevice(deviceName.toStdString().c_str()).getDeviceName();
        getGlovString("DRIVER_INTERFACE-GENERAL_INTERFACE")->lovAdd(d, d);
    }
    if (getDevice(deviceName.toStdString().c_str()).getDriverInterface() &
            INDI::BaseDevice::DRIVER_INTERFACE::CCD_INTERFACE)
    {
        QString d = getDevice(deviceName.toStdString().c_str()).getDeviceName();
        getGlovString("DRIVER_INTERFACE-CCD_INTERFACE")->lovAdd(d, d);
    }
    if (getDevice(deviceName.toStdString().c_str()).getDriverInterface() &
            INDI::BaseDevice::DRIVER_INTERFACE::TELESCOPE_INTERFACE)
    {
        QString d = getDevice(deviceName.toStdString().c_str()).getDeviceName();
        getGlovString("DRIVER_INTERFACE-TELESCOPE_INTERFACE")->lovAdd(d, d);
    }
    if (getDevice(deviceName.toStdString().c_str()).getDriverInterface() &
            INDI::BaseDevice::DRIVER_INTERFACE::GUIDER_INTERFACE)
    {
        QString d = getDevice(deviceName.toStdString().c_str()).getDeviceName();
        getGlovString("DRIVER_INTERFACE-GUIDER_INTERFACE")->lovAdd(d, d);
    }
    if (getDevice(deviceName.toStdString().c_str()).getDriverInterface() &
            INDI::BaseDevice::DRIVER_INTERFACE::FOCUSER_INTERFACE)
    {
        QString d = getDevice(deviceName.toStdString().c_str()).getDeviceName();
        getGlovString("DRIVER_INTERFACE-FOCUSER_INTERFACE")->lovAdd(d, d);
    }
    if (getDevice(deviceName.toStdString().c_str()).getDriverInterface() &
            INDI::BaseDevice::DRIVER_INTERFACE::FILTER_INTERFACE)
    {
        QString d = getDevice(deviceName.toStdString().c_str()).getDeviceName();
        getGlovString("DRIVER_INTERFACE-FILTER_INTERFACE")->lovAdd(d, d);
    }
    if (getDevice(deviceName.toStdString().c_str()).getDriverInterface() &
            INDI::BaseDevice::DRIVER_INTERFACE::GPS_INTERFACE)
    {
        QString d = getDevice(deviceName.toStdString().c_str()).getDeviceName();
        getGlovString("DRIVER_INTERFACE-GPS_INTERFACE")->lovAdd(d, d);
    }
    return true;
}
bool IndiModule::defineMeAsFocuser()
{
    defineMeAsImager();

    OST::PropertyMulti* pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ElementBool* b = new  OST::ElementBool("Abort focus", "foc99", "");
    b->setValue(false, false);
    pm->addElt("abortfocus", b);
    b = new  OST::ElementBool("Autofocus", "foc50", "");
    b->setValue(false, false);
    pm->addElt("autofocus", b);
    b = new  OST::ElementBool("Loop", "foc70", "");
    b->setValue(false, false);
    pm->addElt("loop", b);
    mIsFocuser = true;
    return true;

}
bool IndiModule::defineMeAsGuider()
{
    defineMeAsImager();

    OST::PropertyMulti* pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ElementBool* b = new  OST::ElementBool("Abort guider", "guid99", "");
    b->setValue(false, false);
    pm->addElt("abortguider", b);
    b = new  OST::ElementBool("Guide", "guid50", "");
    b->setValue(false, false);
    pm->addElt("guide", b);
    b = new  OST::ElementBool("Calibrate", "guid00", "");
    b->setValue(false, false);
    pm->addElt("calibrate", b);
    mIsGuider = true;
    return true;

}
bool IndiModule::defineMeAsSequencer()
{
    defineMeAsImager();

    OST::PropertyMulti* pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ElementBool* b = new  OST::ElementBool("Abort sequence", "seq99", "");
    b->setValue(false, false);
    pm->addElt("abortsequence", b);
    b = new  OST::ElementBool("Start sequence", "seq00", "");
    b->setValue(false, false);
    pm->addElt("startsequence", b);
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
        createProperty("image", dynprop);
    }

    if (getStore().contains("image"))
    {
        if (!(getStore()["image"]->getElts()->contains("image")))
        {
            OST::PropertyMulti * pm = getProperty("image");
            OST::ElementImg* img = new OST::ElementImg("", "00", "");
            pm->addElt("image", img);
        }

    }

    OST::PropertyMulti * pm = getProperty("parms");
    pm->setRule(OST::SwitchsRule::Any);

    if (!getStore()["parms"]->getElts()->contains("exposure"))
    {
        OST::ElementFloat* f = new  OST::ElementFloat("Exposure", "exp000", "");
        f->setValue(0, false);
        f->setDirectEdit(true);
        f->setAutoUpdate(true);
        f->setMinMax(0.00001, 300);
        f->setStep(0.001);
        f->setSlider(OST::SliderAndValue);
        pm->addElt("exposure", f);
    }



    if (!getStore()["parms"]->getElts()->contains("gain"))
    {
        OST::ElementInt* i  = new  OST::ElementInt("Gain", "exp020", "");
        i->setValue(0, false);
        i->setDirectEdit(true);
        i->setAutoUpdate(true);
        i->setMinMax(0, 500);
        i->setStep(1);
        i->setSlider(OST::SliderAndValue);
        pm->addElt("gain", i);
    }

    if (!getStore()["parms"]->getElts()->contains("offset"))
    {
        OST::ElementInt* i  = new  OST::ElementInt("Offset", "exp030", "");
        i->setValue(0, false);
        i->setDirectEdit(true);
        i->setAutoUpdate(true);
        i->setMinMax(0, 500);
        i->setStep(1);
        i->setSlider(OST::SliderAndValue);
        pm->addElt("offset", i);
    }

    mIsImager = true;
    return true;

}
bool IndiModule::defineMeAsNavigator()
{
    defineMeAsImager();


    OST::PropertyMulti* pm  = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ElementBool* b = new  OST::ElementBool("Abort navigator", "nav99", "");
    b->setValue(false, false);
    pm->addElt("abortnavigator", b);
    b = new  OST::ElementBool("Center target", "nav00", "");
    b->setValue(false, false);
    pm->addElt("gototarget", b);
    OST::ElementString* s = new  OST::ElementString("Target name", "nav50", "");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addElt("targetname", s);
    OST::ElementFloat* f = new  OST::ElementFloat("Target RA", "nav51", "");
    f->setDirectEdit(true);
    f->setAutoUpdate(true);
    pm->addElt("targetra", f);
    f = new  OST::ElementFloat("Target DEC", "nav52", "");
    f->setDirectEdit(true);
    f->setAutoUpdate(true);
    pm->addElt("targetde", f);
    mIsNavigator = true;
    return true;

}
double IndiModule::getPixelSize(const QString &deviceName)
{
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());
    if (!dp.isValid())
    {
        sendError("getPixelSize - device " + deviceName + " not found. Aborting.");
        return 0;
    }
    if (!dp.isConnected())
    {
        sendWarning("getPixelSize - " + deviceName + " not connected, trying to connect");
        if (!connectDevice(deviceName)) return 0;
    }
    INDI::PropertyNumber prop = dp.getNumber("CCD_INFO");
    return prop.findWidgetByName("CCD_PIXEL_SIZE")->getValue();

}
double IndiModule::getSampling()
{
    if (!isImager())
    {
        sendWarning("getSampling - module is not defined as an imager - defaults to 1");
        return 1;
    }
    return 206 * getPixelSize(getString("devices", "camera")) / (getFloat("optic", "fl") * getFloat("optic", "red"));
}

bool IndiModule::giveMeADevice(QString name, QString label, INDI::BaseDevice::DRIVER_INTERFACE interface)
{
    OST::PropertyMulti* pm = getProperty("devices");
    OST::ElementString* s = new  OST::ElementString(label, label, "");
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
        default:
            sendWarning("giveMeADevice unhandled interface type");
            break;
    }

    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addElt(name, s);
    return true;

}
bool IndiModule::giveMeAnOptic()
{
    if (getStore().contains("optic")) return false;

    OST::PropertyMulti* pm = new OST::PropertyMulti("optic", "Optic", OST::ReadWrite, "Parameters", "", "222Parms333", true,
            false);
    createProperty("optic", pm);
    OST::ElementFloat* f = new  OST::ElementFloat("Focal length", "1", "");
    f->setDirectEdit(true);
    f->setMinMax(1, 4000);
    f->setAutoUpdate(true);
    pm->addElt("fl", f);
    OST::ElementFloat* d = new  OST::ElementFloat("Diameter", "2", "");
    d->setDirectEdit(true);
    d->setMinMax(1, 4000);
    d->setAutoUpdate(true);
    pm->addElt("diam", d);
    OST::ElementFloat* r = new  OST::ElementFloat("Reducer", "3", "");
    r->setDirectEdit(true);
    r->setMinMax(0.1, 10);
    r->setAutoUpdate(true);
    r->setValue(1);
    pm->addElt("red", r);
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
    if ((fl < 1) || (diam < 1)) return false;
    if (!sendModNewNumber(device, "SCOPE_INFO", "APERTURE", diam)) return false;
    if (!sendModNewNumber(device, "SCOPE_INFO", "FOCAL_LENGTH", fl)) return false;
    return true;

}

