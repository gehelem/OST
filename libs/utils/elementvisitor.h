#ifndef OST_ELEMENTVISITOR_H
#define OST_ELEMENTVISITOR_H
#include <QtCore>
namespace  OST
{

class ElementInt;
class ElementBool;
class ElementFloat;
class ElementString;
class ElementLight;
class ElementImg;
class ElementVideo;
class ElementMessage;
class ElementGraph;
class ElementPrg;

class ElementVisitor
{

    protected:
        ElementVisitor() = default;

    public:
        virtual void visit(ElementInt* pElement) = 0;
        virtual void visit(ElementInt* pElement, QVariantMap &data) = 0;
        virtual void visit(ElementInt* pElement, QString &action, QVariantMap &data) = 0;
        virtual void visit(ElementBool* pElement) = 0;
        virtual void visit(ElementBool* pElement, QVariantMap &data) = 0;
        virtual void visit(ElementBool* pElement, QString &action, QVariantMap &data) = 0;
        virtual void visit(ElementFloat* pElement) = 0;
        virtual void visit(ElementFloat* pElement, QVariantMap &data) = 0;
        virtual void visit(ElementFloat* pElement, QString &action, QVariantMap &data) = 0;
        virtual void visit(ElementString* pElement) = 0;
        virtual void visit(ElementString* pElement, QVariantMap &data) = 0;
        virtual void visit(ElementString* pElement, QString &action, QVariantMap &data) = 0;
        virtual void visit(ElementLight* pElement) = 0;
        virtual void visit(ElementLight* pElement, QVariantMap &data) = 0;
        virtual void visit(ElementLight* pElement, QString &action, QVariantMap &data) = 0;
        virtual void visit(ElementImg* pElement) = 0;
        virtual void visit(ElementImg* pElement, QVariantMap &data) = 0;
        virtual void visit(ElementImg* pElement, QString &action, QVariantMap &data) = 0;
        virtual void visit(ElementVideo* pElement) = 0;
        virtual void visit(ElementVideo* pElement, QVariantMap &data) = 0;
        virtual void visit(ElementVideo* pElement, QString &action, QVariantMap &data) = 0;
        virtual void visit(ElementMessage* pElement) = 0;
        virtual void visit(ElementMessage* pElement, QVariantMap &data) = 0;
        virtual void visit(ElementMessage* pElement, QString &action, QVariantMap &data) = 0;
        virtual void visit(ElementGraph* pElement) = 0;
        virtual void visit(ElementGraph* pElement, QVariantMap &data) = 0;
        virtual void visit(ElementGraph* pElement, QString &action, QVariantMap &data) = 0;
        virtual void visit(ElementPrg* pElement) = 0;
        virtual void visit(ElementPrg* pElement, QVariantMap &data) = 0;
        virtual void visit(ElementPrg* pElement, QString &action, QVariantMap &data) = 0;
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
        virtual void visit(ValueBool* pValue) = 0;
        virtual void visit(ValueFloat* pValue) = 0;
        virtual void visit(ValueString* pValue) = 0;
        virtual void visit(ValueLight* pValue) = 0;
        virtual void visit(ValueImg*  pValue) = 0;
        virtual void visit(ValueVideo*  pValue) = 0;
        virtual void visit(ValueMessage*  pValue) = 0;
        virtual void visit(ValueGraph*  pValue) = 0;
        virtual void visit(ValuePrg* pValue) = 0;
};

}
#endif //OST_ELEMENTVISITOR_H
