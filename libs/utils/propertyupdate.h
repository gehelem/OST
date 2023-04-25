#ifndef OST_PROPERTYUPDATE_H
#define OST_PROPERTYUPDATE_H
#include <QtCore>
#include <string>
#include <propertybase.h>
#include <propertysimple.h>
#include <propertymulti.h>
#include <propertyvisitor.h>
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
        void visit(PropertySimple* pProperty) override
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
        void visit(PropertySimple *pProperty, QVariantMap &data ) override;
        void visit(PropertyMulti *pProperty, QVariantMap &data ) override;


};
}
#endif //OST_PROPERTYUPDATE_H
