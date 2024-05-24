#ifndef ELEMENTVIDEO_h
#define ELEMENTVIDEO_h

#include <elementtemplate.h>

namespace  OST
{

class ElementVideo: public ElementTemplateNotNumeric<VideoData>
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

class ValueVideo: public ValueTemplate<VideoData>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValueVideo(ElementBase* element): ValueTemplate<VideoData>(element) {}
        ~ValueVideo() {}
        void updateValue() override
        {
            value = static_cast<ElementVideo*>(pElement)->value();
        }
        void updateElement(const bool &emitEvent) override
        {
            static_cast<ElementVideo*>(pElement)->setValue(value, emitEvent);
        }

};
}
#endif
