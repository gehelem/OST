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
    Q_UNUSED(dp)
}
void IndiPanel::removeDevice(INDI::BaseDevice *dp)
{
    Q_UNUSED(dp)
}
void IndiPanel::newProperty(INDI::Property *pProperty)
{
    QString dev = pProperty->getDeviceName();
    QString pro = pProperty->getName();
    QString devpro = dev+pro;
    //BOOST_LOG_TRIVIAL(debug) << "Indipanel new property " << devpro.toStdString();
    createOstProperty(devpro,pProperty->getLabel(),pProperty->getPermission(),pProperty->getDeviceName(),pProperty->getGroupName());
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
        case INDI_BLOB: {
            break;
        }
        case INDI_UNKNOWN: {
            break;
        }
    }

}

void IndiPanel::removeProperty(INDI::Property *property)
{
    QString dev = property->getDeviceName();
    QString pro = property->getName();
    QString devpro = dev+pro;
    BOOST_LOG_TRIVIAL(debug) << "indi remove property " << devpro.toStdString();
    deleteOstProperty(devpro);

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
        case INDI_BLOB: {
            break;
        }
        case INDI_UNKNOWN: {
            break;
        }
    }
}
void IndiPanel::newNumber(INumberVectorProperty *nvp)
{
    Q_UNUSED(nvp)
}
void IndiPanel::newText(ITextVectorProperty *tvp)
{
    Q_UNUSED(tvp)
}
void IndiPanel::newLight(ILightVectorProperty *lvp)
{
    Q_UNUSED(lvp)
}
void IndiPanel::newSwitch(ISwitchVectorProperty *svp)
{
    Q_UNUSED(svp)
}
void IndiPanel::newBLOB(IBLOB *bp)
{
    Q_UNUSED(bp)
}
void IndiPanel::newMessage     (INDI::BaseDevice *dp, int messageID)
{
    QString txt= QString::fromStdString(dp->messageQueue(messageID));
    Q_UNUSED(txt)
}


//void IndiPanel::OnSetPropertyText(TextProperty* prop)
//{
//
//}


