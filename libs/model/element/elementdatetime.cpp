#include "elementdatetime.h"
namespace  OST
{
ElementDateTime::ElementDateTime(const QString &key, const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(key, label, order, hint)
{
}
ElementDateTime::~ElementDateTime()
{
}
void ElementDateTime::accept(ElementVisitor *pVisitor, QVariantMap &data, bool &emitEvent)
{
    pVisitor->visit(this, data, emitEvent);
}
QString ElementDateTime::getType()
{
    return "datetime";
}
void ValueDateTime::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueDateTime::updateValue()
{
    value = static_cast<ElementDateTime*>(pElement)->value();
}
void ValueDateTime::updateElement(const bool &emitEvent)
{
    static_cast<ElementDateTime*>(pElement)->setValue(value, emitEvent);
}



}
