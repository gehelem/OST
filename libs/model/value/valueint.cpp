#include "valueint.h"
namespace  OST
{
ValueInt::ValueInt(const QString &label, const QString &order, const QString &hint)
    : ValueSingleNumeric<int>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovInt::lovChanged, this, &ValueSingleSignalAndSlots::OnLovChanged);
}
ValueInt::~ValueInt()
{
}

}
