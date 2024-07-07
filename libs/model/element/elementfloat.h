#ifndef ELEMENTFLOAT_h
#define ELEMENTFLOAT_h

#include <elementtemplate.h>
#include <lovfloat.h>

namespace  OST
{

class ElementFloat: public ElementTemplateNumeric<double>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementFloat(const QString &label, const QString &order, const QString &hint);
        ~ElementFloat();
        QString getType() override;
        QMap<double, QString> getLov();
        bool lovAdd(double val, QString label);
        bool lovUpdate(double  val, QString label);
        bool lovDel(double  val);
        bool lovClear();

    private:
        LovFloat mLov;

};

class ValueFloat: public ValueTemplate<double>
{
        Q_OBJECT

    public:
        void accept(ValueVisitor* pVisitor)  override;
        ValueFloat(ElementBase* element): ValueTemplate<double>(element) {}
        ~ValueFloat() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};

}
#endif
