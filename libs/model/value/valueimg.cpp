#include "valueimg.h"
namespace  OST
{
ValueImg::ValueImg(const QString &label, const QString &order, const QString &hint)
    : ValueSingleNotNumeric(label, order, hint)
{
}
ValueImg::~ValueImg()
{
}

}
