#ifndef ELEMENTIMG_h
#define ELEMENTIMG_h

#include <elementtemplate.h>

namespace  OST
{

class ElementImg: public ElementTemplateNotNumeric<ImgData>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;
        ElementImg(const QString &label, const QString &order, const QString &hint);
        ~ElementImg();
        QString getType() override;
    private:

};

class ValueImg: public ValueTemplate<ImgData>
{
        Q_OBJECT

    public:
        void accept(ValueVisitor* pVisitor)  override;
        ValueImg(ElementBase* element): ValueTemplate<ImgData>(element) {}
        ~ValueImg() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};
}
#endif
