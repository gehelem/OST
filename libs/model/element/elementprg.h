#ifndef ELEMENTPRG_h
#define ELEMENTPRG_h

#include <elementsingle.h>

namespace  OST
{

class ElementPrg: public ElementSingleNotNumeric<PrgData>
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

        ElementPrg(const QString &label, const QString &order, const QString &hint);
        ~ElementPrg();
        QString getType() override
        {
            return "prg";
        }
        void setPrgValue(const double &v, const bool &emitEvent)
        {
            if ((v < 0) || (v > 100))
            {
                sendMessage(OST::MsgLevel::Warn, "Invalid spinner value (" + QString::number(v) + ")");
                return;
            }
            PrgData d = value();
            d.value = v;
            ElementSingleNotNumeric<PrgData>::setValue(d, emitEvent);
        }
        void setDynLabel(const QString &s, const bool &emitEvent)
        {
            PrgData d = value();
            d.dynlabel = s;
            ElementSingleNotNumeric<PrgData>::setValue(d, emitEvent);
        }
        QString dynLabel()
        {
            return mDynLabel;
        }
        PrgType prgType()
        {
            return mType;
        }
        void setPrgType(PrgType t)
        {
            mType = t;
        }

    private:
        PrgType mType = spinner;
        QString mDynLabel = "" ;

};

class ValuePrg: public ValueTemplate<PrgData>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValuePrg(ElementBase* element): ValueTemplate<PrgData>(element) {}
        ~ValuePrg() {}
        void updateValue() override
        {
            value = static_cast<ElementPrg*>(pElement)->value();
        }
        void updateElement(const bool &emitEvent) override
        {
            static_cast<ElementPrg*>(pElement)->setValue(value, emitEvent);
        }

};

}
#endif
