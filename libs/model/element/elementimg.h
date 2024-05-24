#ifndef ELEMENTIMG_h
#define ELEMENTIMG_h

#include <elementtemplate.h>

namespace  OST
{

class ElementImg: public ElementTemplateNotNumeric<ImgData>
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

        ElementImg(const QString &label, const QString &order, const QString &hint);
        ~ElementImg();
        QString getType() override
        {
            return "img";
        }
    private:

};

class ValueImg: public ValueTemplate<ImgData>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValueImg(ElementBase* element): ValueTemplate<ImgData>(element) {}
        ~ValueImg() {}
        void updateValue() override
        {
            value = static_cast<ElementImg*>(pElement)->value();
        }
        void updateElement(const bool &emitEvent) override
        {
            static_cast<ElementImg*>(pElement)->setValue(value, emitEvent);
        }

};
}
#endif
