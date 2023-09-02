#ifndef VALUEFLOAT_h
#define VALUEFLOAT_h

#include <valuebase.h>

namespace  OST
{

class ValueFloat: public ValueBase
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

        ValueFloat(const QString &label, const QString &order, const QString &hint);
        ~ValueFloat();
        double value()
        {
            return mValue;
        }
        void setValue(const double &value);
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

        ListOfValues<double> lov;
        Grid<double> grid;

    private:
        double mValue = 0;
        double mMin = 0;
        double mMax = 0;
        double mStep = 0;
        QString mFormat = "";

};

}
#endif
