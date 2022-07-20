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
}
void IndiPanel::removeDevice(INDI::BaseDevice *dp)
{
}
void IndiPanel::newProperty(INDI::Property *pProperty)
{
    QString dev = pProperty->getDeviceName();
    QString pro = pProperty->getName();
    QString devpro = dev+pro;
    createOstProperty(devpro,pProperty->getLabel(),pProperty->getPermission(),pProperty->getDeviceName(),pProperty->getGroupName());
    switch (pProperty->getType()) {

        case INDI_NUMBER: {
            //INDI::PropertyView<INumber> prop = pProperty->getNumber();
            //for (INumber elt : prop.np) {
            //    createOstElement(pProperty->getDeviceName()&pProperty->getName(),elt.name,elt.label);
            //}
        }
        case INDI_SWITCH: {
        }
        case INDI_TEXT: {
            //INDI::PropertyView<IText> prop = pProperty->getText();
            //for (IText elt : prop.tp) {
            //    createOstElement(pProperty->getDeviceName()&pProperty->getName(),elt.name,elt.label);
            //}
        }
        case INDI_LIGHT: {
        }
    }

    switch (pProperty->getType()) {

        case INDI_NUMBER: {
            if ( (strcmp(pProperty->getName(),"CCD_EXPOSURE")==0) ||
                 (strcmp(pProperty->getName(),"GUIDER_EXPOSURE")==0) )
            {
                INDI::BaseDevice *_wdp=getDevice(pProperty->getDeviceName());
                if (_wdp->getDriverInterface() & INDI::BaseDevice::CCD_INTERFACE)
                {
                    BOOST_LOG_TRIVIAL(debug) << "Setting blob mode for " << _wdp->getDeviceName();
                    setBLOBMode(B_ALSO,_wdp->getDeviceName(),nullptr);
                }

            }
            break;
        }

        case INDI_SWITCH: {
            break;
        }

        case INDI_TEXT: {
            break;
        }

        case INDI_LIGHT: {
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
            break;
        }

        case INDI_SWITCH: {
            break;
        }

        case INDI_TEXT: {
            break;
        }

        case INDI_LIGHT: {
            break;
        }

        default:
            break;
    }
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
    image = new Image();
    image->LoadFromBlob(bp);
    image->CalcStats();
    image->computeHistogram();
    image->saveStretchedToJpeg(_webroot+"/"+QString(bp->bvp->device)+".jpeg",100);

}
void IndiPanel::OnSucessSEP(void)
{
    BOOST_LOG_TRIVIAL(debug) << "IMG stars found " << _solver.stars.size();
}
void IndiPanel::newMessage     (INDI::BaseDevice *dp, int messageID)
{
    QString txt= QString::fromStdString(dp->messageQueue(messageID));
}


//void IndiPanel::OnSetPropertyText(TextProperty* prop)
//{
//
//}


