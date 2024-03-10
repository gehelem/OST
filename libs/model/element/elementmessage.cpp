#include "elementmessage.h"
namespace  OST
{
ElementMessage::ElementMessage(const QString &label, const QString &order, const QString &hint)
    : ElementSingleNotNumeric(label, order, hint)
{
}
ElementMessage::~ElementMessage()
{
}

}
