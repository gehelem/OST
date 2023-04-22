#ifndef OST_VALUEVISITOR_H
#define OST_VALUEVISITOR_H
#include <QtCore>

class RootProperty;
class BasicProperty;
class NumberProperty;
class TextProperty;
class MultiProperty;

class ValueVisitor
{

    protected:
        ValueVisitor() = default;

    public:
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

#endif //OST_VALUEVISITOR_H
