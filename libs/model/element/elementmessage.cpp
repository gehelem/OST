#include "elementmessage.h"
namespace  OST
{
ElementMessage::ElementMessage(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(label, order, hint)
{
}
ElementMessage::~ElementMessage()
{
}
void ElementMessage::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementMessage::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
void ElementMessage::accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data)
{
    pVisitor->visit(this, action, data);
}

QString ElementMessage::getType()
{
    return "message";
}
QString ElementMessage::message()
{
    return value().message;
}
MsgLevel ElementMessage::level()
{
    return value().level;
}
QDateTime ElementMessage::ts()
{
    return value().ts;
}

void ValueMessage::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueMessage::updateValue()
{
    value = static_cast<ElementMessage*>(pElement)->value();
}
void ValueMessage::updateElement(const bool &emitEvent)
{
    static_cast<ElementMessage*>(pElement)->setValue(value, emitEvent);
}


}
