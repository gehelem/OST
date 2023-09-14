#ifndef VALUEINT_h
#define VALUEINT_h

#include <valuesingle.h>
#include <lovint.h>

namespace  OST
{

class ValueInt: public ValueSingle<int>
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

        ValueInt(const QString &label, const QString &order, const QString &hint);
        ~ValueInt();
        QString getType() override
        {
            return "int";
        }
        int min()
        {
            return mMin;
        }
        void setMin(const int &min);
        int max()
        {
            return mMax;
        }
        void setMax(const int &max);
        int step()
        {
            return mStep;
        }
        void setStep(const int &step);
        QString format()
        {
            return mFormat;
        }
        void setFormat(const QString &format);
        QMap<int, QString> getLov()
        {
            return mLov.getLov();
        }
        bool lovAdd(int val, QString label)
        {
            return mLov.lovAdd(val, label);
        }
        bool lovUpdate(int  val, QString label)
        {
            return mLov.lovUpdate(val, label);
        }
        bool lovDel(int  val)
        {
            return mLov.lovDel(val);
        }
        bool lovClear()
        {
            return mLov.lovClear();
        }
    private:
        int mMin = 0;
        int mMax = 0;
        int mStep = 0;
        QString mFormat = "";
        LovInt mLov;
};

}
#endif
