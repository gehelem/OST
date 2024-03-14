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

class ValueLight: public ValueTemplate<State>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValueLight(ElementBase* element): ValueTemplate<State>(element) {}
        ~ValueLight() {}
        void updateValue() override
        {
            value = static_cast<ElementLight*>(pElement)->value();
        }
        void updateElement(const bool &emitEvent) override
        {
            static_cast<ElementLight*>(pElement)->setValue(value, emitEvent);
        }

};

}
#endif
