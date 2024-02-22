#ifndef VALUEBOOL_h
#define VALUEBOOL_h

#include <valuesingle.h>

namespace  OST
{

class ValueBool: public ValueSingle<bool>
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
        QString getType() override
        {
            return "bool";
        }

    private:

};

}
#endif
