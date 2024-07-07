#include "elementint.h"
namespace  OST
{
ElementInt::ElementInt(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNumeric<int>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovInt::lovChanged, this, &ElementTemplate::OnLovChanged);
}
ElementInt::~ElementInt()
{
}
void ElementInt::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementInt::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
void ElementInt::accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data)
{
    pVisitor->visit(this, action, data);
}

QString ElementInt::getType()
{
    return "int";
}
QMap<int, QString> ElementInt::getLov()
{
    return mLov.getLov();
}
bool ElementInt::lovAdd(int val, QString label)
{
    return mLov.lovAdd(val, label);
}
bool ElementInt::lovUpdate(int  val, QString label)
{
    return mLov.lovUpdate(val, label);
}
bool ElementInt::lovDel(int  val)
{
    return mLov.lovDel(val);
}
bool ElementInt::lovClear()
{
    return mLov.lovClear();
}

void ValueInt::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueInt::updateValue()
{
    value = static_cast<ElementInt*>(pElement)->value();
}
void ValueInt::updateElement(const bool &emitEvent)
{
    static_cast<ElementInt*>(pElement)->setValue(value, emitEvent);
}



}
