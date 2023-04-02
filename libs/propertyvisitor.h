#ifndef OST_PROPERTYVISITOR_H
#define OST_PROPERTYVISITOR_H

class RootProperty;
class BasicProperty;
class NumberProperty;
class TextProperty;

class PropertyVisitor
{

    protected:
        PropertyVisitor() = default;

    public:
        virtual void visit(RootProperty* pProperty) = 0;
        virtual void visit(BasicProperty* pProperty) = 0;
        virtual void visit(NumberProperty* pProperty) = 0;
        virtual void visit(TextProperty* pProperty) = 0;
};

#endif //OST_PROPERTYVISITOR_H
