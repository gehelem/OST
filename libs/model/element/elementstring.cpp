#include "elementstring.h"
namespace  OST
{
ElementString::ElementString(const QString &label, const QString &order, const QString &hint)
    : ElementTemplateNotNumeric<QString>(label, order, hint), mLov(label)
{
    connect(&mLov, &LovString::lovChanged, this, &ElementTemplate::OnLovChanged);
}
ElementString::~ElementString()
{
}
void ElementString::accept(ElementVisitor *pVisitor)
{
    pVisitor->visit(this);
}
void ElementString::accept(ElementVisitor *pVisitor, QVariantMap &data)
{
    pVisitor->visit(this, data);
}
void ElementString::accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data)
{
    pVisitor->visit(this, action, data);
}

QString ElementString::getType()
{
    return "string";
}
QMap<QString, QString> ElementString::getLov()
{
    return mLov.getLov();
}
bool ElementString::lovAdd(QString val, QString label)
{
    return mLov.lovAdd(val, label);
}
bool ElementString::lovUpdate(QString  val, QString label)
{
    return mLov.lovUpdate(val, label);
}
bool ElementString::lovDel(QString  val)
{
    return mLov.lovDel(val);
}
bool ElementString::lovClear()
{
    return mLov.lovClear();
}

void ValueString::accept(ValueVisitor* pVisitor)
{
    pVisitor->visit(this);
}
void ValueString::updateValue()
{
    value = static_cast<ElementString*>(pElement)->value();
}
void ValueString::updateElement(const bool &emitEvent)
{
    static_cast<ElementString*>(pElement)->setValue(value, emitEvent);
}


}

