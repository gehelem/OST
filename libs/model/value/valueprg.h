#ifndef VALUEPRG_h
#define VALUEPRG_h

#include <valuesingle.h>

namespace  OST
{

class ValuePrg: public ValueSingleNotNumeric<double>
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
        void setValue(const double &value, const bool &emitEvent)
        {
            if ((value < 0) || (value > 100))
            {
                sendMessage(OST::MsgLevel::Warn, "Invalid spinner value (" + QString::number(value) + ")");
                return;
            }
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
