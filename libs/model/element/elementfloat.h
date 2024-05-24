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

        ElementFloat(const QString &label, const QString &order, const QString &hint);
        ~ElementFloat();
        QString getType() override
        {
            return "float";
        }
        QMap<double, QString> getLov()
        {
            return mLov.getLov();
        }
        bool lovAdd(double val, QString label)
        {
            return mLov.lovAdd(val, label);
        }
        bool lovUpdate(double  val, QString label)
        {
            return mLov.lovUpdate(val, label);
        }
        bool lovDel(double  val)
        {
            return mLov.lovDel(val);
        }
        bool lovClear()
        {
            return mLov.lovClear();
        }

    private:
        LovFloat mLov;

};

class ValueFloat: public ValueTemplate<double>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValueFloat(ElementBase* element): ValueTemplate<double>(element) {}
        ~ValueFloat() {}
        void updateValue() override
        {
            value = static_cast<ElementFloat*>(pElement)->value();
        }
        void updateElement(const bool &emitEvent) override
        {
            static_cast<ElementFloat*>(pElement)->setValue(value, emitEvent);
        }

};

}
#endif
