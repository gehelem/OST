#include "indiclient.h"
#include <QApplication>
#include <basedevice.h>
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
#include <model/propertyfactory.h>

#include "boost/log/trivial.hpp"

/**************************************************************************************
**
***************************************************************************************/


/* singleton  : getting unique instance if exists, create it if not */
IndiCLient* IndiCLient::getInstance()
{
    if (instance == nullptr)
    {
        instance = new IndiCLient();
    }

    return instance;
}
/* Null, because instance will be initialized on demand. */
IndiCLient* IndiCLient::instance = nullptr;

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

    qRegisterMetaType<std::string>();
}
void IndiCLient::serverConnected()
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

    switch (pProperty->getType()) {

        case INDI_NUMBER: {
            emit newPropertyReceived(PropertyFactory::createProperty(pProperty->getNumber()));
            break;
        }

        case INDI_SWITCH: {

            emit newPropertyReceived(PropertyFactory::createProperty(pProperty->getSwitch()));
            break;
        }

        default:
            break;
    }
}
void IndiCLient::removeProperty(INDI::Property *property)
{
    BOOST_LOG_TRIVIAL(debug) << "Property Removed: " << property->getDeviceName() << " : " << property->getName();
    emit SigRemoveProperty(property);
}
void IndiCLient::newNumber(INumberVectorProperty *nvp)
{
    emit propertyUpdated(PropertyFactory::createProperty(nvp));
}
void IndiCLient::newText(ITextVectorProperty *tvp)
{
    BOOST_LOG_TRIVIAL(debug) << "Text received: " << extract(tvp);
    emit SigNewText(tvp);
}
void IndiCLient::newSwitch(ISwitchVectorProperty *svp)
{
    emit newPropertyReceived(PropertyFactory::createProperty(svp));
}
void IndiCLient::newLight(ILightVectorProperty *lvp)
{
    BOOST_LOG_TRIVIAL(debug) << extract(lvp);
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
    emit newBlobReceived(data, bp->format);
}

std::string IndiCLient::extract(ITextVectorProperty *pVector) {

    std::stringstream stream;

    QString deviceName = pVector->device;
    QString vectorGroup = pVector->group;
    QString vectorName = pVector->name;
    QString vectorLabel = pVector->label;
    int vectorPermission = pVector->p;
    double vectorTimeout = pVector->timeout;
    int vectorState = pVector->s;
    QString vectorTimeStamp = pVector->timestamp;

    stream << "VectDevice=" << deviceName.toStdString()
    << ". VectGroup=" << vectorGroup.toStdString()
    << ". VectName=" << vectorName.toStdString()
    << ". VectLabel=" << vectorLabel.toStdString()
    << ". VectPerm=" << vectorPermission
    << ". VectTimeout=" << vectorTimeout
    << ". VectTimeStamp=" << vectorTimeStamp.toStdString()
    << ". State=" << vectorState;

    for (int i = 0; i < pVector->ntp; ++i) {

        IText currentValue = pVector->tp[i];
        QString textName = currentValue.name;
        QString textLabel = currentValue.label;
        QString text = currentValue.text;

        if (vectorName == "DRIVER_INFO" &&
            textName == "DRIVER_INTERFACE" &&
            strtol(text.toStdString().c_str(), nullptr, 10) & INDI::BaseDevice::CCD_INTERFACE ) {
            setBLOBMode(B_ALSO, deviceName.toStdString().c_str());
        }

        stream << "  *Name=" << textName.toStdString()
        << ". Label=" << textLabel.toStdString()
        << ". Text=" << text.toStdString() << ". ";
    }
    return stream.str();

}

std::string IndiCLient::extract(ILightVectorProperty* pVector) {

    std::stringstream stream;

    QString deviceName = pVector->device;
    QString vectorGroup = pVector->group;
    QString vectorName = pVector->name;
    QString vectorLabel = pVector->label;
    int vectorState = pVector->s;
    QString vectorTimeStamp = pVector->timestamp;

    stream << "VectDevice=" << deviceName.toStdString()
    << ". VectGroup=" << vectorGroup.toStdString()
    << ". VectName=" << vectorName.toStdString()
    << ". VectLabel=" << vectorLabel.toStdString()
    << ". VectTimeStamp=" << vectorTimeStamp.toStdString()
    << ". State=" << vectorState
    << ". Values: ";

    for (int i = 0; i < pVector->nlp; ++i) {

        ILight currentValue = pVector->lp[i];
        QString lightName = currentValue.name;
        QString lightLabel = currentValue.label;
        int lightState = currentValue.s;

        stream << "  *Name=" << lightName.toStdString()
        << ". Label: " << lightLabel.toStdString()
        << ". State: " << lightState;
    }
    return stream.str();
}

