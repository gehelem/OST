#ifndef VALUEMESSAGE_h
#define VALUEMESSAGE_h

#include <valuebase.h>

namespace  OST
{

class ValueMessage: public ValueBase
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

        ValueMessage(const QString &label, const QString &order, const QString &hint);
        ~ValueMessage();
        QString value()
        {
            return mValue;
        }
        void setValue(const QString &value);

    private:
        QString mValue;

};

}
#endif
