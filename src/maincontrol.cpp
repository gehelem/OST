#include "maincontrol.h"
#include "model/textproperty.h"


MainControl::MainControl(QString name,QString label)
    : Basemodule(name,label)
{
//    properties=Properties::getInstance();
//    _devices["camera"]="";
//    _devices["focuser"]="";
//    properties->createDevcat(_modulename,"indi","Indi parameters",9);
//    properties->createProp(_modulename,"server","Server"    ,PT_TEXT,"indi","",OP_RW,OSRule::OSR_NOFMANY,0,OPState::OPS_IDLE,"","",1);
//    properties->appendElt(_modulename,"server","host","localhost","indi server host name","","");
//    properties->appendElt(_modulename,"server","port","7624","indi server port","","");
//
//    properties->createDevcat(_modulename,"modules","Modules",1);
//    properties->createProp(_modulename,"loadmodule","Load module"    ,PT_SWITCH,"modules","",OP_RW,OSRule::OSR_1OFMANY,0,OPState::OPS_IDLE,"","",1);
//

    TextProperty* prop = new TextProperty(_modulename,"modules","root","availablemodules","Available modules",1,0);

    QDir directory(QCoreApplication::applicationDirPath());
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList() << "*ost*.so");
    QStringList libs = directory.entryList();
    foreach(QString lib, libs)
    {
        BOOST_LOG_TRIVIAL(debug) << "Module lib found " << lib.toStdString();
        prop->addText(new TextValue(lib,lib,"hint",lib));
    }

    emit propertyCreated(prop,&_modulename);
    _propertyStore.add(prop);

    //ITextVectorProperty testtext;
    //strcpy(testtext.name, "proprtest");
    //strcpy(testtext.label, "propriété de test");
    //Property test = new Property("root","root","proptest","propriété de test",1,0,this);
    //BOOST_LOG_TRIVIAL(debug) << "Maincontrol properties size before " << _propertyStore.getSize();
    //BOOST_LOG_TRIVIAL(debug) << "Maincontrol properties size after " << _propertyStore.getSize();
}

MainControl::~MainControl()
{

}
void MainControl::newProperty(INDI::Property *pProperty)
{
}






