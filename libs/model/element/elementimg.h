#ifndef ELEMENTIMG_h
#define ELEMENTIMG_h

#include <elementsingle.h>

namespace  OST
{

class ElementImg: public ElementSingleNotNumeric<ImgData>
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

class GridImg: public GridTemplate<ImgData>
{

        Q_OBJECT

    public:

        void accept(GridVisitor* pVisitor, QString &action, int &line)  override
        {
            pVisitor->visit(this, action, line);
        }

        GridImg(ElementBase* value): GridTemplate<ImgData>(value), mElement(static_cast<ElementImg*>(value)) {}
        ~GridImg() {}
        QString getType() override
        {
            return "img";
        }
        ElementImg*  mElement;
};


}
#endif
