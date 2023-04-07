#include "propertyfactory.h"
#include "model/multiproperty.h"
#include "model/numberproperty.h"
//#include "model/switchproperty.h"
#include "model/textproperty.h"
//#include "model/lightproperty.h"
//#include "model/lightvalue.h"

RootProperty* PropertyFactory::createProperty(const QVariantMap &pData)
{

    if(!pData.contains("type"))
    {
        if (pData.contains("value"))
        {
            if (strcmp(pData["value"].typeName(), "QString") == 0)
            {
                auto *pProperty = new TextProperty(pData["propertyLabel"].toString(),
                                                   pData["devcat"].toString(),
                                                   pData["group"].toString(),
                                                   TextProperty::ReadWrite);
                pProperty->setValue(pData["value"].toString());
                return pProperty;

            }


            if (
                (strcmp(pData["value"].typeName(), "double") == 0) ||
                (strcmp(pData["value"].typeName(), "float") == 0) ||
                (strcmp(pData["value"].typeName(), "qlonglong") == 0) ||
                (strcmp(pData["value"].typeName(), "int") == 0)
            )

            {
                auto *pProperty = new NumberProperty(pData["propertyLabel"].toString(),
                                                     pData["devcat"].toString(),
                                                     pData["group"].toString(),
                                                     TextProperty::ReadWrite);
                pProperty->setValue(pData["value"].toDouble());
                return pProperty;

            }

        }

    }
    auto *pProperty = new MultiProperty(pData["propertyLabel"].toString(),
                                        pData["devcat"].toString(),
                                        pData["group"].toString(),
                                        TextProperty::ReadWrite);
    if (pData.contains("elements"))
    {
        QVariantMap elts = pData["elements"].toMap();
        foreach(const QString &key, elts.keys())
        {
            QVariantMap elt = elts[key].toMap();
            RootProperty *rp = PropertyFactory::createProperty(elt);
            if (rp != nullptr)
            {
                pProperty->addElt(key, rp);
            }
        }

    }








    return pProperty;

    /*auto* pProperty = new BasicProperty(pData["propertyLabel"].toString(), pData["devcat"].toString(),
                                        pData["group"].toString(), BasicProperty::Permission::ReadOnly);
    return pProperty;*/
    return nullptr;



}


/*RootProperty* PropertyFactory::createProperty(INumberVectorProperty *pVector) {

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


/*RootProperty* PropertyFactory::createProperty(INumberVectorProperty *pVector) {

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

RootProperty *PropertyFactory::createProperty(ISwitchVectorProperty *pVector) {

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

RootProperty *PropertyFactory::createProperty(ITextVectorProperty *pVector) {

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

RootProperty *PropertyFactory::createProperty(ILightVectorProperty *pVector) {

    auto* pProperty = new LightProperty (
           "indipanel",
            pVector->device,
            pVector->group,
            pVector->name,
            pVector->label,
            //
            // INDI light properties have no permission flag and are considered
            // as ReadOnly: hence 0
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
*/
