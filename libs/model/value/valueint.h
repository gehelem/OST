#ifndef VALUEINT_h
#define VALUEINT_h

#include <valuebase.h>

namespace  OST
{

class ValueInt: public ValueBase
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

        ValueInt(const QString &label, const QString &order, const QString &hint);
        ~ValueInt();
        long value()
        {
            return mValue;
        }
        void setValue(const long &value);
        long min()
        {
            return mMin;
        }
        void setMin(const long &min);
        long max()
        {
            return mMax;
        }
        void setMax(const long &max);
        long step()
        {
            return mStep;
        }
        void setStep(const long &step);
        QString format()
        {
            return mFormat;
        }
        void setFormat(const QString &format);

    private:
        long mValue = 0;
        long mMin = 0;
        long mMax = 0;
        long mStep = 0;
        QString mFormat = "";

};

}
#endif
