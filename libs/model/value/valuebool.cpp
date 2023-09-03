#include "valuebool.h"
namespace  OST
{
ValueBool::ValueBool(const QString &label, const QString &order, const QString &hint)
    : ValueBase(label, order, hint)
{
}
ValueBool::~ValueBool()
{
}
void ValueBool::setValue(const bool &value, const bool &emitEvent)
{
    mValue = value;
    if (emitEvent) emit valueChanged(this);
}

}
