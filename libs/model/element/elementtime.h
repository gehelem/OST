#ifndef ELEMENTTIME_h
#define ELEMENTTIME_h

#include <elementtemplate.h>

namespace  OST
{

class ElementTime: public ElementTemplateNotNumeric<TimeData>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementTime(const QString &label, const QString &order, const QString &hint);
        ~ElementTime();
        QString getType() override;

    private:

};

class ValueTime: public ValueTemplate<TimeData>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override;
        ValueTime(ElementBase* element): ValueTemplate<TimeData>(element) {}
        ~ValueTime() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};

}
#endif
