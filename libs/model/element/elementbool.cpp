#include "elementbool.h"
namespace  OST
{
ElementBool::ElementBool(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(label, order, hint)
{
    setValue(false, false);
}
ElementBool::~ElementBool()
{
}

}
