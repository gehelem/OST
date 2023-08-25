#include "valuemessage.h"
namespace  OST
{
ValueMessage::ValueMessage(const QString &label, const QString &order, const QString &hint)
    : ValueBase(label, order, hint)
{
}
ValueMessage::~ValueMessage()
{
}
void ValueMessage::setValue(const QString &value)
{
    mValue = value;
}

}
