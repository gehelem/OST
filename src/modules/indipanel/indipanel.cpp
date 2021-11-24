#include "indipanel.h"

IndiPanel *initialize()
{
    IndiPanel *basemodule = new IndiPanel();
    return basemodule;
}

IndiPanel::IndiPanel()
{
    properties=Properties::getInstance();
    _modulename = "indipanel";
    properties->createModule(_modulename,"Indi control panel",9);

}

IndiPanel::~IndiPanel()
{

}

void IndiPanel::newDevice(INDI::BaseDevice *dp)
{
    properties->createCateg(_modulename,QString(dp->getDeviceName()).replace( " ", "" ),dp->getDeviceName(),0);
}
void IndiPanel::removeDevice(INDI::BaseDevice *dp)
{
    properties->deleteCateg(_modulename,QString(dp->getDeviceName()).replace( " ", "" ));
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
            qDebug() << _modulename << QString(property->getDeviceName()).replace( " ", "" ) << QString(property->getGroupName()).replace( " ", "" ) << property->getGroupName();
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




