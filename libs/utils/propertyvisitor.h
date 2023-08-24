#ifndef OST_PROPERTYVISITOR_H
#define OST_PROPERTYVISITOR_H
#include <QtCore>
namespace  OST
{

class PropertyBase;
class PropertyMulti;


class PropertyVisitor
{

    protected:
        PropertyVisitor() = default;

    public:
        virtual void visit(PropertyBase* pProperty) = 0;
        virtual void visit(PropertyBase* pProperty, QVariantMap &data ) = 0;
        virtual void visit(PropertyMulti* pProperty) = 0;
        virtual void visit(PropertyMulti* pProperty, QVariantMap &data ) = 0;

};

}
#endif //OST_PROPERTYVISITOR_H
