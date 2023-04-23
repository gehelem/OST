#ifndef OST_PROPERTYVISITOR_H
#define OST_PROPERTYVISITOR_H
#include <QtCore>
namespace  OST
{

class PropertyBase;
class PropertySimple;
class PropertyMulti;

class RootProperty;
class BasicProperty;
class NumberProperty;
class TextProperty;
class MultiProperty;

class PropertyVisitor
{

    protected:
        PropertyVisitor() = default;

    public:
        virtual void visit(PropertyBase* pProperty) = 0;
        virtual void visit(PropertyBase* pProperty, QVariantMap &data ) = 0;
        virtual void visit(PropertySimple* pProperty) = 0;
        virtual void visit(PropertySimple* pProperty, QVariantMap &data ) = 0;
        virtual void visit(PropertyMulti* pProperty) = 0;
        virtual void visit(PropertyMulti* pProperty, QVariantMap &data ) = 0;

        virtual void visit(RootProperty* pProperty) = 0;
        virtual void visit(BasicProperty* pProperty) = 0;
        virtual void visit(NumberProperty* pProperty) = 0;
        virtual void visit(TextProperty* pProperty) = 0;
        virtual void visit(MultiProperty* pProperty) = 0;
        virtual void visit(RootProperty* pProperty, QVariantMap &data ) = 0;
        virtual void visit(BasicProperty* pProperty, QVariantMap &data ) = 0;
        virtual void visit(NumberProperty* pProperty, QVariantMap &data ) = 0;
        virtual void visit(TextProperty* pProperty, QVariantMap &data ) = 0;
};

}
#endif //OST_PROPERTYVISITOR_H
