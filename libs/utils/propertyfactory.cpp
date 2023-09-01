#include "propertyfactory.h"
namespace  OST
{

PropertyMulti *PropertyFactory::createProperty(const QString &pKey, const QVariantMap &pData)
{
    //qDebug() << "PropertyFactory::createProperty - " << pData;
    auto *pProperty = new PropertyMulti(pKey,
                                        pData["label"].toString(),
                                        OST::IntToPermission(pData["permission"].toInt()),
                                        pData["devcat"].toString(),
                                        pData["group"].toString(),
                                        pData["order"].toString(),
                                        pData["hasprofile"].toBool(),
                                        pData.contains("grid")
                                       );

    if (!pData.contains("elements"))
    {
        qDebug() << "Multiproperty defined without elements " << pData;
        return pProperty;
    }
    QVariantMap elts = pData["elements"].toMap();
    foreach(const QString &key, elts.keys())
    {
        QVariantMap elt = elts[key].toMap();
        ValueBase *v = ValueFactory::createValue(elt);
        if (v != nullptr)
        {
            pProperty->addValue(key, v);
        }
    }
    return pProperty;


    qDebug() << "Can't create property " << pData;
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
}
