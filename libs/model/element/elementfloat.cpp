#include "valuefloat.h"
namespace  OST
{
ValueFloat::ValueFloat(const QString &label, const QString &order, const QString &hint)
    : ValueSingleNumeric<double>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovFloat::lovChanged, this, &ValueSingle::OnLovChanged);
}
ValueFloat::~ValueFloat()
{
}

}
