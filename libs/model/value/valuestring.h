#ifndef VALUESTRING_h
#define VALUESTRING_h

#include <valuebase.h>

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
        QString value()
        {
            return mValue;
        }
        void setValue(const QString &value, const bool &emitEvent);
        void setValue(const QString &value, const bool &emitEvent, const double &line);
        ListOfValues<QString> lov;
        Grid<QString> grid;

    private:
        QString mValue;

};

}
#endif
