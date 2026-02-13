#include "elementprg.h"
namespace  OST
{
ElementPrg::ElementPrg(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric(label, order, hint)
{
}
ElementPrg::~ElementPrg()
{
}
void ElementPrg::accept(ElementVisitor *pVisitor, QVariantMap &data, SignalType &signalType)
{
    pVisitor->visit(this, data, signalType);
}
QString ElementPrg::getType()
{
    return "prg";
}
void ElementPrg::setPrgValue(const double &v, const SignalType &signalType)
{
    if ((v < 0) || (v > 100))
    {
        logMessage(OST::LogLevel::Warning, "Invalid spinner value (%1)", {QString::number(v)});
        return;
    }
    PrgData d = value();
    d.value = v;
    ElementTemplateNotNumeric<PrgData>::setValue(d, signalType);
}
void ElementPrg::setDynLabel(const QString &s, const SignalType &signalType)
{
    PrgData d = value();
    d.dynlabel = s;
    ElementTemplateNotNumeric<PrgData>::setValue(d, signalType);
}
QString ElementPrg::dynLabel()
{
    return mDynLabel;
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
void ValuePrg::updateElement(const SignalType &signalType)
{
    static_cast<ElementPrg*>(pElement)->setValue(value, signalType);
}



}
