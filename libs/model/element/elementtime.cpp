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
void ElementTime::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementTime::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
void ElementTime::accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data)
{
    pVisitor->visit(this, action, data);
}

QString ElementTime::getType()
{
    return "time";
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
