#include <QtCore>
#include <basedevice.h>
#include "basemodule.h"

Basemodule::Basemodule(QString name, QString label, QString profile)
    :_modulename(name),
      _modulelabel(label)
{
    setVerbose(false);
    _moduletype="basemodule";
    /*createOstProperty("moduleName","Module name",0,"info","");

    createOstProperty("moduleLabel","Module label",0,"info","");

    createOstProperty("moduleDescription","Module description",0,"info","");

    createOstProperty("version","Version",0,"info","");

    createOstProperty("baseVersion","BaseVersion",0,"info","");

    createOstProperty("indiConnected","Indi server connected",0,"info","");

    createOstProperty("message","Message",0,"info","");*/
    loadPropertiesFromFile(":basemodule.json");
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Basemodule::connectIndiTimer);
    timer->start(10000);



}
void Basemodule::connectIndiTimer()
{
        if (!isServerConnected()) {
            if (connectServer()){
                newUniversalMessage("Indi server connected");
                sendMessage("Indi server connected");
            } else {
                sendMessage("Couldn't connect to Indi server");
            }
        }
}
void Basemodule::setHostport(QString host, int port)
{
    setServer(host.toStdString().c_str(), port);
}

/*!
 * Connects to indi server
 * Should we add host/port ??
 * IMHO this would be overkill as headless instance of OST and indiserver should be 99% hosted on the same machine
 * It would be easy to manage 1% remaining with indiserver chaining capabilities
 */
bool Basemodule::connectIndi()
{

    if (isServerConnected())
    {
        sendMessage("Indi server already connected");
        newUniversalMessage("Indi server already connected");
        return true;
    } else {
        if (connectServer()){
            newUniversalMessage("Indi server connected");
            sendMessage("Indi server connected");
            return true;
        } else {
            sendMessage("Couldn't connect to Indi server");
            return false;
        }
    }

}
void Basemodule::sendMessage(QString message)
{
    QString mess = QDateTime::currentDateTime().toString("[yyyyMMdd hh:mm:ss.zzz]") + " - " + _modulename + " - " + message;
}
void Basemodule::OnDumpAsked()
{
    emit moduleEvent("moduledump","*");
}

bool Basemodule::disconnectIndi(void)
{
    if (isServerConnected())
    {
        if (disconnectServer()){
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
void Basemodule::setBlobMode(void)
{
    BOOST_LOG_TRIVIAL(debug) << "Looking for blob mode... ";
    std::vector<INDI::BaseDevice *> devs = getDevices();
    for(std::size_t i = 0; i < devs.size(); i++) {
        BOOST_LOG_TRIVIAL(debug) << "Looking for blob mode on device ... " << devs[i]->getDeviceName();
            if (devs[i]->getDriverInterface() & INDI::BaseDevice::CCD_INTERFACE)
            {
                BOOST_LOG_TRIVIAL(debug) << "Setting blob mode " << devs[i]->getDeviceName();
                sendMessage("Setting blob mode " + QString(devs[i]->getDeviceName()));
                setBLOBMode(B_ALSO,devs[i]->getDeviceName(),nullptr);
            }
    }


}

/*!
 * Asks every device to connect
 */
void Basemodule::connectAllDevices()
{
    std::vector<INDI::BaseDevice *> devs = getDevices();
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

           sendNewSwitch(svp);
            if (devs[i]->getDriverInterface() & INDI::BaseDevice::CCD_INTERFACE)
            {
                sendMessage("Setting blob mode " + QString(devs[i]->getDeviceName()));
                setBLOBMode(B_ALSO,devs[i]->getDeviceName(),nullptr);
            }

        }

    }


}

/*!
 * Asks every device to disconnect
 */
void Basemodule::disconnectAllDevices()
{
    std::vector<INDI::BaseDevice *> devs = getDevices();

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
            sendNewSwitch(svp);

        }

    }


}

/*!
 * Asks every device to load saved configuration
 */
void Basemodule::loadDevicesConfs()
{
    std::vector<INDI::BaseDevice *> devs = getDevices();
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
                sendNewSwitch(svp);
            }

        }
    }
}



