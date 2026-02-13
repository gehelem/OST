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

void ElementBool::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementBool::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
QString ElementBool::getType()
{
    return "bool";
}

void ValueBool::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueBool::updateValue()
{
    value = static_cast<ElementBool*>(pElement)->value();
}
void ValueBool::updateElement(const SignalType &signalType)
{
    static_cast<ElementBool*>(pElement)->setValue(value, signalType);
}


}
