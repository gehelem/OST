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
    emit SigNewDevice(dp);
}
void IndiCLient::removeDevice(INDI::BaseDevice *dp)
{
    BOOST_LOG_TRIVIAL(debug) << "Device removed";
    emit SigRemoveDevice(dp);
}
void IndiCLient::newProperty(INDI::Property *property)
{
    std::stringstream stream;


    stream << "Property received: "
    << "Device: " << property->getDeviceName()
    << ". Group: " << property->getGroupName()
    << ". Name: " << property->getName()
    << ". Label: " << property->getName()
    << ". Type: " << _propertyTypesToNamesMap[property->getType()] << ". ";

    switch (property->getType()) {

        case INDI_NUMBER: stream << extract(property->getNumber()); break;
        case INDI_TEXT: stream << extract(property->getText()); break;
        case INDI_SWITCH: stream << extract(property->getSwitch()); break;

        default:
            break;
    }

    BOOST_LOG_TRIVIAL(debug) << stream.str();
    emit SigNewProperty(property);
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
    BOOST_LOG_TRIVIAL(debug) << "Message received";
    emit SigNewMessage(dp,messageID);
}
void IndiCLient::newBLOB(IBLOB *bp)
{
    BOOST_LOG_TRIVIAL(debug) << "BLOB received";
    emit SigNewBLOB(bp);
}

std::string IndiCLient::extract(ITextVectorProperty *pVector) {

    std::stringstream stream;

    stream << "Device: " << pVector->device;
    stream << ". Name: " << pVector->name;
    stream << ". Label: " << pVector->label;
    stream << ". Group: " << pVector->group;
    stream << ". Values = ";

    for (int i = 0; i < pVector->ntp; ++i) {
        stream << "\"" << pVector->tp[i].label << "\"(" << pVector->tp[i].name << ")=";
        stream << "\"" << pVector->tp[i].text << "\" ";
    }
    return stream.str();
}

std::string IndiCLient::extract(INumberVectorProperty *pVector) {

    std::stringstream stream;

    stream << "Device: " << pVector->device;
    stream << ". Name: " << pVector->name;
    stream << ". Label: " << pVector->label;
    stream << ". Group: " << pVector->group;
    stream << ". Values = ";

    for (int i = 0; i < pVector->nnp; ++i) {
        stream << "\"" << pVector->np[i].label << "\"(" << pVector->np[i].name << ")=";
        stream << pVector->np[i].value << " ";
    }
    return stream.str();
}

std::string IndiCLient::extract(ISwitchVectorProperty *pVector) {

    std::stringstream stream;

    stream << "Device: " << pVector->device;
    stream << ". Name: " << pVector->name;
    stream << ". Label: " << pVector->label;
    stream << ". Group: " << pVector->group;
    stream << ". Values = ";

    for (int i = 0; i < pVector->nsp; ++i) {
        stream << "\"" << pVector->sp[i].label << "\"(" << pVector->sp[i].name << ")=";
        stream << pVector->sp[i].s << " ";
    }
    stream <<"- Rule: " << _switchRuleToNamesMap[pVector->r];
    return stream.str();
}

