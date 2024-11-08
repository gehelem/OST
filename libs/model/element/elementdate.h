#ifndef ELEMENTDATE_h
#define ELEMENTDATE_h

#include <elementtemplate.h>

namespace  OST
{

class ElementDate: public ElementTemplateNotNumeric<DateData>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementDate(const QString &label, const QString &order, const QString &hint);
        ~ElementDate();
        QString getType() override;

    private:

};

class ValueDate: public ValueTemplate<DateData>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override;
        ValueDate(ElementBase* element): ValueTemplate<DateData>(element) {}
        ~ValueDate() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};

}
#endif
