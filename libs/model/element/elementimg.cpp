#include "elementimg.h"
namespace  OST
{
ElementImg::ElementImg(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(label, order, hint)
{
}
ElementImg::~ElementImg()
{
}
void ElementImg::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementImg::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
void ElementImg::accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data)
{
    pVisitor->visit(this, action, data);
}

QString ElementImg::getType()
{
    return "img";
}
bool ElementImg::getShowStats(void)
{
    return mShowStats;
}
void ElementImg::setShowStats(bool b)
{
    mShowStats = b;
}


void ValueImg::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueImg::updateValue()
{
    value = static_cast<ElementImg*>(pElement)->value();
}
void ValueImg::updateElement(const bool &emitEvent)
{
    static_cast<ElementImg*>(pElement)->setValue(value, emitEvent);
}
}
