#ifndef OST_VALUEVISITOR_H
#define OST_VALUEVISITOR_H
#include <QtCore>
namespace  OST
{

class ValueInt;
class ValueBool;
class ValueFloat;
class ValueString;
class ValueLight;

class ValueVisitor
{

    protected:
        ValueVisitor() = default;

    public:
        virtual void visit(ValueInt* pValue) = 0;
        virtual void visit(ValueInt* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueBool* pValue) = 0;
        virtual void visit(ValueBool* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueFloat* pValue) = 0;
        virtual void visit(ValueFloat* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueString* pValue) = 0;
        virtual void visit(ValueString* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueLight* pValue) = 0;
        virtual void visit(ValueLight* pValue, QVariantMap &data) = 0;
};

}
#endif //OST_VALUEVISITOR_H
