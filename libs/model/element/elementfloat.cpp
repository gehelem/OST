#include "elementfloat.h"
namespace  OST
{
ElementFloat::ElementFloat(const QString &label, const QString &order, const QString &hint)
    : ElementSingleNumeric<double>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovFloat::lovChanged, this, &ElementSingle::OnLovChanged);
}
ElementFloat::~ElementFloat()
{
}

}
