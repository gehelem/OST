#ifndef VALUEFLOAT_h
#define VALUEFLOAT_h

#include <valuesingle.h>

namespace  OST
{

class ValueFloat: public ValueSingle<double>
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

        ValueFloat(const QString &label, const QString &order, const QString &hint);
        ~ValueFloat();
        QString getType() override
        {
            return "float";
        }
        double min()
        {
            return mMin;
        }
        void setMin(const double &min);
        double max()
        {
            return mMax;
        }
        void setMax(const double &max);
        double step()
        {
            return mStep;
        }
        void setStep(const double &step);
        QString format()
        {
            return mFormat;
        }
        void setFormat(const QString &format);

    private:
        double mMin = 0;
        double mMax = 0;
        double mStep = 0;
        QString mFormat = "";

};

}
#endif
