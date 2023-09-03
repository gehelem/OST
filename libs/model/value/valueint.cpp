#include "valueint.h"
namespace  OST
{
ValueInt::ValueInt(const QString &label, const QString &order, const QString &hint)
    : ValueBase(label, order, hint)
{
}
ValueInt::~ValueInt()
{
}
void ValueInt::setValue(const long &value, const bool &emitEvent)
{
    mValue = value;
    if (emitEvent) emit valueChanged(this);
}
void ValueInt::setMin(const long &min)
{
    mMin = min;
}
void ValueInt::setMax(const long &max)
{
    mMax = max;
}
void ValueInt::setStep(const long &step)
{
    mStep = step;
}
void ValueInt::setFormat(const QString &format)
{
    mFormat = format;
}

}