auto Basemodule::sendModNewNumber(const QString& deviceName, const QString& propertyName,const QString&  elementName, const double& value) -> bool
{
    //qDebug() << "taskSendNewNumber" << " " << deviceName << " " << propertyName<< " " << elementName;
    INDI::BaseDevice *dp = getDevice(deviceName.toStdString().c_str());

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
            sendNewNumber(prop);
            return true;
        }
    }
    sendMessage("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;

}
bool Basemodule::getModNumber(const QString& deviceName, const QString& propertyName, const QString&  elementName, double &value)
{
    //qDebug() << "taskSendNewNumber" << " " << deviceName << " " << propertyName<< " " << elementName;
    INDI::BaseDevice *dp = getDevice(deviceName.toStdString().c_str());

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
            value = prop->np[i].value;
            return true;
        }
    }
    sendMessage("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool Basemodule::getModSwitch(const QString& deviceName, const QString& propertyName, const QString&  elementName, bool &value)
{
    //qDebug() << "taskSendNewNumber" << " " << deviceName << " " << propertyName<< " " << elementName;
    INDI::BaseDevice *dp = getDevice(deviceName.toStdString().c_str());

    if (dp== nullptr)
    {
        sendMessage("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    ISwitchVectorProperty *prop = nullptr;
    prop = dp->getSwitch(propertyName.toStdString().c_str());
    if (prop == nullptr)
    {
        sendMessage("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
        return false;
    }

    for (int i=0;i<prop->nsp;i++) {
        if (strcmp(prop->sp[i].name, elementName.toStdString().c_str()) == 0) {
            value = prop->sp[i].s;
            return true;
        }
    }
    sendMessage("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool Basemodule::getModText(const QString& deviceName, const QString& propertyName, const QString&  elementName, QString& value)
{
    //qDebug() << "taskSendNewNumber" << " " << deviceName << " " << propertyName<< " " << elementName;
    INDI::BaseDevice *dp = getDevice(deviceName.toStdString().c_str());

    if (dp== nullptr)
    {
        sendMessage("Error - unable to find " + deviceName + " device. Aborting.");
        return false;
    }
    ITextVectorProperty *prop = nullptr;
    prop = dp->getText(propertyName.toStdString().c_str());
    if (prop == nullptr)
    {
        sendMessage("Error - unable to find " + deviceName + "/" + propertyName + " property. Aborting.");
        return false;
    }

    for (int i=0;i<prop->ntp;i++) {
        if (strcmp(prop->tp[i].name, elementName.toStdString().c_str()) == 0) {
            value = prop->tp[i].text;
            return true;
        }
    }
    sendMessage("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool Basemodule::sendModNewText  (QString deviceName,QString propertyName,QString elementName, QString text)
{
    //qDebug() << "taskSendNewText";
    INDI::BaseDevice *dp = getDevice(deviceName.toStdString().c_str());

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
            sendNewText(deviceName.toStdString().c_str(),propertyName.toStdString().c_str(),elementName.toStdString().c_str(),text.toStdString().c_str());
            return true;
        }
    }
    sendMessage("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;
}
bool Basemodule::sendModNewSwitch(QString deviceName,QString propertyName,QString elementName, ISState sw)
{
    qDebug() << "taskSendNewSwitch";

    INDI::BaseDevice *dp;
    dp = getDevice(deviceName.toStdString().c_str());

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
        if (prop->r==ISR_1OFMANY) prop->sp[i].s=ISS_OFF;
        if (strcmp(prop->sp[i].name, elementName.toStdString().c_str()) == 0) {
            prop->sp[i].s=sw;
            sendNewSwitch(prop);
            BOOST_LOG_TRIVIAL(debug)  << "SENDNEWSWITCH "<< deviceName.toStdString() <<propertyName.toStdString()<< elementName.toStdString() << " >" << sw << "<";
            return true;
        }
    }
    sendMessage("Error - unable to find " + deviceName + "/" + propertyName + "/" + elementName + " element. Aborting.");
    return false;

}
bool Basemodule::frameSet(QString devicename,double x,double y,double width,double height)
{
    INDI::BaseDevice *dp;
    dp = getDevice(devicename.toStdString().c_str());
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

    sendNewNumber(prop);
    return true;
}
bool Basemodule::frameReset(QString devicename)
{
    INDI::BaseDevice *dp;
    dp = getDevice(devicename.toStdString().c_str());
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

    sendNewSwitch(prop);
    emit askedFrameReset(devicename);
    return true;
}

void Basemodule::createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel, const int &pPropertyPermission,const  QString &pPropertyDevcat, const QString &pPropertyGroup)
{
    BOOST_LOG_TRIVIAL(debug) << "createOstProperty  - " << _modulename.toStdString() << "-" << pPropertyName.toStdString();
    QVariantMap _prop=_ostproperties[pPropertyName].toMap();
    _prop["propertyLabel"]=pPropertyLabel;
    _prop["permission"]=pPropertyPermission;
    _prop["devcat"]=pPropertyDevcat;
    _prop["group"]=pPropertyGroup;
    _prop["name"]=pPropertyName;
    _ostproperties[pPropertyName]=_prop;
    emit moduleEvent("addprop",pPropertyName);
}
void Basemodule::deleteOstProperty(QString propertyName)
{
    BOOST_LOG_TRIVIAL(debug) << "deleteOstProperty  - " << _modulename.toStdString() << "-" << propertyName.toStdString();
    _ostproperties.remove(propertyName);
    emit moduleEvent("delprop",propertyName);

}

void Basemodule::setOstProperty(const QString &pPropertyName, QVariant _value, bool emitEvent)
{
    BOOST_LOG_TRIVIAL(debug) << "setpropvalue  - " << _modulename.toStdString() << "-" << pPropertyName.toStdString();

    //setProperty(propertyName.toStdString().c_str(),propertyValue);
    QVariantMap _prop=_ostproperties[pPropertyName].toMap();
    _prop["value"]=_value;
    _ostproperties[pPropertyName]=_prop;
    if (emitEvent) emit moduleEvent("setpropvalue",pPropertyName);
}
void Basemodule::createOstElement (QString propertyName, QString elementName, QString elementLabel)
{
    QVariantMap _prop=_ostproperties[propertyName].toMap();
    QVariantMap elements=_prop["elements"].toMap();
    QVariantMap element=elements[elementName].toMap();
    element["elementLabel"]=elementLabel;
    elements[elementName]=element;
    _prop["elements"]=elements;
    _ostproperties[propertyName]=_prop;
    emit moduleEvent("addelt",propertyName);
}
void Basemodule::setOstElement    (QString propertyName, QString elementName, QVariant elementValue, bool emitEvent)
{
    QVariantMap _prop=_ostproperties[propertyName].toMap();
    if (_prop.contains("elements")) {
        if (_prop["elements"].toMap().contains(elementName)) {
            QVariantMap elements=_prop["elements"].toMap();
            QVariantMap element=elements[elementName].toMap();
            element["value"]=elementValue;
            elements[elementName]=element;
            _prop["elements"]=elements;
        }
    }
    _ostproperties[propertyName]=_prop;
    if (emitEvent) emit moduleEvent("setpropvalue",propertyName);
}



void Basemodule::loadPropertiesFromFile(QString fileName)
{
    QString val;
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject props = d.object();
    foreach(const QString& key, props.keys()) {
        if (key!="baseVersion"&&key!="baseVersion")
        {
            _ostproperties[key]=props[key].toVariant();
        }
    }

    QByteArray docByteArray = d.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    BOOST_LOG_TRIVIAL(debug) << "loadPropertiesFromFile  - " << _modulename.toStdString() << " - filename=" << fileName.toStdString() << " - " << strJson.toStdString();

}

void Basemodule::savePropertiesToFile(QString fileName)
{
    QVariantMap map = property("ostproperties").toMap();
    /*foreach(const QString& key, map.keys()) {
        if (key!="ostproperties")
        {
            QVariantMap mm=map[key].toMap();
            mm["propertyValue"]=property(key.toStdString().c_str());
            map[key]=mm;
        }
    }*/
    QJsonObject obj =QJsonObject::fromVariantMap(map);
    QJsonDocument doc(obj);

    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(doc.toJson());
    jsonFile.close();

}
void Basemodule::requestProfile(QString profileName)
{
    emit moduleEvent("profilerequest",profileName);
}

void Basemodule::setProfile(QVariantMap profiledata)
{
    foreach(const QString& key, profiledata.keys()) {
        if (_ostproperties.contains(key)) {
            QVariantMap data= profiledata[key].toMap();
            if (_ostproperties[key].toMap().contains("hasprofile")) {
                setOstProperty(key,data["value"],true);
                if (_ostproperties[key].toMap().contains("elements")
                    &&data.contains("elements")) {
                    foreach(const QString& eltkey, profiledata[key].toMap()["elements"].toMap().keys()) {
                        setOstElement(key,eltkey,profiledata[key].toMap()["elements"].toMap()[eltkey].toMap()["value"],true);
                    }

                }
            }
         }

    }

}
