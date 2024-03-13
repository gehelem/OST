#ifndef ELEMENTVIDEO_h
#define ELEMENTVIDEO_h

#include <elementsingle.h>

namespace  OST
{

class ElementVideo: public ElementSingleNotNumeric<VideoData>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override
        {
            pVisitor->visit(this, action, data);
        }

        ElementVideo(const QString &label, const QString &order, const QString &hint);
        ~ElementVideo();
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

        GridVideo(ElementBase* value): GridTemplate<VideoData>(value), mElement(static_cast<ElementVideo*>(value)) {}
        ~GridVideo() {}

        QString getType() override
        {
            return "video";
        }
        ElementVideo* mElement;

};
}
#endif