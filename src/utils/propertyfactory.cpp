#include "propertyfactory.h"
#include "model/numberproperty.h"
#include "model/switchproperty.h"
#include "model/textproperty.h"
#include "model/lightproperty.h"
#include "model/lightvalue.h"

Property* PropertyFactory::createProperty(INumberVectorProperty *pVector) {

    auto* pProperty = new NumberProperty(
            "indipanel",
            pVector->device,
            pVector->group,
            pVector->name,
            pVector->label,
            pVector->p,
            pVector->s);

    for (int i = 0; i < pVector->nnp; ++i) {

        INumber currentNumber = pVector->np[i];

        pProperty->addNumber(new NumberValue(
                currentNumber.name,
                currentNumber.label,
                "",
                currentNumber.value,
                currentNumber.format,
                currentNumber.min,
                currentNumber.max,
                currentNumber.step));
    }

    return pProperty;
}

Property *PropertyFactory::createProperty(ISwitchVectorProperty *pVector) {

    auto *pProperty = new SwitchProperty(
            "indipanel",
            pVector->device,
            pVector->group,
            pVector->name,
            pVector->label,
            pVector->p,
            pVector->s,
            pVector->r
    );

    for (int i = 0; i < pVector->nsp; ++i) {

        ISwitch currentValue = pVector->sp[i];

        pProperty->addSwitch(new SwitchValue(

                currentValue.name,
                currentValue.label,
                "",
                currentValue.s
        ));
    }

    return pProperty;
}

Property *PropertyFactory::createProperty(ITextVectorProperty *pVector) {

    auto* pProperty = new TextProperty(
            "indipanel",
            pVector->device,
            pVector->group,
            pVector->name,
            pVector->label,
            pVector->p,
            pVector->s);

    for (int i = 0; i < pVector->ntp; ++i) {

        IText currentText = pVector->tp[i];

        pProperty->addText(new TextValue(
                currentText.name,
                currentText.label,
                "",
                currentText.text));
    }

    return pProperty;
}

Property *PropertyFactory::createProperty(ILightVectorProperty *pVector) {

    auto* pProperty = new LightProperty (
           "indipanel",
            pVector->device,
            pVector->group,
            pVector->name,
            pVector->label,
            /*
             * INDI light properties have no permission flag and are considered
             * as ReadOnly: hence 0 */
            0,
            pVector->s);

    for (int i = 0; i < pVector->nlp; ++i) {

        ILight currentLight = pVector->lp[i];

        pProperty->addLight(new LightValue (
                currentLight.name,
                currentLight.label,
                "",
                currentLight.s));
    }

    return pProperty;
}
