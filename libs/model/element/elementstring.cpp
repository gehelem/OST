#include "elementstring.h"
namespace  OST
{
ElementString::ElementString(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric<QString>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovString::lovChanged, this, &ElementTemplate::OnLovChanged);
}
ElementString::~ElementString()
{
}

}

