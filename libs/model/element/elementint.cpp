#include "elementint.h"
namespace  OST
{
ElementInt::ElementInt(const QString &label, const QString &order, const QString &hint)
    : ElementSingleNumeric<int>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovInt::lovChanged, this, &ElementSingle::OnLovChanged);
}
ElementInt::~ElementInt()
{
}

}
