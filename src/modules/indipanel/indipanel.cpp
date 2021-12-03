#include "indipanel.h"

IndiPanel *initialize(QString name,QString label)
{
    IndiPanel *basemodule = new IndiPanel(name,label);
    return basemodule;
}

IndiPanel::IndiPanel(QString name,QString label)
    : Basemodule(name,label)
{

}

IndiPanel::~IndiPanel()
{

}
void IndiPanel::newDevice(INDI::BaseDevice *dp)
{
}
void IndiPanel::removeDevice(INDI::BaseDevice *dp)
{
}
void IndiPanel::newProperty(INDI::Property *pProperty)
{
    switch (pProperty->getType()) {

        case INDI_NUMBER: {
            emit propertyCreated(PropertyFactory::createProperty(pProperty->getNumber()),&_modulename);
            _propertyStore.add(PropertyFactory::createProperty(pProperty->getNumber()));
            break;
        }

        case INDI_SWITCH: {
            emit propertyCreated(PropertyFactory::createProperty(pProperty->getSwitch()),&_modulename);
            _propertyStore.add(PropertyFactory::createProperty(pProperty->getSwitch()));
            break;
        }

        case INDI_TEXT: {
            emit propertyCreated(PropertyFactory::createProperty(pProperty->getText()),&_modulename);
            _propertyStore.add(PropertyFactory::createProperty(pProperty->getText()));
            break;
        }

        case INDI_LIGHT: {
            emit propertyCreated( PropertyFactory::createProperty(pProperty->getLight()),&_modulename);
            _propertyStore.add(PropertyFactory::createProperty(pProperty->getLight()));
            break;
        }

        default:
            break;
    }

}

void IndiPanel::removeProperty(INDI::Property *property)
{
    switch (property->getType()) {

        case INDI_NUMBER: {
            emit propertyRemoved(PropertyFactory::createProperty(property->getNumber()),&_modulename);
            _propertyStore.remove(PropertyFactory::createProperty(property->getNumber()));
            break;
        }

        case INDI_SWITCH: {
            emit propertyRemoved(PropertyFactory::createProperty(property->getSwitch()),&_modulename);
            _propertyStore.remove(PropertyFactory::createProperty(property->getSwitch()));
            break;
        }

        case INDI_TEXT: {
            emit propertyRemoved(PropertyFactory::createProperty(property->getText()),&_modulename);
            _propertyStore.remove(PropertyFactory::createProperty(property->getText()));
            break;
        }

        case INDI_LIGHT: {
            emit propertyRemoved( PropertyFactory::createProperty(property->getLight()),&_modulename);
            _propertyStore.remove(PropertyFactory::createProperty(property->getLight()));
            break;
        }

        default:
            break;
    }
}

void IndiPanel::newNumber(INumberVectorProperty *nvp)
{
    _propertyStore.update(PropertyFactory::createProperty(nvp));
    emit propertyUpdated(PropertyFactory::createProperty(nvp),&_modulename);
}

void IndiPanel::newText(ITextVectorProperty *tvp)
{
    _propertyStore.update(PropertyFactory::createProperty(tvp));
    emit propertyUpdated(PropertyFactory::createProperty(tvp),&_modulename);
}

void IndiPanel::newLight(ILightVectorProperty *lvp)
{
    _propertyStore.update(PropertyFactory::createProperty(lvp));
    emit propertyUpdated(PropertyFactory::createProperty(lvp),&_modulename);
}


void IndiPanel::newSwitch(ISwitchVectorProperty *svp)
{
    _propertyStore.update(PropertyFactory::createProperty(svp));
    emit propertyUpdated(PropertyFactory::createProperty(svp),&_modulename);
}

void IndiPanel::newBLOB(IBLOB *bp)
{
}

void IndiPanel::newMessage     (INDI::BaseDevice *dp, int messageID)
{
    QString mess= QString::fromStdString(dp->messageQueue(messageID));
    QString dev = QString::fromStdString(dp->getDeviceName());
    dev.replace(" ","");
    emit newMessageSent(mess,&_modulename,dev);
}


