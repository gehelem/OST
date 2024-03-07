#include "valueprg.h"
namespace  OST
{
ValuePrg::ValuePrg(const QString &label, const QString &order, const QString &hint)
    : ValueSingleNotNumeric(label, order, hint)
{
}
ValuePrg::~ValuePrg()
{
}

}
