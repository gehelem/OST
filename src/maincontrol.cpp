#include "maincontrol.h"


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
//    QDir directory(QCoreApplication::applicationDirPath());
//    directory.setFilter(QDir::Files);
//    directory.setNameFilters(QStringList() << "*ost*.so");
//    QStringList libs = directory.entryList();
//    foreach(QString lib, libs)
//    {
//        BOOST_LOG_TRIVIAL(debug) << "Module lib found " << lib.toStdString();
//        properties->appendElt(_modulename,"loadmodule",lib,OSState::OSS_OFF,lib,"","");
//    }
    Devcat *cat = new Devcat("testdev","device de test",this);
    CreateProperty("testprop","test de propriété",0,0,this);
    //connect(test,&Property::propertyCreated,this,&MainControl::OnPropertyCreated);
    //test->emitProp();

    CreateProperty("testprop","test de propriété dupliquée",0,0,this);
    CreateProperty("testpropdev","test de propriété sur dev",0,0,cat);
    CreateProperty("testprop","test de propriété pas vraiment dupliquée",0,0,cat);
    //connect(test4,&Property::propertyCreated,this,&MainControl::OnPropertyCreated);
    //test4->emitProp();
    QObjectList children = this->children();
    for (int i = 0; i < children.length(); i++)
    {
        BOOST_LOG_TRIVIAL(debug) << "childrens of maincontrol : " << children[i]->metaObject()->className() << "-" << children[i]->objectName().toStdString() << "-" ;//<< ((Property)(*children[i])).getLabel().toStdString();
        if (strcmp(children[i]->metaObject()->className(),"Devcat")==0)
        {
            BOOST_LOG_TRIVIAL(debug) << "childrens of children : ohoh, let's check... " ;
            QObjectList children2 = children[i]->children();
            for (int i = 0; i < children2.length(); i++)
            {
                BOOST_LOG_TRIVIAL(debug) << "childrens of children : " << children2[i]->metaObject()->className() << "-" << children2[i]->objectName().toStdString();

            }


        }
    }
}

MainControl::~MainControl()
{

}







