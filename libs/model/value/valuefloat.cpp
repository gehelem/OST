#include "valuefloat.h"
namespace  OST
{
ValueFloat::ValueFloat(const QString &label, const QString &order, const QString &hint)
    : ValueBase(label, order, hint)
{
}
ValueFloat::~ValueFloat()
{
}
void ValueFloat::setValue(const double &value, const bool &emitEvent)
{
    mValue = value;
    if (emitEvent) emit valueChanged(this);
}
void ValueFloat::setMin(const double &min)
{
    mMin = min;
}
void ValueFloat::setMax(const double &max)
{
    mMax = max;
}
void ValueFloat::setStep(const double &step)
{
    mStep = step;
}
void ValueFloat::setFormat(const QString &format)
{
    mFormat = format;
}

}
