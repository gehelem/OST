#ifndef ELEMENTLIGHT_h
#define ELEMENTLIGHT_h

#include <elementtemplate.h>

namespace  OST
{

class ElementLight: public ElementTemplateNotNumeric<State>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementLight(const QString &label, const QString &order, const QString &hint);
        ~ElementLight();
        QString getType() override;

    private:

};

class ValueLight: public ValueTemplate<State>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override;
        ValueLight(ElementBase* element): ValueTemplate<State>(element) {}
        ~ValueLight() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};

}
#endif
