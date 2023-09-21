#ifndef VALUESTRING_h
#define VALUESTRING_h
#include <valuesingle.h>
#include <lovstring.h>
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
        QMap<QString, QString> getLov()
        {
            return mLov.getLov();
        }
        bool lovAdd(QString val, QString label)
        {
            return mLov.lovAdd(val, label);
        }
        bool lovUpdate(QString  val, QString label)
        {
            return mLov.lovUpdate(val, label);
        }
        bool lovDel(QString  val)
        {
            return mLov.lovDel(val);
        }
        bool lovClear()
        {
            return mLov.lovClear();
        }
    private:
        LovString mLov;

};

}
#endif
