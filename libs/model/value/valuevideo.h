#ifndef VALUEVIDEO_h
#define VALUEVIDEO_h

#include <valuesingle.h>

namespace  OST
{

class ValueVideo: public ValueSingleNotNumeric<VideoData>
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
    private:

};

}
#endif
