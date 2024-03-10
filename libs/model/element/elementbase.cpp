#include "elementbase.h"
namespace  OST
{

ElementBase::ElementBase(const QString &label, const QString &order, const QString &hint)
    : mLabel(label), mOrder(order), mHint(hint)
{
}
ElementBase::~ElementBase()
{

}

GridBase::GridBase(ElementBase* value)
{

}
GridBase::~GridBase()
{

}



}
