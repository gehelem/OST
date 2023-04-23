#ifndef VALUESTRING_h
#define VALUESTRING_h

#include "valuebase.h"

namespace  OST
{

class ValueString: public ValueBase
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

        ValueString(const QString &label, const QString &order, const QString &hint);
        ~ValueString();
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
