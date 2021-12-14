#include "indipanel.h"

IndiPanel *initialize(QString name,QString label)
{
    IndiPanel *basemodule = new IndiPanel(name,label);
    return basemodule;
}

IndiPanel::IndiPanel(QString name,QString label)
    : Basemodule(name,label)
{
    _moduledescription="Indi control panel";

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
            emit propertyCreated(PropertyFactory::createProperty(pProperty->getNumber(),&_modulename),&_modulename);
            _propertyStore.add(PropertyFactory::createProperty(pProperty->getNumber(),&_modulename));
            //BOOST_LOG_TRIVIAL(debug) << "Indipanel propeties size " << _propertyStore.getSize();
            break;
        }

        case INDI_SWITCH: {
            emit propertyCreated(PropertyFactory::createProperty(pProperty->getSwitch(),&_modulename),&_modulename);
            _propertyStore.add(PropertyFactory::createProperty(pProperty->getSwitch(),&_modulename));
            //BOOST_LOG_TRIVIAL(debug) << "Indipanel propeties size " << _propertyStore.getSize();
            break;
        }

        case INDI_TEXT: {
            emit propertyCreated(PropertyFactory::createProperty(pProperty->getText(),&_modulename),&_modulename);
            _propertyStore.add(PropertyFactory::createProperty(pProperty->getText(),&_modulename));
            //BOOST_LOG_TRIVIAL(debug) << "Indipanel propeties size " << _propertyStore.getSize();
            break;
        }

        case INDI_LIGHT: {
            emit propertyCreated( PropertyFactory::createProperty(pProperty->getLight(),&_modulename),&_modulename);
            _propertyStore.add(PropertyFactory::createProperty(pProperty->getLight(),&_modulename));
            //BOOST_LOG_TRIVIAL(debug) << "Indipanel propeties size " << _propertyStore.getSize();
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
            emit propertyRemoved(PropertyFactory::createProperty(property->getNumber(),&_modulename),&_modulename);
            _propertyStore.remove(PropertyFactory::createProperty(property->getNumber(),&_modulename));
            //BOOST_LOG_TRIVIAL(debug) << "Indipanel propeties size " << _propertyStore.getSize();
            break;
        }

        case INDI_SWITCH: {
            emit propertyRemoved(PropertyFactory::createProperty(property->getSwitch(),&_modulename),&_modulename);
            _propertyStore.remove(PropertyFactory::createProperty(property->getSwitch(),&_modulename));
            //BOOST_LOG_TRIVIAL(debug) << "Indipanel propeties size " << _propertyStore.getSize();
            break;
        }

        case INDI_TEXT: {
            emit propertyRemoved(PropertyFactory::createProperty(property->getText(),&_modulename),&_modulename);
            _propertyStore.remove(PropertyFactory::createProperty(property->getText(),&_modulename));
            //BOOST_LOG_TRIVIAL(debug) << "Indipanel propeties size " << _propertyStore.getSize();
            break;
        }

        case INDI_LIGHT: {
            emit propertyRemoved( PropertyFactory::createProperty(property->getLight(),&_modulename),&_modulename);
            _propertyStore.remove(PropertyFactory::createProperty(property->getLight(),&_modulename));
            //BOOST_LOG_TRIVIAL(debug) << "Indipanel propeties size " << _propertyStore.getSize();
            break;
        }

        default:
            break;
    }
}

void IndiPanel::newNumber(INumberVectorProperty *nvp)
{
    _propertyStore.update(PropertyFactory::createProperty(nvp,&_modulename));
    emit propertyUpdated(PropertyFactory::createProperty(nvp,&_modulename),&_modulename);
}

void IndiPanel::newText(ITextVectorProperty *tvp)
{
    _propertyStore.update(PropertyFactory::createProperty(tvp,&_modulename));
    emit propertyUpdated(PropertyFactory::createProperty(tvp,&_modulename),&_modulename);
}

void IndiPanel::newLight(ILightVectorProperty *lvp)
{
    _propertyStore.update(PropertyFactory::createProperty(lvp,&_modulename));
    emit propertyUpdated(PropertyFactory::createProperty(lvp,&_modulename),&_modulename);
}


void IndiPanel::newSwitch(ISwitchVectorProperty *svp)
{
    _propertyStore.update(PropertyFactory::createProperty(svp,&_modulename));
    emit propertyUpdated(PropertyFactory::createProperty(svp,&_modulename),&_modulename);
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


void IndiPanel::OnSetPropertyText(TextProperty* prop)
{

    if (!(prop->getModuleName()==_modulename)) return;

    INDI::BaseDevice *dp = getDevice(prop->getDeviceName().toStdString().c_str());
    if (dp== nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) << "Indipanel device not found " << prop->getDeviceName().toStdString();
        return;
    }
    INDI::Property *iprop;
    iprop =  dp->getProperty(prop->getName().toStdString().c_str());
    if (iprop== nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) << "Indipanel property not found " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();
        return;
    }

    if (iprop->getType()==INDI_TEXT) {
        ITextVectorProperty *inditprop;
        inditprop =  dp->getText(prop->getName().toStdString().c_str());
        if (inditprop== nullptr)
        {
            BOOST_LOG_TRIVIAL(debug) << "Indipanel text property not found " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();
            return;
        }
        QList<TextValue*> texts=prop->getTexts();
        for (int j = 0; j < inditprop->ntp; ++j) {
                for (int i = 0; i < texts.size(); ++i) {
                    if (strcmp(texts[i]->name().toStdString().c_str(),inditprop->tp[j].name)==0) {
                        strcpy(inditprop->tp[j].text,texts[i]->text().toStdString().c_str());
                        BOOST_LOG_TRIVIAL(debug) << "Indipanel text property item  modified "
                                                 << prop->getName().toStdString() << "/"
                                                 << texts[j]->name().toStdString() << "/"
                                                 << texts[j]->text().toStdString();
                    }
                }
        }
        sendNewText(inditprop);
        return;
    }


    return;

}

void IndiPanel::OnSetPropertyNumber(NumberProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;

    INDI::BaseDevice *dp = getDevice(prop->getDeviceName().toStdString().c_str());
    if (dp== nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) << "Indipanel device not found " << prop->getDeviceName().toStdString();
        return;
    }
    INDI::Property *iprop;
    iprop =  dp->getProperty(prop->getName().toStdString().c_str());
    if (iprop== nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) << "Indipanel property not found " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();
        return;
    }

    if (iprop->getType()==INDI_NUMBER) {
        INumberVectorProperty *indiprop;
        indiprop =  dp->getNumber(prop->getName().toStdString().c_str());
        if (indiprop== nullptr)
        {
            BOOST_LOG_TRIVIAL(debug) << "Indipanel text property not found " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();
            return;
        }
        QList<NumberValue*> numbers=prop->getNumbers();
        for (int i = 0; i < indiprop->nnp; ++i) {
            for (int j = 0; j < numbers.size(); ++j) {
                if (strcmp(numbers[j]->name().toStdString().c_str(),indiprop->np[i].name)==0) {
                    //strcpy(inditprop->tp[i].text,texts[j]->text().toStdString().c_str());
                    indiprop->np[i].value=numbers[j]->getValue();
                    BOOST_LOG_TRIVIAL(debug) << "Indipanel number propertyitem  modified " << indiprop->np[i].name << "/" << indiprop->np[i].value;
                }
            }
        }
        sendNewNumber(indiprop);
        return;
    }


    return;
}
void IndiPanel::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
}
