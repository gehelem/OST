#ifndef OST_PROPERTYVISITOR_H
#define OST_PROPERTYVISITOR_H
#include <QString>
class NumberProperty;
class TextProperty;
class SwitchProperty;
class LightProperty;
class MessageProperty;
class ImageProperty;
class GridProperty;

class PropertyVisitor {

protected:
    PropertyVisitor() = default;

public:
    virtual void visit(NumberProperty* pProperty, QString* moduleName) = 0;
    virtual void visit(SwitchProperty* pProperty, QString* moduleName) = 0;
    virtual void visit(TextProperty* pProperty, QString* moduleName) = 0;
    virtual void visit(LightProperty* pProperty, QString* moduleName) = 0;
    virtual void visit(MessageProperty* pProperty, QString* moduleName) = 0;
    virtual void visit(ImageProperty* pProperty, QString* moduleName) = 0;
    virtual void visit(GridProperty* pProperty, QString* moduleName) = 0;
    virtual void visit(GridProperty* pProperty, QString* moduleName,double s,double x,double y,double z) = 0;

};


#endif //OST_PROPERTYVISITOR_H
