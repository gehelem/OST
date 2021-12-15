#include "dummy.h"
#include "model/textproperty.h"
#include "model/switchproperty.h"

Dummy *initialize(QString name,QString label)
{
    Dummy *basemodule = new Dummy(name,label);
    return basemodule;
}

Dummy::Dummy(QString name,QString label)
    : Basemodule(name,label)
{

    _moduledescription="Dummy module to show what we can do";

    TextProperty* prop = new TextProperty(_modulename,"Examples","Texts","extextRO","Text example - read only",0,0);
    prop->addText(new TextValue("extext1","Text 1","hint","Text 1 initial value" ));
    prop->addText(new TextValue("extext2","Text 2","hint","Text 2 initial value" ));
    prop->addText(new TextValue("extext3","Text 3","hint","Text 3 initial value" ));
    emit propertyCreated(prop,&_modulename);
    _propertyStore.add(prop);

    prop = new TextProperty(_modulename,"Examples","Texts","extextRW","Text example - read/write",2,0);
    prop->addText(new TextValue("extext1","Text 1","hint","Text 1 initial value" ));
    prop->addText(new TextValue("extext2","Text 2","hint","Text 2 initial value" ));
    prop->addText(new TextValue("extext3","Text 3","hint","Text 3 initial value" ));
    emit propertyCreated(prop,&_modulename);
    _propertyStore.add(prop);

    SwitchProperty* props = new SwitchProperty(_modulename,"Examples","Switches","swRO","Switches read only",0,0,0);
    props->addSwitch(new SwitchValue("swRO1","Switch RO 1 ","hint",1));
    props->addSwitch(new SwitchValue("swRO2","Switch RO 2 ","hint",0));
    props->addSwitch(new SwitchValue("swRO3","Switch RO 3 ","hint",0));
    emit propertyCreated(props,&_modulename);
    _propertyStore.add(props);

    props = new SwitchProperty(_modulename,"Examples","Switches","swRW","Switches read/write",1,0,0);
    props->addSwitch(new SwitchValue("swRW1","Switch RW 1 ","hint",0));
    props->addSwitch(new SwitchValue("swRW2","Switch RW 2 ","hint",0));
    props->addSwitch(new SwitchValue("swRW3","Switch RW 3 ","hint",0));
    props->addSwitch(new SwitchValue("swRW4","Switch RW 4 ","hint",1));
    props->addSwitch(new SwitchValue("swRW5","Switch RW 5 ","hint",0));
    props->addSwitch(new SwitchValue("swRW6","Switch RW 6 ","hint",0));
    emit propertyCreated(props,&_modulename);
    _propertyStore.add(props);

    props = new SwitchProperty(_modulename,"Testdevice","Fake Group","swRO","Switches read only",0,0,0);
    props->addSwitch(new SwitchValue("swRO1","Switch RO 1 ","hint",1));
    props->addSwitch(new SwitchValue("swRO2","Switch RO 2 ","hint",0));
    props->addSwitch(new SwitchValue("swRO3","Switch RO 3 ","hint",0));
    emit propertyCreated(props,&_modulename);
    _propertyStore.add(props);
    MessageProperty* mess = new MessageProperty(_modulename,"Testdevice","root","message1","First message zone",0,0,0);
    emit propertyCreated(mess,&_modulename);
    _propertyStore.add(mess);
    mess->setMessage("this is a first message from first device");
    emit propertyCreated(mess,&_modulename);
    mess->setMessage("this is a second message from module");
    emit propertyCreated(mess,&_modulename);
    mess->setMessage("this is a third message from module");
    emit propertyCreated(mess,&_modulename);
    _propertyStore.add(mess);

    mess = new MessageProperty(_modulename,"Examples","root","message2","Second message zone",0,0,0);
    mess->setMessage("this is a message from another device");
    emit propertyCreated(mess,&_modulename);
    _propertyStore.add(mess);



}

Dummy::~Dummy()
{

}


