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
class ValueImg;
class ValueVideo;
class ValueMessage;
class ValueGraph;
class ValuePrg;

class ValueVisitor
{

    protected:
        ValueVisitor() = default;

    public:
        virtual void visit(ValueInt* pValue) = 0;
        virtual void visit(ValueInt* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueInt* pValue, QString &action, QVariantMap &data) = 0;
        virtual void visit(ValueBool* pValue) = 0;
        virtual void visit(ValueBool* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueBool* pValue, QString &action, QVariantMap &data) = 0;
        virtual void visit(ValueFloat* pValue) = 0;
        virtual void visit(ValueFloat* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueFloat* pValue, QString &action, QVariantMap &data) = 0;
        virtual void visit(ValueString* pValue) = 0;
        virtual void visit(ValueString* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueString* pValue, QString &action, QVariantMap &data) = 0;
        virtual void visit(ValueLight* pValue) = 0;
        virtual void visit(ValueLight* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueLight* pValue, QString &action, QVariantMap &data) = 0;
        virtual void visit(ValueImg* pValue) = 0;
        virtual void visit(ValueImg* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueImg* pValue, QString &action, QVariantMap &data) = 0;
        virtual void visit(ValueVideo* pValue) = 0;
        virtual void visit(ValueVideo* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueVideo* pValue, QString &action, QVariantMap &data) = 0;
        virtual void visit(ValueMessage* pValue) = 0;
        virtual void visit(ValueMessage* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueMessage* pValue, QString &action, QVariantMap &data) = 0;
        virtual void visit(ValueGraph* pValue) = 0;
        virtual void visit(ValueGraph* pValue, QVariantMap &data) = 0;
        virtual void visit(ValueGraph* pValue, QString &action, QVariantMap &data) = 0;
        virtual void visit(ValuePrg* pValue) = 0;
        virtual void visit(ValuePrg* pValue, QVariantMap &data) = 0;
        virtual void visit(ValuePrg* pValue, QString &action, QVariantMap &data) = 0;
};

class GridInt;
class GridBool;
class GridFloat;
class GridString;
class GridLight;
class GridImg;
class GridVideo;
class GridMessage;
class GridGraph;
class GridPrg;


class GridVisitor
{

    protected:
        GridVisitor() = default;

    public:
        virtual void visit(GridInt* pGrid, QString &action, int &line) = 0;
        virtual void visit(GridBool* pGrid, QString &action, int &line) = 0;
        virtual void visit(GridFloat* pGrid, QString &action, int &line) = 0;
        virtual void visit(GridString* pGrid, QString &action, int &line) = 0;
        virtual void visit(GridLight* pGrid, QString &action, int &line) = 0;
        virtual void visit(GridImg* pGrid, QString &action, int &line) = 0;
        virtual void visit(GridVideo* pGrid, QString &action, int &line) = 0;
        virtual void visit(GridMessage* pGrid, QString &action, int &line) = 0;
        virtual void visit(GridGraph* pGrid, QString &action, int &line) = 0;
        virtual void visit(GridPrg* pGrid, QString &action, int &line) = 0;
};

}
#endif //OST_VALUEVISITOR_H
