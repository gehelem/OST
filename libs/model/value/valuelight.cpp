#include "valuelight.h"
namespace  OST
{
ValueLight::ValueLight(const QString &label, const QString &order, const QString &hint)
    : ValueSingle(label, order, hint)
{
    setValue(Idle, false);
}
ValueLight::~ValueLight()
{
}

}
