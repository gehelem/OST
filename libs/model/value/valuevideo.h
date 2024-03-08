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

class GridVideo: public GridTemplate<VideoData>
{

        Q_OBJECT

    public:

        void accept(GridVisitor* pVisitor, QString &action, int &line)  override
        {
            pVisitor->visit(this, action, line);
        }

        GridVideo(ValueBase* value): GridTemplate<VideoData>(value), mElement(static_cast<ValueVideo*>(value)) {}
        ~GridVideo() {}

        QString getType() override
        {
            return "video";
        }
        ValueVideo* mElement;

};
}
#endif
