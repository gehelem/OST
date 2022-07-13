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
    setProperty("Name","dummymodule");
    setProperty("Version",0.2);
    setProperty("MyDummyProperty","JohnDoe");
    setProperty("Description","Dummy module description, just for demo purpose");

    TextProperty* prop = new TextProperty(_modulename,"Examples","Texts","extextRO","Text example - read only",0,0);
    prop->addText(new TextValue("extext1","Text 1","hint","Text 1 initial value" ));
    prop->addText(new TextValue("extext2","Text 2","hint","Text 2 initial value" ));
    prop->addText(new TextValue("extext3","Text 3","hint","Text 3 initial value" ));
    emit propertyCreated(prop,&_modulename);
    _propertyStore.add(prop);
    createOstProperty("extext1","Text 1",0);
    setOstProperty("extext1","Text 1 initial value");

    prop = new TextProperty(_modulename,"Examples","Texts","extextRW","Text example - read/write",2,0);
    prop->addText(new TextValue("extext1","Text 1","hint","Text 1 initial value" ));
    prop->addText(new TextValue("extext2","Text 2","hint","Text 2 initial value" ));
    prop->addText(new TextValue("extext3","Text 3","hint","Text 3 initial value" ));
    emit propertyCreated(prop,&_modulename);
    _propertyStore.add(prop);

    SwitchProperty* props = new SwitchProperty(_modulename,"Examples","Switches","swRO","Switches read only",0,0,0);
    props->addSwitch(new SwitchValue("swRO1","Switch RO 1 ","hint",true));
    props->addSwitch(new SwitchValue("swRO2","Switch RO 2 ","hint",false));
    props->addSwitch(new SwitchValue("swRO3","Switch RO 3 ","hint",false));
    emit propertyCreated(props,&_modulename);
    _propertyStore.add(props);

    props = new SwitchProperty(_modulename,"Examples","Switches","swRW1ofmany","Switches read/write 1 of many",2,0,0);
    props->addSwitch(new SwitchValue("swRW1","Switch RW 1 of many 1 ","hint",false));
    props->addSwitch(new SwitchValue("swRW2","Switch RW 2 of many ","hint",true));
    props->addSwitch(new SwitchValue("swRW3","Switch RW 3 of many ","hint",false));
    emit propertyCreated(props,&_modulename);
    _propertyStore.add(props);

    props = new SwitchProperty(_modulename,"Examples","Switches","swRWatmostone","Switches read/write at most 1",2,0,1);
    props->addSwitch(new SwitchValue("swRW1","Switch RW atmost 1 ","hint",true));
    props->addSwitch(new SwitchValue("swRW2","Switch RW atmost 2 ","hint",false));
    props->addSwitch(new SwitchValue("swRW3","Switch RW atmost 3 ","hint",false));
    emit propertyCreated(props,&_modulename);
    _propertyStore.add(props);

    props = new SwitchProperty(_modulename,"Examples","Switches","swRWnoofmany","Switches read/write any",2,0,2);
    props->addSwitch(new SwitchValue("swRW1","Switch RW any 1 ","hint",false));
    props->addSwitch(new SwitchValue("swRW2","Switch RW any 2 ","hint",true));
    props->addSwitch(new SwitchValue("swRW3","Switch RW any 3 ","hint",true));
    emit propertyCreated(props,&_modulename);
    _propertyStore.add(props);

    props = new SwitchProperty(_modulename,"Testdevice","Fake Group","swRO","Switches read only",0,0,0);
    props->addSwitch(new SwitchValue("swRO1","Switch RO 1 ","hint",true));
    props->addSwitch(new SwitchValue("swRO2","Switch RO 2 ","hint",false));
    props->addSwitch(new SwitchValue("swRO3","Switch RO 3 ","hint",false));
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

    setProperty("Message","Dummy module init finished");


}

Dummy::~Dummy()
{

}

void Dummy::OnSetPropertyText(TextProperty* prop)
{

    if (!(prop->getModuleName()==_modulename)) return;
    QList<TextValue*> texts=prop->getTexts();
    for (int i = 0; i < texts.size(); ++i) {
        //setProperty("Message","RCV Text :" + prop->getName() + ":" + texts[i]->name()+ ":" + texts[i]->text());
        texts[i]->setText(texts[i]->text()+" modified by module");
        //titi[texts[i]->name()]=texts[0]->text();
        //jsonobj["name"]=texts[i]->name();
        //jsonobj["label"]=texts[i]->label();
        //jsonobj["text"]=texts[i]->text();
        //jsonarray.append(jsonobj);
        setOstProperty(texts[i]->name(),texts[i]->text());
    }

    prop->setState(1);
    emit propertyCreated(prop,&_modulename);
    _propertyStore.add(prop);

}
void Dummy::OnSetPropertyNumber(NumberProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;

    QList<NumberValue*> numbers=prop->getNumbers();
    QJsonArray jsonarray;
    QJsonObject jsonobj;
    for (int i = 0; i < numbers.size(); ++i) {
        numbers[i]->setValue(numbers[i]->getValue()+2);
        jsonobj["name"]=numbers[i]->name();
        jsonobj["label"]=numbers[i]->label();
        jsonobj["value"]=numbers[i]->getValue();
        jsonarray.append(jsonobj);

    }
    setProperty("json",jsonarray);
    prop->setState(1);
    emit propertyCreated(prop,&_modulename);
    _propertyStore.add(prop);

}
void Dummy::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;

    SwitchProperty* temp = _propertyStore.getSwitch(prop->getDeviceName(),prop->getGroupName(),prop->getName());
    QString name =prop->getSwitches()[0]->name(); /* only 1 switch is sent */
    temp->setSwitch(name,true);
    temp->setState(1);
    emit propertyCreated(temp,&_modulename);
    _propertyStore.add(temp);

}

