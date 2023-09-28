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
    setOstElementValue("indiGit", "hash", QString::fromStdString(Version::GIT_SHA1), false);
    setOstElementValue("indiGit", "date", QString::fromStdString(Version::GIT_DATE), false);
    setOstElementValue("indiGit", "message", QString::fromStdString(Version::GIT_COMMIT_SUBJECT), false);

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
                setOstElementValue(keyprop, keyelt, false, false);
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
                setOstElementValue(keyprop, keyelt, false, false);
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
        sendWarning("Indi server already connected");
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
                sendWarning("Can't set blob mode " + QString(devs[i].getDeviceName()));
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
    OST::ValueString* s = new  OST::ValueString("name", "", "");
    s->setValue("--", false);
    s->setAutoUpdate(true);
    pm->addValue("name", s);

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
    OST::PropertyMulti* pm = getProperty("devices");
    OST::ValueString* s = new  OST::ValueString("Focus camera", "", "");
    s->setGlobalLov("DRIVER_INTERFACE-CCD_INTERFACE");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addValue("focuscamera", s);
    s = new  OST::ValueString("Focuser", "", "");
    s->setGlobalLov("DRIVER_INTERFACE-FOCUSER_INTERFACE");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addValue("focusfocuser", s);
    s = new  OST::ValueString("Focus filter", "", "");
    s->setGlobalLov("DRIVER_INTERFACE-FILTER_INTERFACE");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addValue("focusfilter", s);

    pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ValueBool* b = new  OST::ValueBool("Abort focus", "", "");
    b->setValue(false, false);
    pm->addValue("abortfocus", b);
    b = new  OST::ValueBool("Autofocus", "", "");
    b->setValue(false, false);
    pm->addValue("autofocus", b);
    b = new  OST::ValueBool("Loop", "", "");
    b->setValue(false, false);
    pm->addValue("loop", b);
    mIsFocuser = true;
    return true;

}
bool IndiModule::defineMeAsGuider()
{
    defineMeAsImager();
    OST::PropertyMulti* pm = getProperty("devices");
    OST::ValueString* s = new  OST::ValueString("Guide camera", "", "");
    s->setGlobalLov("DRIVER_INTERFACE-CCD_INTERFACE");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addValue("guidecamera", s);
    s = new  OST::ValueString("Guider", "", "");
    s->setGlobalLov("DRIVER_INTERFACE-GUIDER_INTERFACE");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addValue("guider", s);

    pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ValueBool* b = new  OST::ValueBool("Abort guider", "", "");
    b->setValue(false, false);
    pm->addValue("abortguider", b);
    b = new  OST::ValueBool("Guide", "", "");
    b->setValue(false, false);
    pm->addValue("guide", b);
    b = new  OST::ValueBool("Calibrate", "", "");
    b->setValue(false, false);
    pm->addValue("calibrate", b);
    mIsGuider = true;
    return true;

}
bool IndiModule::defineMeAsSequencer()
{
    defineMeAsImager();
    OST::PropertyMulti* pm = getProperty("devices");
    OST::ValueString* s = new  OST::ValueString("Sequencer camera", "", "");
    s->setGlobalLov("DRIVER_INTERFACE-CCD_INTERFACE");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addValue("sequencercamera", s);
    s = new  OST::ValueString("Sequencer filter", "", "");
    s->setGlobalLov("DRIVER_INTERFACE-FILTER_INTERFACE");
    s->setAutoUpdate(true);
    s->setDirectEdit(true);
    pm->addValue("sequencerfilter", s);


    pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ValueBool* b = new  OST::ValueBool("Abort sequence", "", "");
    b->setValue(false, false);
    pm->addValue("abortsequence", b);
    b = new  OST::ValueBool("Start sequence", "", "");
    b->setValue(false, false);
    pm->addValue("startsequence", b);
    mIsSequencer = true;
    return true;

}
bool IndiModule::defineMeAsImager()
{
    if (!getStore().contains("image"))
    {
        OST::PropertyMulti* dynprop = new OST::PropertyMulti("image", "Image", OST::Permission::WriteOnly, "Control",
                "", "Control010", false, false);
        createProperty("image", dynprop);
    }

    if (getStore().contains("image"))
    {
        if (!(getStore()["image"]->getValues().contains("image")))
        {
            OST::PropertyMulti * pm = getProperty("image");
            OST::ValueImg* img = new OST::ValueImg("", "", "");
            pm->addValue("image", img);
        }

    }

    OST::PropertyMulti * pm = getProperty("parms");
    pm->setRule(OST::SwitchsRule::Any);

    if (!getStore()["parms"]->getValues().contains("exposure"))
    {
        OST::ValueFloat* f = new  OST::ValueFloat("Exposure", "200", "");
        f->setValue(0, false);
        f->setDirectEdit(true);
        f->setAutoUpdate(true);
        pm->addValue("exposure", f);
    }



    if (!getStore()["parms"]->getValues().contains("gain"))
    {
        OST::ValueInt* i  = new  OST::ValueInt("Gain", "205", "");
        i->setValue(0, false);
        i->setDirectEdit(true);
        i->setAutoUpdate(true);
        pm->addValue("gain", i);
    }

    if (!getStore()["parms"]->getValues().contains("offset"))
    {
        OST::ValueInt* i  = new  OST::ValueInt("Offset", "210", "");
        i->setValue(0, false);
        i->setDirectEdit(true);
        i->setAutoUpdate(true);
        pm->addValue("offset", i);
    }

    mIsImager = true;
    return true;

}
bool IndiModule::defineMeAsNavigator()
{
    defineMeAsImager();
    OST::PropertyMulti* pm = getProperty("devices");
    OST::ValueString* s = new  OST::ValueString("Navigator camera", "", "");
    s->setGlobalLov("DRIVER_INTERFACE-CCD_INTERFACE");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addValue("navigatorcamera", s);
    s = new  OST::ValueString("Navigator mount", "", "");
    s->setGlobalLov("DRIVER_INTERFACE-TELESCOPE_INTERFACE");
    s->setAutoUpdate(true);
    s->setDirectEdit(true);
    pm->addValue("navigatormount", s);


    pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::AtMostOne);
    OST::ValueBool* b = new  OST::ValueBool("Abort navigator", "", "");
    b->setValue(false, false);
    pm->addValue("abortnavigator", b);
    b = new  OST::ValueBool("Center target", "", "");
    b->setValue(false, false);
    pm->addValue("gototarget", b);
    s = new  OST::ValueString("Target name", "50", "");
    s->setDirectEdit(true);
    s->setAutoUpdate(true);
    pm->addValue("targetname", s);
    OST::ValueFloat* f = new  OST::ValueFloat("Target RA", "51", "");
    f->setDirectEdit(true);
    f->setAutoUpdate(true);
    pm->addValue("targetra", f);
    f = new  OST::ValueFloat("Target DEC", "52", "");
    f->setDirectEdit(true);
    f->setAutoUpdate(true);
    pm->addValue("targetde", f);

    mIsNavigator = true;
    return true;

}
