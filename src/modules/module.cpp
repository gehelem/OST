#include <QtCore>
#include <basedevice.h>
#include "module.h"

Module::Module()
{
}
Module::~Module()
{
}
QMap<QString,QString> Module::getDevices(void)
{
    return _devices;
}
void Module::setDevices(QMap<QString,QString> devices)
{
    _devices =devices;
}
void Module::setAction(QString action)
{
    _actions[action]=true;
}
void Module::sendMessage(QString message)
{
    QString mess = QDateTime::currentDateTime().toString("[yyyyMMdd hh:mm:ss.zzz]") + " - " + _modulename + " - " + message;
    qDebug() << mess.toStdString().c_str();
    emit newMessage(mess);
}
/*!
 * Connects to indi server
 * Should we add host/port ??
 * IMHO this would be overkill as headless instance of OST and indiserver should be 99% hosted on the same machine
 * It would be easy to manage 1% remaining with indiserver chaining capabilities
 */
bool Module::connectIndi()
{

    if (indiclient->isServerConnected())
    {
        sendMessage("Indi server already connected");
        return true;
    } else {
        indiclient->setServer("localhost", 7624);
        if (indiclient->connectServer()){
            sendMessage("Indi server connected");
            return true;
        } else {
            sendMessage("Couldn't connect to Indi server");
            return false;
        }
    }
}
bool Module::disconnectIndi(void)
{
    if (indiclient->isServerConnected())
    {
        if (indiclient->disconnectServer()){
            sendMessage("Indi server disconnected");
            return true;
        } else {
            sendMessage("Couldn't disconnect from Indi server");
            return false;
        }
    } else {
        sendMessage("Indi server already disconnected");
        return true;
    }
}

/*!
 * Asks every device to connect
 */
void Module::connectAllDevices()
{
    std::vector<INDI::BaseDevice *> devs = indiclient->getDevices();
    for(std::size_t i = 0; i < devs.size(); i++) {
        ISwitchVectorProperty *svp = devs[i]->getSwitch("CONNECTION");

        if (svp==nullptr) {
            sendMessage("Couldn't find CONNECTION switch");
        } else {
            for (int j=0;j<svp->nsp;j++) {
                if (strcmp(svp->sp[j].name,"CONNECT")==0) {
                    svp->sp[j].s=ISS_ON;
                } else {
                    svp->sp[j].s=ISS_OFF;
                }
            }

           indiclient->sendNewSwitch(svp);
            if (devs[i]->getDriverInterface() & INDI::BaseDevice::CCD_INTERFACE)
            {
                sendMessage("Setting blob mode " + QString(devs[i]->getDeviceName()));
                indiclient->setBLOBMode(B_ALSO,devs[i]->getDeviceName(),nullptr);
            }

        }

    }


}

/*!
 * Asks every device to disconnect
 */
void Module::disconnectAllDevices()
{
    std::vector<INDI::BaseDevice *> devs = indiclient->getDevices();

    for(std::size_t i = 0; i < devs.size(); i++) {
        ISwitchVectorProperty *svp = devs[i]->getSwitch("CONNECTION");

        if (svp==nullptr) {
            sendMessage("Couldn't find CONNECTION switch");
        } else {
            for (int j=0;j<svp->nsp;j++) {
                if (strcmp(svp->sp[j].name,"DISCONNECT")==0) {
                    svp->sp[j].s=ISS_ON;
                } else {
                    svp->sp[j].s=ISS_OFF;
                }
            }
            indiclient->sendNewSwitch(svp);

        }

    }


}

/*!
 * Asks every device to load saved configuration
 */
