#ifndef OST_PROPERTYFACTORY_H
#define OST_PROPERTYFACTORY_H

#include <baseclient.h>
#include "model/property.h"

class PropertyFactory {

public:
    PropertyFactory() = delete;
    static Property* createProperty(INumberVectorProperty* pVector);
    static Property* createProperty(ISwitchVectorProperty * pVector);
    static Property* createProperty(ITextVectorProperty* pVector);
    static Property* createProperty(ILightVectorProperty* pVector);
    static Property* createProperty(QString name,QString label);
};


#endif //OST_PROPERTYFACTORY_H
