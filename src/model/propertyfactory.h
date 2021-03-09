//
// Created by deufrai on 09/03/2021.
//

#ifndef OST_PROPERTYFACTORY_H
#define OST_PROPERTYFACTORY_H

#include <baseclient.h>
#include "property.h"

class PropertyFactory {

public:
    PropertyFactory() = delete;
    static Property* createProperty(INumberVectorProperty* pVector);
};


#endif //OST_PROPERTYFACTORY_H
