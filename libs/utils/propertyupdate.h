#ifndef OST_PROPERTYUPDATE_H
#define OST_PROPERTYUPDATE_H
#include <string>
#include <propertymulti.h>
namespace  OST
{
class Property;

class PropertyUpdate : public PropertyVisitor
{

    public:
        PropertyUpdate() = default;
        void visit(PropertyBase* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(PropertyMulti* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(PropertyBase *pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty)
            Q_UNUSED(data)
        }
        void visit(PropertyMulti *pProperty, QVariantMap &data ) override;


};
}
#endif //OST_PROPERTYUPDATE_H
