#ifndef VALUEINT_h
#define VALUEINT_h

#include <valuesingle.h>
#include <lovint.h>

namespace  OST
{

class ValueInt: public ValueSingleNumeric<int>
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
        LovInt mLov;
};

class GridInt: public GridTemplate<int>
{

        Q_OBJECT

    public:

        void accept(GridVisitor* pVisitor, QString &action, int &line)  override
        {
            pVisitor->visit(this, action, line);
        }

        GridInt(ValueBase* value): GridTemplate<int>(value), mElement(static_cast<ValueInt*>(value)) {}
        ~GridInt() {}

        QString getType() override
        {
            return "int";
        }
        ValueInt* mElement;
};

}

#endif
