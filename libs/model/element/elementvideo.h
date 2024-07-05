#ifndef ELEMENTVIDEO_h
#define ELEMENTVIDEO_h

#include <elementtemplate.h>

namespace  OST
{

class ElementVideo: public ElementTemplateNotNumeric<VideoData>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementVideo(const QString &label, const QString &order, const QString &hint);
        ~ElementVideo();
        QString getType() override;
    private:

};

class ValueVideo: public ValueTemplate<VideoData>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override;
        ValueVideo(ElementBase* element): ValueTemplate<VideoData>(element) {}
        ~ValueVideo() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};
}
#endif
