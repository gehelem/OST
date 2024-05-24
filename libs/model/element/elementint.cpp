#include "elementint.h"
namespace  OST
{
ElementInt::ElementInt(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNumeric<int>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovInt::lovChanged, this, &ElementTemplate::OnLovChanged);
}
ElementInt::~ElementInt()
{
}

}
