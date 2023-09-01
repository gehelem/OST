#ifndef OST_PROPERTYFACTORY_H
#define OST_PROPERTYFACTORY_H

#include <baseclient.h>
#include <propertymulti.h>
#include <valuefactory.h>

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
        static PropertyMulti* createProperty(const QString &pKey, const QVariantMap &pData);
};

}
#endif //OST_PROPERTYFACTORY_H
