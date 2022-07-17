#include "maincontrol.h"
#include "model/textproperty.h"


MainControl::MainControl(QString name, QString label)
    : Basemodule(name,label)
{

    _moduledescription="Main control module - and so on";


    QDir directory(QCoreApplication::applicationDirPath());
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList() << "*ost*.so");
    QStringList libs = directory.entryList();
    foreach(QString lib, libs)
    {
        QString tt = lib.replace(".so","");
        createOstProperty("mod"+tt,"Load module " + tt,2);
        setOstProperty("mod"+tt,tt);
        auto *tmp = new TextProperty(_modulename,"modules","root",tt,"Load module " + tt,2,0);
        tmp->addText(new TextValue("loadmodname","Give it a name","hint",tt));
        emit propertyCreated(tmp,&_modulename);
        _propertyStore.add(tmp);

    }

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

    QList<TextValue*> texts=prop->getTexts();
    for (int j = 0; j < texts.size(); ++j) {
            if (texts[j]->name()=="loadmodname") {
            auto *tmp = _propertyStore.getText(prop->getDeviceName(),prop->getGroupName(),prop->getName());
            tmp->setText(texts[j]->name(),texts[j]->text());
            _propertyStore.update(tmp);
            tmp->setState(1);
            emit propertyUpdated(tmp,&_modulename);
            emit loadModule(tmp->getName(),texts[j]->text());

        }

    }

}
void MainControl::OnSetPropertyNumber(NumberProperty* prop)
{

}
void MainControl::OnSetPropertySwitch(SwitchProperty* prop)
{


}






