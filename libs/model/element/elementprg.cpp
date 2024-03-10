#include "elementprg.h"
namespace  OST
{
ElementPrg::ElementPrg(const QString &label, const QString &order, const QString &hint)
    : ElementSingleNotNumeric(label, order, hint)
{
}
ElementPrg::~ElementPrg()
{
}

}
