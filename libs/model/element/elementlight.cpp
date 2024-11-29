#include "elementlight.h"
namespace  OST
{
ElementLight::ElementLight(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(label, order, hint)
{
    setValue(Idle, false);
}
ElementLight::~ElementLight()
{
}
void ElementLight::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementLight::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
void ElementLight::accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data)
{
    pVisitor->visit(this, action, data);
}

QString ElementLight::getType()
{
    return "light";
}


void ValueLight::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueLight::updateValue()
{
    value = static_cast<ElementLight*>(pElement)->value();
}
void ValueLight::updateElement(const bool &emitEvent)
{
    static_cast<ElementLight*>(pElement)->setValue(value, emitEvent);
}

}
