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
void ElementPrg::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementPrg::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
void ElementPrg::accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data)
{
    pVisitor->visit(this, action, data);
}

QString ElementPrg::getType()
{
    return "prg";
}
void ElementPrg::setPrgValue(const double &v, const bool &emitEvent)
{
    if ((v < 0) || (v > 100))
    {
        sendMessage(OST::MsgLevel::Warn, "Invalid spinner value (" + QString::number(v) + ")");
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
void ValuePrg::updateElement(const bool &emitEvent)
{
    static_cast<ElementPrg*>(pElement)->setValue(value, emitEvent);
}



}
