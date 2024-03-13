#ifndef ELEMENTPRG_h
#define ELEMENTPRG_h

#include <elementsingle.h>

namespace  OST
{

class ElementPrg: public ElementSingleNotNumeric<double>
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
        void setValue(const double &value, const bool &emitEvent)
        {
            if ((value < 0) || (value > 100))
            {
                sendMessage(OST::MsgLevel::Warn, "Invalid spinner value (" + QString::number(value) + ")");
                return;
            }
            mValue = value;
            if (emitEvent) emit valueSet(this);
        }
        QString dynLabel()
        {
            return mDynLabel;
        }
        void setDynLabel(const QString &value, const bool &emitEvent)
        {
            mDynLabel = value;
            if (emitEvent) emit eltChanged(this);
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
        double mValue = 0;
        PrgType mType = spinner;
        QString mDynLabel = "" ;

};

class GridPrg: public GridTemplate<double>
{

        Q_OBJECT

    public:

        void accept(GridVisitor* pVisitor, QString &action, int &line)  override
        {
            pVisitor->visit(this, action, line);
        }

        GridPrg(ElementBase* value): GridTemplate<double>(value), mElement(static_cast<ElementPrg*>(value)) {}
        ~GridPrg() {}

        QString getType() override
        {
            return "prg";
        }
        ElementPrg* mElement;
};

}
#endif
