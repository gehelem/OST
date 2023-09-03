#include "valuebase.h"
namespace  OST
{

ValueBase::ValueBase(const QString &label, const QString &order, const QString &hint)
    : mLabel(label), mOrder(order), mHint(hint)
{
}
ValueBase::~ValueBase()
{

}
void ValueBase::setState(const State &state)
{
    mState = state;
}

}
