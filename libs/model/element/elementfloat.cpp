#include "elementfloat.h"
namespace  OST
{
ElementFloat::ElementFloat(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNumeric<double>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovFloat::lovChanged, this, &ElementTemplate::OnLovChanged);
}
ElementFloat::~ElementFloat()
{
}
void ElementFloat::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementFloat::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
void ElementFloat::accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data)
{
    pVisitor->visit(this, action, data);
}

QString ElementFloat::getType()
{
    return "float";
}
QMap<double, QString> ElementFloat::getLov()
{
    return mLov.getLov();
}
bool ElementFloat::lovAdd(double val, QString label)
{
    return mLov.lovAdd(val, label);
}
bool ElementFloat::lovUpdate(double  val, QString label)
{
    return mLov.lovUpdate(val, label);
}
bool ElementFloat::lovDel(double  val)
{
    return mLov.lovDel(val);
}
bool ElementFloat::lovClear()
{
    return mLov.lovClear();
}

void ValueFloat::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueFloat::updateValue()
{
    value = static_cast<ElementFloat*>(pElement)->value();
}
void ValueFloat::updateElement(const bool &emitEvent)
{
    static_cast<ElementFloat*>(pElement)->setValue(value, emitEvent);
}



}
