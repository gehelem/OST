#include "valuemessage.h"
namespace  OST
{
ValueMessage::ValueMessage(const QString &label, const QString &order, const QString &hint)
    : ValueSingleNotNumeric(label, order, hint)
{
}
ValueMessage::~ValueMessage()
{
}

}
