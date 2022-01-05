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
    if (libs.size() > 0) {
        QString ll = libs[0];
        _selectedlibrary=ll;
        _chooseModule->setSwitch(ll.replace(".so",""),true);
    }
    emit propertyCreated(_chooseModule,&_modulename);
    _propertyStore.add(_chooseModule);

    _giveItaName = new TextProperty(_modulename,"modules","root","giveItaName","Give it a name",2,0);
    _giveItaName->addText(new TextValue("givemeaname","Name","hint","Give me another name please"));
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
    if (!(prop->getModuleName()==_modulename)) return;

    if (prop->getName()==_giveItaName->getName()) {
        QList<TextValue*> texts=prop->getTexts();
        for (int j = 0; j < texts.size(); ++j) {
            _giveItaName->setText(texts[j]->name(),texts[j]->text());
            _selectedlabel=texts[j]->text();
            _propertyStore.update(_giveItaName);
            emit propertyUpdated(_giveItaName,&_modulename);
        }

    }

}
void MainControl::OnSetPropertyNumber(NumberProperty* prop)
{

}
void MainControl::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
    SwitchProperty* wprop = _propertyStore.getSwitch(prop->getDeviceName(),prop->getGroupName(),prop->getName());
    QList<SwitchValue*> switchs=prop->getSwitches();

    if (prop->getName()==_chooseModule->getName()) {
        _chooseModule->setState(3);
        for (int j = 0; j < switchs.size(); ++j) {
            _selectedlibrary=switchs[j]->name()+".so";
            _chooseModule->setSwitch(switchs[j]->name(),true);
            _chooseModule->setState(1);
            _propertyStore.update(_chooseModule);
            emit propertyUpdated(_chooseModule,&_modulename);
            BOOST_LOG_TRIVIAL(debug) << "Mainctl choose module : " << _selectedlibrary.toStdString();
        }
    }

    if (prop->getName()==_loadModule->getName()) {
        _loadModule->setState(1);
        BOOST_LOG_TRIVIAL(debug) << "Mainctl load module : " << _selectedlibrary.toStdString() << "-" << _selectedlabel.toStdString();
        _propertyStore.update(_loadModule);
        emit propertyUpdated(_loadModule,&_modulename);
        emit loadModule(_selectedlibrary,_selectedlabel);
    }

    BOOST_LOG_TRIVIAL(debug) << "Mainctl switch property item modified " << wprop->getName().toStdString();


}






