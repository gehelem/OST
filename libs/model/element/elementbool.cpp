#include "elementbool.h"
namespace  OST
{
ElementBool::ElementBool(const QString &label, const QString &order, const QString &hint)
    : ElementSingleNotNumeric(label, order, hint)
{
    setValue(false, false);
}
ElementBool::~ElementBool()
{
}

}
