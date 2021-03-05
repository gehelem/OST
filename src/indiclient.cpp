#include "indiclient.h"
#include <QApplication>
#include <basedevice.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>
#include <QtNetwork>
#include <baseclientqt.h>

#include <sstream>

#include "boost/log/trivial.hpp"

/**************************************************************************************
**
***************************************************************************************/


/* singleton  : getting unique instance if exists, create it if not */
IndiCLient* IndiCLient::getInstance()
{
    if (instance == 0)
    {
        instance = new IndiCLient();
    }

    return instance;
}
/* Null, because instance will be initialized on demand. */
IndiCLient* IndiCLient::instance = 0;

IndiCLient::IndiCLient()
{
    _propertyTypesToNamesMap[0] = "Number";
    _propertyTypesToNamesMap[1] = "Switch";
    _propertyTypesToNamesMap[2] = "Text";
    _propertyTypesToNamesMap[3] = "Light";
    _propertyTypesToNamesMap[4] = "BLOB";

    _switchRuleToNamesMap[0] = "1ofMany";
    _switchRuleToNamesMap[1] = "atMost1";
    _switchRuleToNamesMap[2] = "NofMany";

    _propertyPermsToNamesMap[0] = "ReadOnly";
    _propertyPermsToNamesMap[1] = "WriteOnly";
    _propertyPermsToNamesMap[2] = "ReadWrite";

    _propertyStatesToNamesMap[0] = "Idle";
    _propertyStatesToNamesMap[1] = "OK";
    _propertyStatesToNamesMap[2] = "Busy";
    _propertyStatesToNamesMap[3] = "Alert";
}
void IndiCLient::serverConnected(void)
{
    BOOST_LOG_TRIVIAL(debug) << "Server connected";
    emit SigServerConnected();
}
void IndiCLient::serverDisconnected(int exit_code)
{
    BOOST_LOG_TRIVIAL(debug) << "Server disconnected";
    emit SigServerDisconnected(exit_code);
}
void IndiCLient::newDevice(INDI::BaseDevice *dp)
{
    BOOST_LOG_TRIVIAL(debug) << "New Device: " << dp->getDeviceName();
    emit newDeviceSeen(dp->getDeviceName());
}
void IndiCLient::removeDevice(INDI::BaseDevice *dp)
{
    BOOST_LOG_TRIVIAL(debug) << "Device removed";
    emit deviceRemoved(dp->getDeviceName());
}
void IndiCLient::newProperty(INDI::Property *pProperty)
{
    std::stringstream stream;

    std::string deviceName = pProperty->getDeviceName();
    std::string groupName = pProperty->getGroupName();
    std::string propertyName = pProperty->getName();
    std::string propertyLabel = pProperty->getLabel();
    std::string propertyPermission = _propertyPermsToNamesMap[pProperty->getPermission()];
    int propertyState = pProperty->getState();

    stream << "Property received: "
    << "PropDevice=" << deviceName
    << ". PropGroup=" << groupName
    << ". PropName=" << propertyName
    << ". PropLabel=" << propertyLabel
    << ". PropPermission=" << propertyPermission
    << ". PropState=" << _propertyStatesToNamesMap[propertyState] << '(' << propertyState << ')'
    << ". PropType=" << _propertyTypesToNamesMap[pProperty->getType()] << ". ";

    switch (pProperty->getType()) {

        case INDI_NUMBER: stream << extract(pProperty->getNumber()); break;
        case INDI_TEXT: stream << extract(pProperty->getText()); break;
        case INDI_SWITCH: stream << extract(pProperty->getSwitch()); break;

        default:
            break;
    }

    BOOST_LOG_TRIVIAL(debug) << stream.str();
    emit SigNewProperty(pProperty);
}
void IndiCLient::removeProperty(INDI::Property *property)
{
    BOOST_LOG_TRIVIAL(debug) << "Property Removed: " << property->getDeviceName() << " : " << property->getName();
    emit SigRemoveProperty(property);
}
void IndiCLient::newNumber(INumberVectorProperty *nvp)
{
    //if (strcmp(svp->name,"CONFIG_PROCESS")==0)
    //for (int i=0;i<nvp->nnp;i++) IDLog("Got number %s %s %s %f\n",nvp->device,nvp->name,nvp->np[i].name,nvp->np[i].value);
    BOOST_LOG_TRIVIAL(debug) << "Number received: " << extract(nvp);
    emit SigNewNumber(nvp);
}
void IndiCLient::newText(ITextVectorProperty *tvp)
{
    BOOST_LOG_TRIVIAL(debug) << "Text received: " << extract(tvp);
    emit SigNewText(tvp);
}
void IndiCLient::newSwitch(ISwitchVectorProperty *svp)
{
    //if (strcmp(svp->name,"CONFIG_PROCESS")==0)
    //for (int i=0;i<svp->nsp;i++) IDLog("Got switch %s %s %s\n",svp->device,svp->name,svp->sp[i].name);
    BOOST_LOG_TRIVIAL(debug) << "Switch received: " << extract(svp);
    emit SigNewSwitch(svp);
}
void IndiCLient::newLight(ILightVectorProperty *lvp)
{
    BOOST_LOG_TRIVIAL(debug) << "Light received";
    emit SigNewLight(lvp);
}
void IndiCLient::newMessage(INDI::BaseDevice *dp, int messageID)
{
    BOOST_LOG_TRIVIAL(debug) << "Message received: " << dp->messageQueue(messageID);
    emit SigNewMessage(dp,messageID);
}
void IndiCLient::newBLOB(IBLOB *bp)
{
    BOOST_LOG_TRIVIAL(debug) << "BLOB received: Format=" << bp->format << ". Size (byptes)=" << bp->bloblen;

    const QByteArray data((char*)bp->blob, bp->bloblen);
    QFile outputFile(QString("/tmp/ost_blob_test").append(bp->format));
    outputFile.open(QIODevice::WriteOnly);
    qint64 writtenLength = outputFile.write(data);
    if ( -1 == writtenLength ) {
        BOOST_LOG_TRIVIAL(error) << "written bytes to file: " << writtenLength;
    }
    outputFile.close();
    emit SigNewBLOB(bp);
}

