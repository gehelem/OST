#ifndef ELEMENTTIME_h
#define ELEMENTTIME_h

#include <elementtemplate.h>

namespace  OST
{

class ElementTime: public ElementTemplateNotNumeric<QTime>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementTime(const QString &label, const QString &order, const QString &hint);
        ~ElementTime();
        QString getType() override;
        void setUseMs(bool b);
        bool getUseMs(void);

    private:
        bool mUseMs = false;

};

class ValueTime: public ValueTemplate<QTime>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override;
        ValueTime(ElementBase* element): ValueTemplate<QTime>(element) {}
        ~ValueTime() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};

}
#endif
