#include "elementlight.h"
namespace  OST
{
ElementLight::ElementLight(const QString &key, const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(key, label, order, hint)
{
    setValue(Idle, false);
}
ElementLight::~ElementLight()
{
}
void ElementLight::accept(ElementVisitor *pVisitor, QVariantMap &data, bool &emitEvent)
{
    pVisitor->visit(this, data, emitEvent);
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
