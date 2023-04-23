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
void ValueBool::setValue(const bool &value)
{
    mValue = value;
}

}
