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



}

Dummy::~Dummy()
{

}

void Dummy::OnSetPropertyText(TextProperty* prop)
{

    if (!(prop->getModuleName()==_modulename)) return;
    QList<TextValue*> texts=prop->getTexts();

    for (int i = 0; i < texts.size(); ++i) {
        texts[i]->setText(texts[i]->text()+" modified by module");
    }
    prop->setState(1);
    emit propertyCreated(prop,&_modulename);
    _propertyStore.add(prop);

}
void Dummy::OnSetPropertyNumber(NumberProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;

    QList<NumberValue*> numbers=prop->getNumbers();

    for (int i = 0; i < numbers.size(); ++i) {
        numbers[i]->setValue(numbers[i]->getValue()+2);
    }
    prop->setState(1);
    emit propertyCreated(prop,&_modulename);
    _propertyStore.add(prop);

}
void Dummy::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;

    for (int i = 0; i < prop->getSwitches().size(); ++i) {
        BOOST_LOG_TRIVIAL(debug) << "Switch recieved " << prop->getSwitches()[i]->name().toStdString() << "-" << prop->getSwitches()[i]->switchState();
    }

    SwitchProperty* temp = _propertyStore.getSwitch(prop->getDeviceName(),prop->getGroupName(),prop->getName());

    QList<SwitchValue*> switches=temp->getSwitches();
    BOOST_LOG_TRIVIAL(debug) << "Switch rule " << temp->getRule();

    /*!< Only 1 switch of many can be ON (e.g. radio buttons) */
    if (temp->getRule()==ISR_1OFMANY) {
        for (int i = 0; i < switches.size(); ++i) {

            for (int j = 0; j < prop->getSwitches().size(); ++j) {
                if (prop->getSwitches()[j]->name()==switches[i]->name()) {
                    switches[i]->setState(true);
                } else {
                    switches[i]->setState(false);
                }
            }

        }
    }
    /*!< At most one switch can be ON, but all switches can be off. It is similar to ISR_1OFMANY with the exception that all switches can be off. */
    if (temp->getRule()==ISR_ATMOST1) {
        for (int i = 0; i < switches.size(); ++i) {
            for (int j = 0; j < prop->getSwitches().size(); ++j) {
                if (prop->getSwitches()[j]->name()==switches[i]->name()) {
                    if (switches[i]->switchState()) switches[i]->setState(false); else switches[i]->setState(true);
                } else {
                    switches[i]->setState(false);
                }
            }

        }
    }
    /*!< Any number of switches can be ON (e.g. check boxes) */
    if (temp->getRule()==ISR_NOFMANY) {
        for (int i = 0; i < switches.size(); ++i) {
            for (int j = 0; j < prop->getSwitches().size(); ++j) {
                if (prop->getSwitches()[j]->name()==switches[i]->name()) {
                    if (switches[i]->switchState()) switches[i]->setState(false); else switches[i]->setState(true);
                }
            }

        }

    }

    for (int i = 0; i < temp->getSwitches().size(); ++i) {
        BOOST_LOG_TRIVIAL(debug) << "Switch " << temp->getSwitches()[i]->name().toStdString() << "-" << temp->getSwitches()[i]->switchState();
    }
    for (int i = 0; i < switches.size(); ++i) {
        BOOST_LOG_TRIVIAL(debug) << "Switch " << switches[i]->name().toStdString() << "-" << switches[i]->switchState();
    }


    temp->setState(1);
    temp->setSwitches(switches);
    emit propertyCreated(temp,&_modulename);
    _propertyStore.add(temp);

}

