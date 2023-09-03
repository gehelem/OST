#ifndef VALUEBOOL_h
#define VALUEBOOL_h

#include <valuebase.h>

namespace  OST
{

class ValueBool: public ValueBase
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

        ValueBool(const QString &label, const QString &order, const QString &hint);
        ~ValueBool();
        bool value()
        {
            return mValue;
        }
        void setValue(const bool &value, const bool &emitEvent);
        Grid<bool> grid;

    private:
        bool mValue;

};

}
#endif
