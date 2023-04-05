#ifndef OST_PROPERTYUPDATE_H
#define OST_PROPERTYUPDATE_H

#include <string>
#include <numberproperty.h>
#include <basicproperty.h>
#include <propertyvisitor.h>

class Property;

class PropertyUpdate : public PropertyVisitor
{

    public:
        PropertyUpdate() = default;
        void visit(RootProperty* pProperty) override {}
        void visit(BasicProperty* pProperty) override {}
        void visit(NumberProperty* pProperty) override {}
        void visit(TextProperty* pProperty) override {}
        void visit(RootProperty *pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty)
            Q_UNUSED(data)
        }
        void visit(BasicProperty *pProperty, QVariantMap &data ) override;
        void visit(NumberProperty *pProperty, QVariantMap &data ) override;
        void visit(TextProperty *pProperty, QVariantMap &data ) override;


};

#endif //OST_PROPERTYUPDATE_H
