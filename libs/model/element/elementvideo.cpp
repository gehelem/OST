#include "elementvideo.h"
namespace  OST
{
ElementVideo::ElementVideo(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(label, order, hint)
{
}
ElementVideo::~ElementVideo()
{
}
void ElementVideo::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementVideo::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
QString ElementVideo::getType()
{
    return "video";
}

void ValueVideo::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueVideo::updateValue()
{
    value = static_cast<ElementVideo*>(pElement)->value();
}
void ValueVideo::updateElement(const SignalType &signalType)
{
    static_cast<ElementVideo*>(pElement)->setValue(value, signalType);
}


}
