#include <QtCore>
#include <basedevice.h>
#include "basemodule.h"

Basemodule::Basemodule(QString name,QString label)
    :_modulename(name),
      _modulelabel(label)
{
    _propertyStore.cleanup();
    setVerbose(false);
    _moduledescription="This is a base module, it shouldn't be used as is";
    createOstProperty("moduleName","Module name",0);
    setOstProperty("moduleName",name);

    createOstProperty("moduleLabel","Module label",0);
    setOstProperty("moduleLabel",label);

    createOstProperty("moduleDescription","Module description",0);
    setOstProperty("moduleDescription","This is a base module, it shouldn't be used as is");

    createOstProperty("version","Version",0);
    setOstProperty("version",0.1);

    createOstProperty("baseVersion","BaseVersion",0);
    setOstProperty("baseVersion",0.1);

    createOstProperty("indiConnected","Indi server connected",0);
    setOstProperty("indiConnected",false);

    createOstProperty("message","Message",0);
    setOstProperty("message","First base message");
    //loadAttributesFromFile("/home/gilles/projets/OST/test.json");


    SwitchProperty* props = new SwitchProperty(_modulename,"Indi server","root","connect","Indi server",1,0,1);
    props->addSwitch(new SwitchValue("connect","Connect","hint",false));
    props->addSwitch(new SwitchValue("disconnect","Disconnect","hint",false));
    emit propertyCreated(props,&_modulename);
    _propertyStore.add(props);

    _message = new MessageProperty(_modulename,"root","root","message","message",0,0,0);
    emit propertyCreated(_message,&_modulename);
    _propertyStore.add(_message);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Basemodule::connectIndiTimer);
    timer->start(10000);

    setOstProperty("message","Basemodule init finished");

}
void Basemodule::connectIndiTimer()
{
        if (!isServerConnected()) {
            if (connectServer()){
                newUniversalMessage("Indi server connected");
                sendMessage("Indi server connected");
                setOstProperty("IndiConnected",true);
            } else {
                sendMessage("Couldn't connect to Indi server");
                setOstProperty("IndiConnected",false);
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
    //BOOST_LOG_TRIVIAL(debug) << message.toStdString();
    //emit newMessageSent(mess,&_modulename,_modulename);
    _message->setMessage(mess);
    emit propertyUpdated(_message,&_modulename);
    _propertyStore.add(_message);

}
void Basemodule::OnDumpAsked()
{
    usleep(200*1000);
    emit moduleDumped(_propertyStore.toTreeList(),&_modulename,&_modulelabel,&_moduledescription);
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


void Basemodule::createOstProperty(QString propertyName,QString propertyLabel, int propertyPermission)
{
    QVariantMap map = property("ostproperties").toMap();
    QVariantMap prop;
    //attribute["propertyName"]=propertyName;
    prop["propertyLabel"]=propertyLabel;
    prop["permission"]=propertyPermission;
    map[propertyName]=prop;
    setProperty("ostproperties",map);
}
void Basemodule::setOstProperty(QString propertyName, QVariant propertyValue)
{
    setProperty(propertyName.toStdString().c_str(),propertyValue);
}
void Basemodule::createOstElement (QString propertyName, QString elementName, QString elementLabel)
{
    QVariantMap map = property("ostproperties").toMap();
    QVariantMap prop = map[propertyName].toMap();
    QVariantMap elts = prop["elements"].toMap();
    QVariantMap elt = elts[elementName].toMap();
    //elt["elementName"]=elementName;
    elt["elementLabel"]=elementLabel;
    elts[elementName]=elt;
    prop["elements"]=elts;
    map[propertyName]=prop;
    setProperty("ostproperties",map);
}
void Basemodule::setOstElement    (QString propertyName, QString elementName, QVariant elementValue)
{
     QVariantMap prop= property(propertyName.toStdString().c_str()).toMap();
     prop[elementName]=elementValue;
     setProperty(propertyName.toStdString().c_str(),prop);
}



void Basemodule::loadAttributesFromFile(QString fileName)
{
    QString val;
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject attributes = d.object();
    setProperty("ostattributes",attributes.toVariantMap());

    QByteArray docByteArray = d.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    BOOST_LOG_TRIVIAL(debug) << "loadAttributesFromFile  - " << _modulename.toStdString() << " - " << strJson.toStdString();


}

void Basemodule::saveAttributesToFile(QString fileName)
{
    QVariantMap map = property("ostproperties").toMap();
    foreach(const QString& key, map.keys()) {
        if (key!="ostproperties")
        {
            QVariantMap mm=map[key].toMap();
            mm["propertyValue"]=property(key.toStdString().c_str());
            map[key]=mm;
        }
    }
/*    for (auto m : map)
    {
        QVariantMap mm=m.toMap();
        if (mm["propertyName"]!="ostproperties")
        {
            QString pn = mm["propertyName"].toString();
            mm["propertyValue"]=property(pn.toStdString().c_str());
            map[pn]=mm;
        }

    }*/
    QJsonObject obj =QJsonObject::fromVariantMap(map);
    QJsonDocument doc(obj);
    QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    //QJsonObject jsonobj = mod->property(obj["name"].toString().toStdString().c_str()).toJsonObject();
    //QVariant val = jsonobj["value"].toVariant();

    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(doc.toJson());
    jsonFile.close();

}
