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
void IndiPanel::changeProp(Prop prop)
{
    QString dev = properties->getDevcat(_modulename,prop.categname).categlabel;
    if (prop.typ== PT_TEXT)
    {
        for(auto e : prop.t)
        {
            sendModNewText(dev,prop.propname,e.name,e.text);
        }

    }
    if (prop.typ== PT_NUM)
    {
        for(auto e : prop.n)
        {
            sendModNewNumber(dev,prop.propname,e.name,e.value);
        }

    }
    if (prop.typ== PT_SWITCH)
    {
        for(auto e : prop.s)
        {
            sendModNewSwitch(dev,prop.propname,e.name,transw(e.s));
            BOOST_LOG_TRIVIAL(debug)  << _modulename.toStdString() << " MOD SWITCH " << prop.propname.toStdString() << " " << e.name.toStdString();

        }

    }
}
void IndiPanel::newDevice(INDI::BaseDevice *dp)
{
    properties->createDevcat(_modulename,QString(dp->getDeviceName()).replace( " ", "" ),dp->getDeviceName(),0);
}
void IndiPanel::removeDevice(INDI::BaseDevice *dp)
{
    properties->deleteDevcat(_modulename,QString(dp->getDeviceName()).replace( " ", "" ));
}
void IndiPanel::newProperty(INDI::Property *property)
{
   //Properties::createGroup(QString modulename, QString categname, QString groupname,  QString grouplabel,int order)
    //if (properties->getGroup(QString(property->getDeviceName()).replace( " ", "" ), QString(property->getGroupName()).replace( " ", "" )).grouplabel != property->getGroupName())
    //    {
            properties->createGroup(_modulename,
                                QString(property->getDeviceName()).replace( " ", "" ),
                                QString(property->getGroupName()).replace( " ", "" ),
                                property->getGroupName(),0);
            BOOST_LOG_TRIVIAL(debug)  << _modulename.toStdString() << QString(property->getDeviceName()).replace( " ", "" ).toStdString() << QString(property->getGroupName()).replace( " ", "" ).toStdString() << QString(property->getName()).toStdString();
    //    }

   Prop wprop;
   wprop.modulename=_modulename;
   wprop.propname=QString(property->getName()).replace( " ", "" );
   wprop.categname=QString(property->getDeviceName()).replace( " ", "" );
   wprop.groupname=QString(property->getGroupName()).replace( " ", "" );
   wprop.label=property->getLabel();
   switch (property->getType()) {
   case INDI_LIGHT:
       wprop.typ=PT_LIGHT;
       break;
   case INDI_NUMBER:
       wprop.typ=PT_NUM;
       break;
   case INDI_TEXT:
       wprop.typ=PT_TEXT;
       break;
   case INDI_SWITCH:
       wprop.typ=PT_SWITCH;
       break;
   case INDI_BLOB:
       wprop.typ=PT_IMAGE;
       break;
   case INDI_UNKNOWN:
       wprop.typ=PT_TEXT;
       break;
   }

   if (property->getPermission()==IP_RO) wprop.perm=OP_RO;
   if (property->getPermission()==IP_WO) wprop.perm=OP_WO;
   if (property->getPermission()==IP_RW) wprop.perm=OP_RW;


   properties->createProp(_modulename,wprop.propname,wprop);
   switch (property->getType()) {
   case INDI_LIGHT:
       for (int i =0;i<property->getLight()->count();i++) {
           properties->appendElt(_modulename,wprop.propname,property->getLight()->lp[i].name,/*property->getLight()->lp[i].s*/ OPS_OK,property->getLight()->lp[i].label,"","");
       }
       break;
   case INDI_NUMBER:
       for (int i =0;i<property->getNumber()->count();i++) {
           properties->appendElt(_modulename,wprop.propname,property->getNumber()->np[i].name,property->getNumber()->np[i].value,property->getNumber()->np[i].label,"","");
       }
       break;
   case INDI_TEXT:
       for (int i =0;i<property->getText()->count();i++) {
           properties->appendElt(_modulename,wprop.propname,property->getText()->tp[i].name,property->getText()->tp[i].text,property->getText()->tp[i].label,"","");
       }
       break;
   case INDI_SWITCH:
       for (int i =0;i<property->getSwitch()->count();i++) {
           properties->appendElt(_modulename,wprop.propname,property->getSwitch()->sp[i].name,/*property->getSwitch()->sp[i].s*/ OSS_OFF,property->getSwitch()->sp[i].label,"","");
       }
       break;
   case INDI_BLOB:
       break;
   case INDI_UNKNOWN:
       break;
   }

   properties->emitProp(_modulename,wprop.propname);

   //properties->dumproperties();
}
void IndiPanel::removeProperty(INDI::Property *property)
{
    properties->deleteProp(_modulename,property->getName());
}

void IndiPanel::newNumber(INumberVectorProperty *nvp)
{
    for (int i =0;i<nvp->nnp;i++) {
        properties->setElt(_modulename,QString(nvp->name).replace( " ", "" ),QString(nvp->np[i].name).replace( " ", "" ),nvp->np[i].value);
    }
    properties->emitProp(_modulename,QString(nvp->name).replace( " ", "" ));
}

void IndiPanel::newText(ITextVectorProperty *tvp)
{
    for (int i =0;i<tvp->ntp;i++) {
        properties->setElt(_modulename,QString(tvp->name).replace( " ", "" ),QString(tvp->tp[i].name).replace( " ", "" ),tvp->tp[i].text);
    }
    properties->emitProp(_modulename,QString(tvp->name).replace( " ", "" ));
}

void IndiPanel::newLight(ILightVectorProperty *lvp)
{
    for (int i =0;i<lvp->nlp;i++) {
        properties->setElt(_modulename,QString(lvp->name).replace( " ", "" ),QString(lvp->lp[i].name).replace( " ", "" ),lvp->lp[i].s);
    }
    properties->emitProp(_modulename,QString(lvp->name).replace( " ", "" ));

}


void IndiPanel::newSwitch(ISwitchVectorProperty *svp)
{
    for (int i =0;i<svp->nsp;i++) {
        properties->setElt(_modulename,QString(svp->name).replace( " ", "" ),QString(svp->sp[i].name).replace( " ", "" ),svp->sp[i].s);
    }
    properties->emitProp(_modulename,QString(svp->name).replace( " ", "" ));


}

void IndiPanel::newBLOB(IBLOB *bp)
{

}

ISState IndiPanel::transw(OSState osw)
{
    if (osw == OSS_ON) return ISS_ON; else return ISS_OFF;
}



