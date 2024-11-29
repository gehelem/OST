#ifndef ELEMENTMESSAGE_h
#define ELEMENTMESSAGE_h

#include <elementtemplate.h>

namespace  OST
{

class ElementMessage: public ElementTemplateNotNumeric<MsgData>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementMessage(const QString &label, const QString &order, const QString &hint);
        ~ElementMessage();

        QString getType() override;
        QString message();
        MsgLevel level();
        QDateTime ts();

};

class ValueMessage: public ValueTemplate<MsgData>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override;
        ValueMessage(ElementBase* element): ValueTemplate<MsgData>(element) {}
        ~ValueMessage() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};
}
#endif
