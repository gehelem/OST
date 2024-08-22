#ifndef ELEMENTBOOL_h
#define ELEMENTBOOL_h

#include <elementtemplate.h>

namespace  OST
{

class ElementBool: public ElementTemplateNotNumeric<bool>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementBool(const QString &label, const QString &order, const QString &hint);
        ~ElementBool();
        QString getType() override;

};

class ValueBool: public ValueTemplate<bool>
{
        Q_OBJECT

    public:
        void accept(ValueVisitor* pVisitor)  override;
        ValueBool(ElementBase* element): ValueTemplate<bool>(element) {}
        ~ValueBool() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};

}
#endif
