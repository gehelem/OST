#include "valuebool.h"
namespace  OST
{
ValueBool::ValueBool(const QString &label, const QString &order, const QString &hint)
    : ValueSingle(label, order, hint)
{
    setValue(false, false);
}
ValueBool::~ValueBool()
{
}

}
