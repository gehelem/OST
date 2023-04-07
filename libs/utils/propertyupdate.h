#ifndef OST_PROPERTYUPDATE_H
#define OST_PROPERTYUPDATE_H
#include <QtCore>
#include <string>
#include <numberproperty.h>
#include <basicproperty.h>
#include <propertyvisitor.h>

class Property;

class PropertyUpdate : public PropertyVisitor
{

    public:
        PropertyUpdate() = default;
        void visit(RootProperty* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(BasicProperty* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(NumberProperty* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(TextProperty* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(MultiProperty* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
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
