#ifndef ELEMENTINT_h
#define ELEMENTINT_h

#include <elementtemplate.h>
#include <lovint.h>

namespace  OST
{

class ElementInt: public ElementTemplateNumeric<int>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementInt(const QString &label, const QString &order, const QString &hint);
        ~ElementInt();

        QString getType() override;
        QMap<int, QString> getLov();
        bool lovAdd(int val, QString label);
        bool lovUpdate(int  val, QString label);
        bool lovDel(int  val);
        bool lovClear();
    private:
        LovInt mLov;
};

class ValueInt: public ValueTemplate<int>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override;
        ValueInt(ElementBase* element): ValueTemplate<int>(element) {}
        ~ValueInt() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};

}

#endif
