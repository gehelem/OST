#ifndef OST_PROPERTYFACTORY_H
#define OST_PROPERTYFACTORY_H

#include <baseclient.h>
#include "model/property.h"

class PropertyFactory {

public:
    PropertyFactory() = delete;
    static Property* createProperty(INumberVectorProperty* pVector,QString* moduleName );
    static Property* createProperty(ISwitchVectorProperty * pVector,QString* moduleName );
    static Property* createProperty(ITextVectorProperty* pVector,QString* moduleName );
    static Property* createProperty(ILightVectorProperty* pVector,QString* moduleName );
};


#endif //OST_PROPERTYFACTORY_H
