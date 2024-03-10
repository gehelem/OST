#include "valuebool.h"
namespace  OST
{
ValueBool::ValueBool(const QString &label, const QString &order, const QString &hint)
    : ValueSingleNotNumeric(label, order, hint)
{
    setValue(false, false);
}
ValueBool::~ValueBool()
{
}

}
