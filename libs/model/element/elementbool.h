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

class ValueBool: public ValueTemplate<bool>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValueBool(ElementBase* element): ValueTemplate<bool>(element) {}
        ~ValueBool() {}
        void updateValue() override
        {
            value = static_cast<ElementBool*>(pElement)->value();
        }
        void updateElement(const bool &emitEvent) override
        {
            static_cast<ElementBool*>(pElement)->setValue(value, emitEvent);
        }

};

}
#endif
