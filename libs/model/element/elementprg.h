#ifndef ELEMENTPRG_h
#define ELEMENTPRG_h

#include <elementtemplate.h>

namespace  OST
{

class ElementPrg: public ElementTemplateNotNumeric<PrgData>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementPrg(const QString &label, const QString &order, const QString &hint);
        ~ElementPrg();
        QString getType() override;
        void setPrgValue(const double &v, const bool &emitEvent);
        void setDynLabel(const QString &s, const bool &emitEvent);
        QString dynLabel();
        PrgType prgType();
        void setPrgType(PrgType t);

    private:
        PrgType mType = spinner;
        QString mDynLabel = "" ;

};

class ValuePrg: public ValueTemplate<PrgData>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override;
        ValuePrg(ElementBase* element): ValueTemplate<PrgData>(element) {}
        ~ValuePrg() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};

}
#endif
