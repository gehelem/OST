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
void ValueLight::updateElement(const SignalType &signalType)
{
    static_cast<ElementLight*>(pElement)->setValue(value, signalType);
}

}
