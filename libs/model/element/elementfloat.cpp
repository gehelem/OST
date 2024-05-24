#include "elementfloat.h"
namespace  OST
{
ElementFloat::ElementFloat(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNumeric<double>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovFloat::lovChanged, this, &ElementTemplate::OnLovChanged);
}
ElementFloat::~ElementFloat()
{
}

}
