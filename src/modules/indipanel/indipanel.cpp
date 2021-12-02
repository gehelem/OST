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
            emit propertyCreated(PropertyFactory::createProperty(pProperty->getNumber()));
            _propertyStore.add(PropertyFactory::createProperty(pProperty->getNumber()));
            break;
        }

        case INDI_SWITCH: {
            emit propertyCreated(PropertyFactory::createProperty(pProperty->getSwitch()));
            break;
        }

        case INDI_TEXT: {
            emit propertyCreated(PropertyFactory::createProperty(pProperty->getText()));
            break;
        }

        case INDI_LIGHT: {
            emit propertyCreated( PropertyFactory::createProperty(pProperty->getLight()));
            break;
        }

        default:
            break;
    }

}

void IndiPanel::removeProperty(INDI::Property *property)
{
}

void IndiPanel::newNumber(INumberVectorProperty *nvp)
{
}

void IndiPanel::newText(ITextVectorProperty *tvp)
{
}

void IndiPanel::newLight(ILightVectorProperty *lvp)
{
}


void IndiPanel::newSwitch(ISwitchVectorProperty *svp)
{
}

void IndiPanel::newBLOB(IBLOB *bp)
{
}



