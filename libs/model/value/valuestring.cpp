#include "valuestring.h"
namespace  OST
{
ValueString::ValueString(const QString &label, const QString &order, const QString &hint)
    : ValueBase(label, order, hint)
{
}
ValueString::~ValueString()
{
}
void ValueString::setValue(const QString &value, const bool &emitEvent)
{
    mValue = value;
    if (emitEvent) emit valueChanged(this);
}

}
