#ifndef OST_PROPERTYFACTORY_H
#define OST_PROPERTYFACTORY_H

#include <baseclient.h>
#include <model/rootproperty.h>
namespace  OST
{

class PropertyFactory
{

    public:
        PropertyFactory() = delete;
        //static RootProperty* createProperty(INumberVectorProperty* pVector);
        //static RootProperty* createProperty(ISwitchVectorProperty * pVector);
        //static RootProperty* createProperty(ITextVectorProperty* pVector);
        //static RootProperty* createProperty(ILightVectorProperty* pVector);
        static RootProperty* createProperty(const QVariantMap &pData);
};

}
#endif //OST_PROPERTYFACTORY_H
