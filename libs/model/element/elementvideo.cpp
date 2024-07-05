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
void ElementVideo::accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data)
{
    pVisitor->visit(this, action, data);
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
void ValueVideo::updateElement(const bool &emitEvent)
{
    static_cast<ElementVideo*>(pElement)->setValue(value, emitEvent);
}


}
