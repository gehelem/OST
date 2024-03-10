#ifndef ELEMENTLIGHT_h
#define ELEMENTLIGHT_h

#include <elementsingle.h>

namespace  OST
{

class ElementLight: public ElementSingleNotNumeric<State>
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

        ElementLight(const QString &label, const QString &order, const QString &hint);
        ~ElementLight();
        QString getType() override
        {
            return "light";
        }

    private:

};

class GridLight: public GridTemplate<State>
{

        Q_OBJECT

    public:

        void accept(GridVisitor* pVisitor, QString &action, int &line)  override
        {
            pVisitor->visit(this, action, line);
        }

        GridLight(ElementBase* value): GridTemplate<State>(value), mElement(static_cast<ElementLight*>(value)) {}
        ~GridLight() {}

        QString getType() override
        {
            return "light";
        }
        ElementLight* mElement;
};

}
#endif
