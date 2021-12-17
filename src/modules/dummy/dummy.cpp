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

    /*INDI::BaseDevice *dp = getDevice(prop->getDeviceName().toStdString().c_str());
    if (dp== nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) << "Indipanel device not found " << prop->getDeviceName().toStdString();
        return;
    }
    INDI::Property *iprop;
    iprop =  dp->getProperty(prop->getName().toStdString().c_str());
    if (iprop== nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) << "Indipanel property not found " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();
        return;
    }

    if (iprop->getType()==INDI_NUMBER) {
        INumberVectorProperty *indiprop;
        indiprop =  dp->getNumber(prop->getName().toStdString().c_str());
        if (indiprop== nullptr)
        {
            BOOST_LOG_TRIVIAL(debug) << "Indipanel number property not found " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();
            return;
        }
        QList<NumberValue*> numbers=prop->getNumbers();
        for (int i = 0; i < indiprop->nnp; ++i) {
            for (int j = 0; j < numbers.size(); ++j) {
                if (strcmp(numbers[j]->name().toStdString().c_str(),indiprop->np[i].name)==0) {
                    //strcpy(inditprop->tp[i].text,texts[j]->text().toStdString().c_str());
                    indiprop->np[i].value=numbers[j]->getValue();
                    numbers[j]->setValue(indiprop->np[i].value);
                    BOOST_LOG_TRIVIAL(debug) << "Indipanel number propertyitem  modified " << indiprop->np[i].name << "/" << indiprop->np[i].value;
                }
            }
        }
        sendNewNumber(indiprop);
        return;
    }


    return;*/
}
void Dummy::OnSetPropertySwitch(SwitchProperty* prop)
{
    if (!(prop->getModuleName()==_modulename)) return;

    /*INDI::BaseDevice *dp = getDevice(prop->getDeviceName().toStdString().c_str());
    if (dp== nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) << "Indipanel device not found " << prop->getDeviceName().toStdString();
        return;
    }
    INDI::Property *iprop;
    iprop =  dp->getProperty(prop->getName().toStdString().c_str());
    if (iprop== nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) << "Indipanel property not found " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();
        return;
    }

    if (iprop->getType()==INDI_SWITCH) {
        ISwitchVectorProperty *indiprop;
        indiprop =  dp->getSwitch(prop->getName().toStdString().c_str());
        if (indiprop== nullptr)
        {
            BOOST_LOG_TRIVIAL(debug) << "Indipanel switch property not found " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();
            return;
        }
        BOOST_LOG_TRIVIAL(debug) << "Indipanel switch property  " << prop->getDeviceName().toStdString() << " " << prop->getName().toStdString();

        QList<SwitchValue*> switchs=prop->getSwitches();
        for (int i = 0; i < indiprop->nsp; ++i) {
            for (int j = 0; j < switchs.size(); ++j) {
                if (indiprop->r==ISR_1OFMANY) {
                    indiprop->sp[i].s=ISS_OFF;
                    if (strcmp(switchs[j]->name().toStdString().c_str(),indiprop->sp[i].name)==0) {
                        BOOST_LOG_TRIVIAL(debug) << "ISR_1OFMANY";
                        indiprop->sp[i].s=ISS_ON;
                    }

                }
                if (indiprop->r==ISR_ATMOST1) {
                    if (strcmp(switchs[j]->name().toStdString().c_str(),indiprop->sp[i].name)==0) {
                        BOOST_LOG_TRIVIAL(debug) << "ISR_ATMOST1";
                        indiprop->sp[i].s=ISS_OFF;
                        if (indiprop->sp[i].s==ISS_ON ) indiprop->sp[i].s=ISS_OFF;
                        if (indiprop->sp[i].s==ISS_OFF) indiprop->sp[i].s=ISS_ON;
                    }

                }
                if (indiprop->r==ISR_NOFMANY) {
                    if (strcmp(switchs[j]->name().toStdString().c_str(),indiprop->sp[i].name)==0) {
                        BOOST_LOG_TRIVIAL(debug) << "ISR_NOFMANY";
                        if (indiprop->sp[i].s==ISS_ON ) {
                            indiprop->sp[i].s=ISS_OFF;
                        } else {
                            indiprop->sp[i].s=ISS_ON;
                        }
                    }

                }
            }
        }
        sendNewSwitch(indiprop);
        return;
    }


    return;*/
}

