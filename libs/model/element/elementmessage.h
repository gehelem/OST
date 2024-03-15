#ifndef ELEMENTMESSAGE_h
#define ELEMENTMESSAGE_h

#include <elementsingle.h>

namespace  OST
{

class ElementMessage: public ElementSingleNotNumeric<MsgData>
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

        ElementMessage(const QString &label, const QString &order, const QString &hint);
        ~ElementMessage();
        QString getType() override
        {
            return "message";
        }
        QString message()
        {
            return value().message;
        }
        MsgLevel level()
        {
            return value().level;
        }
        QDateTime ts()
        {
            return value().ts;
        }

};

class ValueMessage: public ValueTemplate<MsgData>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValueMessage(ElementBase* element): ValueTemplate<MsgData>(element) {}
        ~ValueMessage() {}
        void updateValue() override
        {
            value = static_cast<ElementMessage*>(pElement)->value();
        }
        void updateElement(const bool &emitEvent) override
        {
            static_cast<ElementMessage*>(pElement)->setValue(value, emitEvent);
        }

};
}
#endif
