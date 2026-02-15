#include "elementtime.h"
namespace  OST
{
ElementTime::ElementTime(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(label, order, hint)
{
}
ElementTime::~ElementTime()
{
}
void ElementTime::accept(ElementVisitor *pVisitor, QVariantMap &data, bool &emitEvent)
{
    pVisitor->visit(this, data, emitEvent);
}
QString ElementTime::getType()
{
    return "time";
}
void ElementTime::setUseMs(bool b)
{
    mUseMs = b;
}
bool ElementTime::getUseMs(void)
{
    return mUseMs;
}
void ValueTime::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueTime::updateValue()
{
    value = static_cast<ElementTime*>(pElement)->value();
}
void ValueTime::updateElement(const bool &emitEvent)
{
    static_cast<ElementTime*>(pElement)->setValue(value, emitEvent);
}



}
