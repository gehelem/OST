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

}
