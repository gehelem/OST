#include "indipanel.h"

IndiPanel *initialize(QString name, QString label)
{
    IndiPanel *basemodule = new IndiPanel(name,label);
    return basemodule;
}

IndiPanel::IndiPanel(QString name, QString label)
    : Basemodule(name,label)
{
    _moduledescription="Indi control panel";
}

IndiPanel::~IndiPanel()
{

}
void IndiPanel::newDevice(INDI::BaseDevice *dp)
{
    MessageProperty* mess = new MessageProperty(_modulename,dp->getDeviceName(),"root",dp->getDeviceName(),dp->getDeviceName(),0,0,0);
    emit propertyCreated(mess,&_modulename);
    _propertyStore.add(mess);
}
void IndiPanel::removeDevice(INDI::BaseDevice *dp)
{
    MessageProperty* mess = new MessageProperty(_modulename,dp->getDeviceName(),"root",dp->getDeviceName(),dp->getDeviceName(),0,0,0);
    emit propertyRemoved(mess,&_modulename);
    _propertyStore.remove(mess);
    ImageProperty* img = new ImageProperty(_modulename,dp->getDeviceName(),QString(dp->getDeviceName()) + " Viewer",QString(dp->getDeviceName()) + "viewer","Image property label",0,0,0);
    emit propertyRemoved(img,&_modulename);
    _propertyStore.remove(img);

}
void IndiPanel::newProperty(INDI::Property *pProperty)
{
    switch (pProperty->getType()) {

        case INDI_NUMBER: {
            emit propertyCreated(PropertyFactory::createProperty(pProperty->getNumber(),&_modulename),&_modulename);
            _propertyStore.add(PropertyFactory::createProperty(pProperty->getNumber(),&_modulename));
            //BOOST_LOG_TRIVIAL(debug) << "Indipanel propeties size " << _propertyStore.getSize();
            if ( (strcmp(pProperty->getName(),"CCD_EXPOSURE")==0) ||
                 (strcmp(pProperty->getName(),"GUIDER_EXPOSURE")==0) )
            {
                INDI::BaseDevice *_wdp=getDevice(pProperty->getDeviceName());
                if (_wdp->getDriverInterface() & INDI::BaseDevice::CCD_INTERFACE)
                {
                    BOOST_LOG_TRIVIAL(debug) << "Setting blob mode for " << _wdp->getDeviceName();
                    setBLOBMode(B_ALSO,_wdp->getDeviceName(),nullptr);
                    ImageProperty* img = new ImageProperty(_modulename,_wdp->getDeviceName(),QString(_wdp->getDeviceName()) + " Viewer",QString(_wdp->getDeviceName()) + "viewer","Image property label",0,0,0);
                    emit propertyCreated(img,&_modulename);
                    _propertyStore.add(img);
                }

            }
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
    image = new Image();
    image->LoadFromBlob(bp);
    image->CalcStats();
    image->computeHistogram();
    image->saveStretchedToJpeg(_webroot+"/"+QString(bp->bvp->device)+".jpeg",100);


    ImageProperty* img = new ImageProperty(
                _modulename,
                bp->bvp->device,
                QString(bp->bvp->device) + " Viewer",
                QString(bp->bvp->device) + "viewer",
                "Image property label",0,0,0
                );
    img->setURL(QString(bp->bvp->device)+".jpeg");
    emit propertyUpdated(img,&_modulename);
    _propertyStore.add(img);

}
void IndiPanel::OnSucessSEP(void)
{
    BOOST_LOG_TRIVIAL(debug) << "IMG stars found " << _solver.stars.size();
}
void IndiPanel::newMessage     (INDI::BaseDevice *dp, int messageID)
{
    QString txt= QString::fromStdString(dp->messageQueue(messageID));

    MessageProperty* mess = new MessageProperty(_modulename,dp->getDeviceName(),"root",dp->getDeviceName(),dp->getDeviceName(),0,0,0);
    mess->setMessage(txt);
    emit propertyUpdated(mess,&_modulename);
    _propertyStore.add(mess);
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

    BOOST_LOG_TRIVIAL(debug) << "Activate blob mode " << dp->getDeviceName();
    if (dp->getDriverInterface() & INDI::BaseDevice::CCD_INTERFACE)
    {
        BOOST_LOG_TRIVIAL(debug) << "Setting blob mode " << dp->getDeviceName();
        sendMessage("Setting blob mode " + QString(dp->getDeviceName()));
        setBLOBMode(B_ALSO,dp->getDeviceName(),nullptr);
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
            BOOST_LOG_TRIVIAL(debug) << "Indipanel number property not found " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();
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

    if (iprop->getType()==INDI_SWITCH) {
        ISwitchVectorProperty *indiprop;
        indiprop =  dp->getSwitch(prop->getName().toStdString().c_str());
        if (indiprop== nullptr)
        {
            BOOST_LOG_TRIVIAL(debug) << "Indipanel switch property not found " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();
            return;
        }
        BOOST_LOG_TRIVIAL(debug) << "Indipanel switch property  " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();

        QList<SwitchValue*> switchs=prop->getSwitches();
        for (int i = 0; i < indiprop->nsp; ++i) {
            for (int j = 0; j < switchs.size(); ++j) {
                if (indiprop->r==ISR_1OFMANY) {
                    indiprop->sp[i].s=ISS_OFF;
                    if (strcmp(switchs[j]->name().toStdString().c_str(),indiprop->sp[i].name)==0) {
                        BOOST_LOG_TRIVIAL(debug) << "ISR_1OFMANY";
                        indiprop->sp[i].s=ISS_ON;
                    }

                }
                if (indiprop->r==ISR_ATMOST1) {
                    if (strcmp(switchs[j]->name().toStdString().c_str(),indiprop->sp[i].name)==0) {
                        BOOST_LOG_TRIVIAL(debug) << "ISR_ATMOST1";
                        indiprop->sp[i].s=ISS_OFF;
                        if (indiprop->sp[i].s==ISS_ON ) indiprop->sp[i].s=ISS_OFF;
                        if (indiprop->sp[i].s==ISS_OFF) indiprop->sp[i].s=ISS_ON;
                    }

                }
                if (indiprop->r==ISR_NOFMANY) {
                    if (strcmp(switchs[j]->name().toStdString().c_str(),indiprop->sp[i].name)==0) {
                        BOOST_LOG_TRIVIAL(debug) << "ISR_NOFMANY";
                        if (indiprop->sp[i].s==ISS_ON ) {
                            indiprop->sp[i].s=ISS_OFF;
                        } else {
                            indiprop->sp[i].s=ISS_ON;
                        }
                    }

                }
            }
        }
        sendNewSwitch(indiprop);
        return;
    }


    return;
}
