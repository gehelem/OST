#ifndef ELEMENTBOOL_h
#define ELEMENTBOOL_h

#include <elementsingle.h>

namespace  OST
{

class ElementBool: public ElementSingleNotNumeric<bool>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override
        {
            pVisitor->visit(this, action, data);
        }

        ElementBool(const QString &label, const QString &order, const QString &hint);
        ~ElementBool();
        QString getType() override
        {
            return "bool";
        }

    private:

};

class GridBool: public GridTemplate<bool>
{
        Q_OBJECT

    public:

        void accept(GridVisitor* pVisitor, QString &action, int &line)  override
        {
            pVisitor->visit(this, action, line);
        }
        GridBool(ElementBase* value): GridTemplate<bool>(value), mElement(static_cast<ElementBool*>(value)) {}
        ~GridBool() {}

        QString getType() override
        {
            return "bool";
        }
        ElementBool* mElement;
};

}
#endif
