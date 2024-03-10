#include "elementstring.h"
namespace  OST
{
ElementString::ElementString(const QString &label, const QString &order, const QString &hint)
    : ElementSingleNotNumeric<QString>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovString::lovChanged, this, &ElementSingle::OnLovChanged);
}
ElementString::~ElementString()
{
}

}

