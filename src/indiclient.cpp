#include <QApplication>
#include <basedevice.h>
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>
#include <QtNetwork>
#include <baseclientqt.h>
#include "boost/log/trivial.hpp"

#include "indiclient.h"
#include "utils/propertyfactory.h"

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

        case INDI_TEXT: {

            emit newPropertyReceived(PropertyFactory::createProperty(pProperty->getText()));
            break;
        }

        case INDI_LIGHT: {

            emit newPropertyReceived(PropertyFactory::createProperty(pProperty->getLight()));
            break;
        }

        default:
            break;
    }
}
void IndiCLient::removeProperty(INDI::Property *pProperty)
{
    switch (pProperty->getType()) {

        case INDI_NUMBER: {
            emit propertyRemoved(PropertyFactory::createProperty(pProperty->getNumber()));
            break;
        }

        case INDI_SWITCH: {

            emit propertyRemoved(PropertyFactory::createProperty(pProperty->getSwitch()));
            break;
        }

        case INDI_TEXT: {

            emit propertyRemoved(PropertyFactory::createProperty(pProperty->getText()));
            break;
        }

        case INDI_LIGHT: {

            emit propertyRemoved(PropertyFactory::createProperty(pProperty->getLight()));
            break;
        }

        default:
            break;
    }
}
void IndiCLient::newNumber(INumberVectorProperty *nvp)
{
    emit propertyUpdated(PropertyFactory::createProperty(nvp));
}
void IndiCLient::newText(ITextVectorProperty *tvp)
{
    emit propertyUpdated(PropertyFactory::createProperty(tvp));
}
void IndiCLient::newSwitch(ISwitchVectorProperty *svp)
{
    emit propertyUpdated(PropertyFactory::createProperty(svp));
}
void IndiCLient::newLight(ILightVectorProperty *lvp)
{
    emit propertyUpdated(PropertyFactory::createProperty(lvp));
}
void IndiCLient::newMessage(INDI::BaseDevice *dp, int messageID)
{
    emit messageReceived(QString(dp->messageQueue(messageID).c_str()));
}
void IndiCLient::newBLOB(IBLOB *bp)
{
    BOOST_LOG_TRIVIAL(debug) << "BLOB received: Format=" << bp->format << ". Size (byptes)=" << bp->bloblen;
    const QByteArray data((char*)bp->blob, bp->bloblen);
    emit newBlobReceived(data, bp->format);
}


