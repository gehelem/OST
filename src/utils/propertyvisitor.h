//
// Created by deufrai on 09/03/2021.
//

#ifndef OST_PROPERTYVISITOR_H
#define OST_PROPERTYVISITOR_H

class NumberProperty;
class TextProperty;
class SwitchProperty;
class LightProperty;

class PropertyVisitor {

protected:
    PropertyVisitor() = default;

public:
    virtual void visit(NumberProperty* pProperty) = 0;
    virtual void visit(SwitchProperty* pProperty) = 0;
    virtual void visit(TextProperty* pProperty) = 0;
    virtual void visit(LightProperty* pProperty) = 0;
};

#endif //OST_PROPERTYVISITOR_H
