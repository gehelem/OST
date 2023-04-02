#ifndef OST_PROPERTYVISITOR_H
#define OST_PROPERTYVISITOR_H

class BasicProperty;
class NumberProperty;

class PropertyVisitor
{

    protected:
        PropertyVisitor() = default;

    public:
        virtual void visit(NumberProperty* pProperty) = 0;
        virtual void visit(BasicProperty* pProperty) = 0;
};

#endif //OST_PROPERTYVISITOR_H
