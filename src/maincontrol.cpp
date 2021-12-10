#include "maincontrol.h"
#include "model/textproperty.h"


MainControl::MainControl(QString name,QString label)
    : Basemodule(name,label)
{

    _moduledescription="Main control module - and so on";

    TextProperty* prop = new TextProperty(_modulename,"modules","root","availablemodules","Available modules",0,0);

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


}

MainControl::~MainControl()
{

}
void MainControl::newProperty(INDI::Property *pProperty)
{
}






