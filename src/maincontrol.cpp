#include "maincontrol.h"
#include "model/textproperty.h"


MainControl::MainControl(QString name, QString label)
    : Basemodule(name,label)
{

    _moduledescription="Main control module - and so on";

    _chooseModule = new SwitchProperty(_modulename,"modules","root","choosemodule","Choose a module",2,0,0);

    QDir directory(QCoreApplication::applicationDirPath());
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList() << "*ost*.so");
    QStringList libs = directory.entryList();
    foreach(QString lib, libs)
    {
        QString tt = lib.replace(".so","");
        //BOOST_LOG_TRIVIAL(debug) << "Module lib found " << lib.toStdString();
        _chooseModule->addSwitch(new SwitchValue(tt,lib,"hint",0));
    }

    emit propertyCreated(_chooseModule,&_modulename);
    _propertyStore.add(_chooseModule);

    _giveItaName = new TextProperty(_modulename,"modules","root","giveItaName","Give it a name",2,0);
    _giveItaName->addText(new TextValue("givemeaname","Name","hint","Give ma another name, please"));
    emit propertyCreated(_giveItaName,&_modulename);
    _propertyStore.add(_giveItaName);

    _loadModule = new SwitchProperty(_modulename,"modules","root","loadmodule","Load this module",2,0,0);
    _loadModule->addSwitch(new SwitchValue("load","Load","hint",0));
    emit propertyCreated(_loadModule,&_modulename);
    _propertyStore.add(_loadModule);

    /*SwitchProperty* _chooseModule;
    TextProperty* _giveItaName;
    SwitchProperty* _loadModule;*/

}

MainControl::~MainControl()
{

}
void MainControl::newProperty(INDI::Property *pProperty)
{
}
void MainControl::OnSetPropertyText(TextProperty* prop)
{

}
void MainControl::OnSetPropertyNumber(NumberProperty* prop)
{

}
void MainControl::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
    SwitchProperty* wprop = _propertyStore.getSwitch(prop->getDeviceName(),prop->getGroupName(),prop->getName());
    QList<SwitchValue*> switchs=prop->getSwitches();
    for (int j = 0; j < switchs.size(); ++j) {
        BOOST_LOG_TRIVIAL(debug) << "Mainctl switch property rcv " << switchs[j]->name().toStdString();
        wprop->setState(1);
    }
    _propertyStore.update(wprop);
    emit propertyUpdated(wprop,&_modulename);
    BOOST_LOG_TRIVIAL(debug) << "Mainctl switch property item modified " << wprop->getName().toStdString();


}






