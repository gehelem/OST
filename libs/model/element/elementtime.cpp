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
void ElementTime::accept(ElementVisitor *pVisitor, QVariantMap &data, SignalType &signalType)
{
    pVisitor->visit(this, data, signalType);
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
void ValueTime::updateElement(const SignalType &signalType)
{
    static_cast<ElementTime*>(pElement)->setValue(value, signalType);
}



}
