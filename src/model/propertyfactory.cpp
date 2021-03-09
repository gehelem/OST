//
// Created by deufrai on 09/03/2021.
//

#include "propertyfactory.h"
#include "numberproperty.h"

Property* PropertyFactory::createProperty(INumberVectorProperty *pVector) {

    auto* pPorperty = new NumberProperty(
            pVector->device,
            pVector->group,
            pVector->name,
            pVector->label,
            pVector->p,
            pVector->s);

    for (int i = 0; i < pVector->nnp; ++i) {

        INumber currentNumber = pVector->np[i];

        pPorperty->addNumber(new NumberValue(
                currentNumber.name,
                currentNumber.label,
                "",
                currentNumber.value,
                currentNumber.format,
                currentNumber.min,
                currentNumber.max,
                currentNumber.step));
    }

    return pPorperty;
}
