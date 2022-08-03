#include "indipanel.h"

IndiPanel *initialize(QString name, QString label,QString profile)
{
    IndiPanel *basemodule = new IndiPanel(name,label,profile);
    return basemodule;
}

IndiPanel::IndiPanel(QString name, QString label, QString profile)
    : Basemodule(name,label,profile)
{

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
            INumberVectorProperty *vp=pProperty->getNumber();
            for (int i=0;i<vp->nnp;i++) {
                createOstElement(devpro,vp->np[i].name,vp->np[i].label);
                setOstElement(devpro,vp->np[i].name,vp->np[i].value,i==vp->nnp-1);
            }

            break;
        }
        case INDI_SWITCH: {
            ISwitchVectorProperty *vp=pProperty->getSwitch();
            for (int i=0;i<vp->nsp;i++) {
                createOstElement(devpro,vp->sp[i].name,vp->sp[i].label);
                setOstElement(devpro,vp->sp[i].name,vp->sp[i].s,i==vp->nsp-1);
            }
            break;
        }
        case INDI_TEXT: {
            ITextVectorProperty *vp=pProperty->getText();
            for (int i=0;i<vp->ntp;i++) {
                createOstElement(devpro,vp->tp[i].name,vp->tp[i].label);
                setOstElement(devpro,vp->tp[i].name,vp->tp[i].text,i==vp->ntp-1);

            }
            break;
        }
        case INDI_LIGHT: {
            ILightVectorProperty *vp=pProperty->getLight();
            for (int i=0;i<vp->nlp;i++) {
                createOstElement(devpro,vp->lp[i].name,vp->lp[i].label);
                setOstElement(devpro,vp->lp[i].name,vp->lp[i].s,i==vp->nlp-1);
            }
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
    QString dev = nvp->device;
    QString pro = nvp->name;
    QString devpro = dev+pro;
    for (int i=0;i<nvp->nnp;i++) {
        setOstElement(devpro,nvp->np[i].name,nvp->np[i].value,i==nvp->nnp-1);
    }
}
void IndiPanel::newText(ITextVectorProperty *tvp)
{
    QString dev = tvp->device;
    QString pro = tvp->name;
    QString devpro = dev+pro;
    for (int i=0;i<tvp->ntp;i++) {
        setOstElement(devpro,tvp->tp[i].name,tvp->tp[i].text,i==tvp->ntp-1);
    }
}
void IndiPanel::newLight(ILightVectorProperty *lvp)
{
    QString dev = lvp->device;
    QString pro = lvp->name;
    QString devpro = dev+pro;
    for (int i=0;i<lvp->nlp;i++) {
        setOstElement(devpro,lvp->lp[i].name,lvp->lp[i].s,i==lvp->nlp-1);
    }
}
void IndiPanel::newSwitch(ISwitchVectorProperty *svp)
{
    QString dev = svp->device;
    QString pro = svp->name;
    QString devpro = dev+pro;
    for (int i=0;i<svp->nsp;i++) {
        setOstElement(devpro,svp->sp[i].name,svp->sp[i].s,i==svp->nsp-1);
    }
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


