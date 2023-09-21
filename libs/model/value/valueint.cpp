#include "valueint.h"
namespace  OST
{
ValueInt::ValueInt(const QString &label, const QString &order, const QString &hint)
    : ValueSingle<int>(label, order, hint), mLov(label)
{
}
ValueInt::~ValueInt()
{
}
void ValueInt::setMin(const int &min)
{
    mMin = min;
}
void ValueInt::setMax(const int &max)
{
    mMax = max;
}
void ValueInt::setStep(const int &step)
{
    mStep = step;
}
void ValueInt::setFormat(const QString &format)
{
    mFormat = format;
}

}
