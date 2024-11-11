#include "elementdate.h"
namespace  OST
{
ElementDate::ElementDate(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(label, order, hint)
{
}
ElementDate::~ElementDate()
{
}
void ElementDate::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementDate::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
void ElementDate::accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data)
{
    pVisitor->visit(this, action, data);
}

QString ElementDate::getType()
{
    return "date";
}
void ValueDate::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueDate::updateValue()
{
    value = static_cast<ElementDate*>(pElement)->value();
}
void ValueDate::updateElement(const bool &emitEvent)
{
    static_cast<ElementDate*>(pElement)->setValue(value, emitEvent);
}



}
