#ifndef VALUEPRG_h
#define VALUEPRG_h

#include <valuebase.h>

namespace  OST
{

class ValuePrg: public ValueBase
{

        Q_OBJECT

    public:
        void accept(ValueVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(ValueVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }
        void accept(ValueVisitor *pVisitor, QString &action, QVariantMap &data) override
        {
            pVisitor->visit(this, action, data);
        }

        ValuePrg(const QString &label, const QString &order, const QString &hint);
        ~ValuePrg();
        QString getType() override
        {
            return "prg";
        }
        double value()
        {
            return mValue;
        }
        void setValue(const double &value, const bool &emitEvent)
        {
            mValue = value;
            if (emitEvent) emit valueChanged(this);
        }
        QString dynLabel()
        {
            return mDynLabel;
        }
        void setDynLabel(const QString &value, const bool &emitEvent)
        {
            mDynLabel = value;
            if (emitEvent) emit valueChanged(this);
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

}
#endif
