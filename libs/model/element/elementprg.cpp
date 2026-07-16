#include "elementprg.h"
namespace  OST
{
ElementPrg::ElementPrg(const QString &key, const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(key, label, order, hint)
{
}
ElementPrg::~ElementPrg()
{
}
void ElementPrg::accept(ElementVisitor *pVisitor, QVariantMap &data, bool &emitEvent)
{
    pVisitor->visit(this, data, emitEvent);
}
QString ElementPrg::getType()
{
    return "prg";
}
void ElementPrg::setPrgValue(const double &v, const bool &emitEvent)
{
    if ((v < 0) || (v > 100))
    {
        logMessage(OST::LogLevel::Warning, "Invalid spinner value (%1)", {QString::number(v)});
        return;
    }
    PrgData d = value();
    d.value = v;
    ElementTemplateNotNumeric<PrgData>::setValue(d, emitEvent);
}
void ElementPrg::setDynLabel(const QString &s, const bool &emitEvent)
{
    PrgData d = value();
    d.dynlabel = s;
    ElementTemplateNotNumeric<PrgData>::setValue(d, emitEvent);
}
QString ElementPrg::dynLabel()
{
    return value().dynlabel;
}
PrgType ElementPrg::prgType()
{
    return mType;
}
void ElementPrg::setPrgType(PrgType t)
{
    mType = t;
}

void ValuePrg::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValuePrg::updateValue()
{
    value = static_cast<ElementPrg*>(pElement)->value();
}
void ValuePrg::updateElement(const bool &emitEvent)
{
    static_cast<ElementPrg*>(pElement)->setValue(value, emitEvent);
}



}
