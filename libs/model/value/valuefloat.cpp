#include "valuefloat.h"
namespace  OST
{
ValueFloat::ValueFloat(const QString &label, const QString &order, const QString &hint)
    : ValueSingle<double>(label, order, hint), mLov(label)
{
}
ValueFloat::~ValueFloat()
{
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
