#ifndef VALUELIGHT_h
#define VALUELIGHT_h

#include <valuesingle.h>

namespace  OST
{

class ValueLight: public ValueSingle<State>
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

        ValueLight(const QString &label, const QString &order, const QString &hint);
        ~ValueLight();
        QString getType() override
        {
            return "light";
        }

    private:

};

}
#endif
