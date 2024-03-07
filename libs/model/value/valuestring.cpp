#include "valuestring.h"
namespace  OST
{
ValueString::ValueString(const QString &label, const QString &order, const QString &hint)
    : ValueSingleNotNumeric<QString>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovString::lovChanged, this, &ValueSingle::OnLovChanged);
}
ValueString::~ValueString()
{
}

}