void Module::loadDevicesConfs()
{
    std::vector<INDI::BaseDevice *> devs = indiclient->getDevices();
    for(std::size_t i = 0; i < devs.size(); i++) {
        sendMessage("Loading device conf " +QString(devs[i]->getDeviceName()));
        if (devs[i]->isConnected()) {
            ISwitchVectorProperty *svp = devs[i]->getSwitch("CONFIG_PROCESS");

            if (svp==nullptr) {
                sendMessage("Couldn't find CONFIG_PROCESS switch");
            } else {
                for (int j=0;j<svp->nsp;j++) {
                    if (strcmp(svp->sp[j].name,"CONFIG_LOAD")==0) {
                        svp->sp[j].s=ISS_ON;
                    } else {
                        svp->sp[j].s=ISS_OFF;
                    }
                }
                indiclient->sendNewSwitch(svp);
            }

        }
    }
}


auto Module::sendNewNumber(const QString& deviceName, const QString& propertyName,const QString&  elementName, const double& value) -> bool
{
    //qDebug() << "taskSendNewNumber" << " " << deviceName << " " << propertyName<< " " << elementName;
    INDI::BaseDevice *dp = indiclient->getDevice(deviceName.toStdString().c_str());

    if (dp== nullptr)
    {
        sendMessage("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    INumberVectorProperty *prop = nullptr;
    prop = dp->getNumber(propertyName.toStdString().c_str());
    if (prop == nullptr)
    {
        sendMessage("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
        return false;
    }

    for (int i=0;i<prop->nnp;i++) {
        if (strcmp(prop->np[i].name, elementName.toStdString().c_str()) == 0) {
            prop->np[i].value=value;
            indiclient->sendNewNumber(prop);
            return true;
        }
    }
    sendMessage("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;

}
bool Module::sendNewText  (QString deviceName,QString propertyName,QString elementName, QString text)
{
    //qDebug() << "taskSendNewText";
    INDI::BaseDevice *dp = indiclient->getDevice(deviceName.toStdString().c_str());

    if (dp== nullptr)
    {
        sendMessage("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    ITextVectorProperty *prop = nullptr;
    prop= dp->getText(propertyName.toStdString().c_str());
    if (prop == nullptr)
    {
        sendMessage("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
        return false;
    }

    for (int i=0;i<prop->ntp;i++) {
        if (strcmp(prop->tp[i].name, elementName.toStdString().c_str()) == 0) {
            indiclient->sendNewText(deviceName.toStdString().c_str(),propertyName.toStdString().c_str(),elementName.toStdString().c_str(),text.toStdString().c_str());
            return true;
        }
    }
    sendMessage("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool Module::sendNewSwitch(QString deviceName,QString propertyName,QString elementName, ISState sw)
{
    //qDebug() << "taskSendNewSwitch";

    INDI::BaseDevice *dp;
    dp = indiclient->getDevice(deviceName.toStdString().c_str());

    if (dp== nullptr)
    {
        sendMessage("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    ISwitchVectorProperty *prop = nullptr;
    prop= dp->getSwitch(propertyName.toStdString().c_str());
    if (prop == nullptr)
    {
        sendMessage("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
        return false;
    }

    for (int i=0;i<prop->nsp;i++) {
        if (strcmp(prop->sp[i].name, elementName.toStdString().c_str()) == 0) {
            prop->sp[i].s=sw;
            indiclient->sendNewSwitch(prop);
            return true;
        }
    }
    sendMessage("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;

}
bool Module::frameSet(QString devicename,double x,double y,double width,double height)
{
    INDI::BaseDevice *dp;
    dp = indiclient->getDevice(devicename.toStdString().c_str());
    if (dp== nullptr)
    {
        sendMessage("Error - unable to find " + devicename + " device. Aborting.");
        return false;
    }

    INumberVectorProperty *prop = nullptr;
    prop = dp->getNumber("CCD_FRAME");
    if (prop == nullptr)
    {
        sendMessage("Error - unable to find " + devicename + "/" + "CCD_FRAME" + " property. Aborting.");
        return false;
    }

    for (int i=0;i<prop->nnp;i++) {
        if (strcmp(prop->np[i].name, "X") == 0) {
            prop->np[i].value=x;
        }
        if (strcmp(prop->np[i].name, "Y") == 0) {
            prop->np[i].value=y;
        }
        if (strcmp(prop->np[i].name, "WIDTH") == 0) {
            prop->np[i].value=width;
        }
        if (strcmp(prop->np[i].name, "HEIGHT") == 0) {
            prop->np[i].value=height;
        }
    }

    indiclient->sendNewNumber(prop);
    return true;
}
bool Module::frameReset(QString devicename)
{
    INDI::BaseDevice *dp;
    dp = indiclient->getDevice(devicename.toStdString().c_str());
    if (dp== nullptr)
    {
        sendMessage("Error - unable to find " + devicename + " device. Aborting.");
        return false;
    }

    ISwitchVectorProperty *prop = nullptr;
    prop = dp->getSwitch("CCD_FRAME_RESET");
    if (prop == nullptr)
    {
        sendMessage("Error - unable to find " + devicename + "/" + "CCD_FRAME_RESET" + " property. Aborting.");
        return true;
    }

    for (int i=0;i<prop->nsp;i++) {
        if (strcmp(prop->sp[i].name, "RESET") == 0) {
            prop->sp[i].s=ISS_ON;
        }
    }

    indiclient->sendNewSwitch(prop);
    emit askedFrameReset(devicename);
    return true;
}



/*
void Module::IndiServerConnected()
{
    //qDebug() << "gotserverConnected";

}
void Module::gotserverDisconnected(int exit_code)
{
    //qDebug() << "gotserverDisconnected";
    Q_UNUSED(exit_code);
}
void Module::gotnewDevice(INDI::BaseDevice *dp)
{
    //qDebug() << "gotnewDevice : " << dp->getDeviceName();
    Q_UNUSED(dp);
}
void Module::gotremoveDevice(INDI::BaseDevice *dp)
{
    //qDebug() << "gotremoveDevice";
    Q_UNUSED(dp);
}
void Module::gotnewProperty(INDI::Property *property)
{
    //qDebug() << "gotnewProperty" << tasks.size();
    Q_UNUSED(property);

}
void Module::gotremoveProperty(INDI::Property *property)
{
    //qDebug() << "gotremoveProperty";
    Q_UNUSED(property);
}


void Module::gotnewText(ITextVectorProperty *tvp)
{
    //qDebug() << "gotnewText";
    Q_UNUSED(tvp);
}

void Module::gotnewSwitch(ISwitchVectorProperty *svp)
{
   // qDebug() << "gotnewSwitch";
    Q_UNUSED(svp);
}

void Module::gotnewNumber(INumberVectorProperty *nvp)
{
    //qDebug() << "gotnewNumber";
    Q_UNUSED(nvp);
}
void Module::gotnewLight(ILightVectorProperty *lvp)
{
    //qDebug() << "gotnewLight";
    Q_UNUSED(lvp);

}
void Module::gotnewBLOB(IBLOB *bp)
{
    //qDebug() << "gotnewBLOB";
    Q_UNUSED(bp);
}
void Module::gotnewMessage(INDI::BaseDevice *dp, int messageID)
{
    //qDebug() << "gotnewMessage";
    Q_UNUSED(dp);
    Q_UNUSED(messageID);
}


void Module::IndiNewNumber(INumberVectorProperty *nvp)
{
    //qDebug() << "gotnewNumber";
    if (QString(nvp->name)=="CCD Simulator")
    sendMessage("New number " + QString(nvp->device) + " - " + QString(nvp->name) + " - " + QString::number(nvp->np[0].value));
    indiclient->setBLOBMode(B_ALSO,"CCD Simulator",nullptr);
}
void Module::IndiNewBLOB(IBLOB *bp)
{
    //qDebug() << "gotnewNumber";
    sendMessage("New blob " + QString(bp->bvp->device) + " - " + QString(bp->name) + " - " + QString::number(bp->bloblen));
}*/
