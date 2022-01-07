#include "guider.h"


GuiderModule *initialize(QString name,QString label)
{
    GuiderModule *basemodule = new GuiderModule(name,label);
    return basemodule;
}

GuiderModule::GuiderModule(QString name,QString label)
    : Basemodule(name,label)
{
    _moduledescription="Guider module";

    _actions = new SwitchProperty(_modulename,"Control","root","actions","Actions",2,0,1);
    _actions->addSwitch(new SwitchValue("condev","Connect devices","hint",0));
    _actions->addSwitch(new SwitchValue("loadconfs","Load devices conf","hint",0));
    _actions->addSwitch(new SwitchValue("abort","Abort","hint",0));
    _actions->addSwitch(new SwitchValue("calibration","Calibration","hint",0));
    emit propertyCreated(_actions,&_modulename);
    _propertyStore.add(_actions);

    _parameters = new NumberProperty(_modulename,"Control","root","parameters","Parameters",2,0);
    _parameters->addNumber(new NumberValue("exposure"      ,"Exposure","hint",_exposure,"",0,120,1));
    emit propertyCreated(_parameters,&_modulename);
    _propertyStore.add(_parameters);

    _values = new NumberProperty(_modulename,"Control","root","values","Values",0,0);
    //_values->addNumber(new NumberValue("xxx","xxx","hint",0,"",0,99,0));
    emit propertyCreated(_values,&_modulename);
    _propertyStore.add(_values);

    _img = new ImageProperty(_modulename,"Control","root","viewer","Image property label",0,0,0);
    emit propertyCreated(_img,&_modulename);
    _propertyStore.add(_img);


}

GuiderModule::~GuiderModule()
{

}
void GuiderModule::OnSetPropertyNumber(NumberProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
}
void GuiderModule::OnSetPropertyText(TextProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;
}

void GuiderModule::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;

    SwitchProperty* wprop = _propertyStore.getSwitch(prop->getDeviceName(),prop->getGroupName(),prop->getName());
    QList<SwitchValue*> switchs=prop->getSwitches();
    for (int j = 0; j < switchs.size(); ++j) {
        if (switchs[j]->name()=="calibration") {
            wprop->setSwitch(switchs[j]->name(),true);
            startCalibration();
        }
        if (switchs[j]->name()=="loadconfs") {
            wprop->setSwitch(switchs[j]->name(),true);
            loadDevicesConfs();
        }
        if (switchs[j]->name()=="abort")  {
            wprop->setSwitch(switchs[j]->name(),true);
            emit abort();
        }
        if (switchs[j]->name()=="condev") {
            wprop->setSwitch(switchs[j]->name(),true);
            connectAllDevices();
        }
        //prop->setSwitches(switchs);
        _propertyStore.update(wprop);
        emit propertyUpdated(wprop,&_modulename);
    }

}

void GuiderModule::newNumber(INumberVectorProperty *nvp)
{


}

void GuiderModule::newBLOB(IBLOB *bp)
{

}

void GuiderModule::newSwitch(ISwitchVectorProperty *svp)
{

}

void GuiderModule::startCalibration()
{
    BOOST_LOG_TRIVIAL(debug) << "Guider module - Start calibration ";
}

