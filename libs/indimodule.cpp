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

                    setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                    if (connectIndi()) setOstPropertyAttribute(keyprop, "status", IPS_OK, true);
                    else setOstPropertyAttribute(keyprop, "status", IPS_ALERT, true);
                }
                if (keyelt == "disconserv")
                {
                    setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                    if (disconnectIndi()) setOstPropertyAttribute(keyprop, "status", IPS_OK, true);
                    else setOstPropertyAttribute(keyprop, "status", IPS_ALERT, true);
                }
            }
            if (keyprop == "devicesactions")
            {
                setOstElementValue(keyprop, keyelt, false, false);
                if (!isServerConnected())
                {
                    sendWarning("Indi server not connected");
                    setOstPropertyAttribute(keyprop, "status", IPS_ALERT, true);
                    break;
                }
                if (keyelt == "condevs")
                {
                    setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                    if (connectAllDevices()) setOstPropertyAttribute(keyprop, "status", IPS_OK, true);
                    else setOstPropertyAttribute(keyprop, "status", IPS_ALERT, true);
                }
                if (keyelt == "discondevs")
                {
                    setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                    if (disconnectAllDevices()) setOstPropertyAttribute(keyprop, "status", IPS_OK, true);
                    else setOstPropertyAttribute(keyprop, "status", IPS_ALERT, true);
                }
                if (keyelt == "loadconfs")
                {
                    setOstPropertyAttribute(keyprop, "status", IPS_BUSY, true);
                    if (loadDevicesConfs()) setOstPropertyAttribute(keyprop, "status", IPS_OK, true);
                    else setOstPropertyAttribute(keyprop, "status", IPS_ALERT, true);
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
    setServer(getString("server", "host").toStdString().c_str(), getInt("server", "port"));
    if (!connectServer())
    {
        sendError("Couldn't connect to Indi server");
        return;
    }
    newUniversalMessage("Indi server connected");
    sendMessage("Indi server connected");

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
        sendError("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertyNumber prop = dp.getNumber(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;

}
bool IndiModule::getModNumber(const QString &deviceName, const QString &propertyName, const QString  &elementName,
                              double &value)
{
    //qDebug() << "getModNumber" << " " << deviceName << " " << propertyName<< " " << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        sendError("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertyNumber prop = dp.getNumber(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool IndiModule::getModSwitch(const QString &deviceName, const QString &propertyName, const QString  &elementName,
                              bool &value)
{
    //qDebug() << "getModswitch " << "-" << deviceName << "-" << propertyName<< "-" << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        sendError("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertySwitch prop = dp.getSwitch(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool IndiModule::getModText(const QString &deviceName, const QString &propertyName, const QString  &elementName,
                            QString &value)
{
    //qDebug() << "getModText" << "-" << deviceName << "-" << propertyName<< "-" << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        sendError("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertyText prop = dp.getText(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool IndiModule::sendModNewText  (QString deviceName, QString propertyName, QString elementName, QString text)
{
    //qDebug() << "sendModNewText" << "-" << deviceName << "-" << propertyName<< "-" << elementName;
    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        sendError("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertyText prop = dp.getText(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool IndiModule::sendModNewSwitch(QString deviceName, QString propertyName, QString elementName, ISState sw)
{
    //qDebug() << "sendModNewswitch " << "-" << deviceName << "-" << propertyName<< "-" << elementName;

    INDI::BaseDevice dp = getDevice(deviceName.toStdString().c_str());

    if (!dp.isValid())
    {
        sendError("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INDI::PropertySwitch prop = dp.getSwitch(propertyName.toStdString().c_str());
    if (!prop.isValid())
    {
        sendError("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
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
    sendError("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;

}
bool IndiModule::frameSet(QString devicename, double x, double y, double width, double height)
{
    //qDebug() << "frameSet" << "-" << devicename;
    INDI::BaseDevice dp = getDevice(devicename.toStdString().c_str());
    if (!dp.isValid())
    {
        sendError("Error - unable to find " + devicename + " device. Aborting.");
        return false;
    }

    INDI::PropertyNumber prop = dp.getNumber("CCD_FRAME");
    if (!prop.isValid())
    {
        sendError("Error - unable to find " + devicename + "/" + "CCD_FRAME" + " property. Aborting.");
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
        sendError("Error - unable to find " + devicename + " device. Aborting.");
        return false;
    }

    INDI::PropertySwitch prop = dp.getSwitch("CCD_FRAME_RESET");
    if (!prop.isValid())
    {
        sendError("Error - unable to find " + devicename + "/" + "CCD_FRAME_RESET" + " property. Aborting.");
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
