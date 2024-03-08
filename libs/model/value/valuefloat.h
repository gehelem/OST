#ifndef VALUEFLOAT_h
#define VALUEFLOAT_h

#include <valuesingle.h>
#include <lovfloat.h>

namespace  OST
{

class ValueFloat: public ValueSingleNumeric<double>
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

class GridFloat: public GridTemplate<double>
{
    public:

        void accept(GridVisitor* pVisitor, QVariant &data)  override
        {
            pVisitor->visit(this, data);
        }

        QString getType() override
        {
            return "float";
        }
};


}
#endif
