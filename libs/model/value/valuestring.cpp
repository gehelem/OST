#include "valuestring.h"
namespace  OST
{
ValueString::ValueString(const QString &label, const QString &order, const QString &hint)
    : ValueSingle<QString>(label, order, hint), mLov(label)
{
}
ValueString::~ValueString()
{
}

}

