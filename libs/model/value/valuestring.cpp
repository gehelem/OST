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
void ValueString::setValue(const QString &value)
{
    mValue = value;
}

}
