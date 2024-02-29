#ifndef VALUEIMG_h
#define VALUEIMG_h

#include <valuesingle.h>

namespace  OST
{

class ValueImg: public ValueSingle<ImgData>
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

        ValueImg(const QString &label, const QString &order, const QString &hint);
        ~ValueImg();
        QString getType() override
        {
            return "img";
        }
    private:

};

}
#endif
