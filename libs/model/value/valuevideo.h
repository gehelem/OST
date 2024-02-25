#ifndef VALUEVIDEO_h
#define VALUEVIDEO_h

#include <valuesingle.h>

namespace  OST
{

class ValueVideo: public ValueSingle<QString>
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

        ValueVideo(const QString &label, const QString &order, const QString &hint);
        ~ValueVideo();
        QString getType() override
        {
            return "video";
        }
        QString value()
        {
            return mValue;
        }
        void setValue(const QString &value, const bool &emitEvent)
        {
            mValue = value;
            if (emitEvent) emit valueChanged(this);
        }

    private:
        QString mValue = "";

};

}
#endif
