#include "valuestring.h"
namespace  OST
{
ValueString::ValueString(const QString &label, const QString &order, const QString &hint)
    : ValueSingle<QString>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovString::lovChanged, this, &ValueSingleSignalAndSlots::OnLovChanged);
}
ValueString::~ValueString()
{
}

}

