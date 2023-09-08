#ifndef VALUESTRING_h
#define VALUESTRING_h
#include <valuesingle.h>

namespace  OST
{

class ValueString: public ValueSingle<QString>
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

        ValueString(const QString &label, const QString &order, const QString &hint);
        ~ValueString();
        QString getType() override
        {
            return "string";
        }

    private:

};

}
#endif