std::string IndiCLient::extract(ITextVectorProperty *pVector) {

    std::stringstream stream;

    std::string deviceName = pVector->device;
    std::string vectorName = pVector->name;
    std::string vectorLabel = pVector->label;
    std::string vectorGroup = pVector->group;
    int vectorState = pVector->s;

    stream << "VectDevice=" << deviceName;
    stream << ". VectGroup=" << vectorGroup;
    stream << ". VectName=" << vectorName;
    stream << ". VectLabel=" << vectorLabel;
    stream << ". State=" << _propertyStatesToNamesMap[vectorState] << '(' << vectorState << ')';
    stream << ". Values: ";

    for (int i = 0; i < pVector->ntp; ++i) {

        IText currentValue = pVector->tp[i];
        std::string textName = currentValue.name;
        std::string textLabel = currentValue.label;
        std::string text = currentValue.text;

        if (vectorName == "DRIVER_INFO" &&
            textName == "DRIVER_INTERFACE" &&
            strtol(text.c_str(), nullptr, 10) & INDI::BaseDevice::CCD_INTERFACE ) {
            setBLOBMode(B_ALSO, deviceName.c_str());
        }

        stream << "  *Name=" << textName;
        stream << ". Label=" << textLabel;
        stream << ". Text=" << text << ". ";
    }
    return stream.str();

}

std::string IndiCLient::extract(INumberVectorProperty *pVector) {

    std::stringstream stream;

    std::string deviceName = pVector->device;
    std::string vectorName = pVector->name;
    std::string vectorLabel = pVector->label;
    std::string vectorGroup = pVector->group;
    int vectorState = pVector->s;

    stream << "VectDevice=" << deviceName;
    stream << ". VectGroup=" << vectorGroup;
    stream << ". VectName=" << vectorName;
    stream << ". VectLabel=" << vectorLabel;
    stream << ". State=" << _propertyStatesToNamesMap[vectorState] << '(' << vectorState << ')';
    stream << ". Values: ";

    for (int i = 0; i < pVector->nnp; ++i) {

        INumber currentValue = pVector->np[i];
        std::string numberName = currentValue.name;
        std::string numberLabel = currentValue.label;
        double numberValue = currentValue.value;
        double numberMin = currentValue.min;
        double numberMax = currentValue.max;
        std::string numberFormat = currentValue.format;
        double numberStep = currentValue.step;

        stream << "  *Name=" << numberName;
        stream << ". Label=" << numberLabel;
        stream << ". Format=" << numberFormat;
        stream << ". Value=" << numberValue;
        stream << ". Min=" << numberMin;
        stream << ". Max=" << numberMax;
        stream << ". Step=" << numberStep << ". ";
    }
    return stream.str();
}

std::string IndiCLient::extract(ISwitchVectorProperty *pVector) {

    std::stringstream stream;

    std::string deviceName = pVector->device;
    std::string vectorName = pVector->name;
    std::string vectorLabel = pVector->label;
    std::string vectorGroup = pVector->group;
    std::string switchRule = _switchRuleToNamesMap[pVector->r];
    int vectorState = pVector->s;

    stream << "VectDevice=" << deviceName;
    stream << ". VectGroup=" << vectorGroup;
    stream << ". VectName=" << vectorName;
    stream << ". VectLabel=" << vectorLabel;
    stream << ". State=" << _propertyStatesToNamesMap[vectorState] << '(' << vectorState << ')';
    stream << ". VectRule=" << switchRule;

    stream << ". Values: ";

    for (int i = 0; i < pVector->nsp; ++i) {

        ISwitch currentValue = pVector->sp[i];
        std::string switchName = currentValue.name;
        std::string switchLabel = currentValue.label;
        bool switchState = currentValue.s;

        stream << "  *Name=" << switchName;
        stream << ". Label: " << switchLabel;
        stream << ". State: " << (switchState ? "true" : "false") << ". ";
    }
    return stream.str();
}

