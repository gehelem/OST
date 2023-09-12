#ifndef OST_LOVVISITOR_H
#define OST_LOVVISITOR_H
#include <QtCore>
namespace  OST
{

class LovInt;
class LovFloat;
class LovString;

class LovVisitor
{

    protected:
        LovVisitor() = default;

    public:
        virtual void visit(LovInt* pLov) = 0;
        virtual void visit(LovFloat* pLov) = 0;
        virtual void visit(LovString* pLov) = 0;
};

}
#endif //OST_LOVVISITOR_H
