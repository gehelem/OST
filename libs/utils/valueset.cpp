#include "valueset.h"
#include <sstream>
namespace  OST
{
void ValueSet::visit(ValueInt *pValue, QVariantMap &data )
{
    pValue->setValue(QVariant(data).toInt(), true);
}
void ValueSet::visit(ValueFloat *pValue, QVariantMap &data )
{
    pValue->setValue(QVariant(data).toDouble(), true);
}
void ValueSet::visit(ValueBool *pValue, QVariantMap &data )
{
    pValue->setValue(QVariant(data).toBool(), true);
}
void ValueSet::visit(ValueString *pValue, QVariantMap &data )
{
    pValue->setValue(QVariant(data).toString(), true);
}
void ValueSet::visit(ValueLight *pValue, QVariantMap &data )
{
    pValue->setValue(IntToState(QVariant(data).toInt()), true);
}
void ValueSet::visit(ValueImg *pValue, QVariantMap &data )
{
    pValue->setValue(QVariantMapToImgData(data), true);
}
void ValueSet::visit(ValueVideo *pValue, QVariantMap &data )
{
    pValue->setValue(VariantMapToVideoData(data), true);
}
void ValueSet::visit(ValueMessage *pValue, QVariantMap &data )
{
    Q_UNUSED(pValue);
    Q_UNUSED(data);
}
void ValueSet::visit(ValueGraph* pValue, QVariantMap &data )
{
    Q_UNUSED(pValue);
    Q_UNUSED(data);
}
void ValueSet::visit(ValuePrg* pValue, QVariantMap &data )
{
    Q_UNUSED(pValue);
    Q_UNUSED(data);
}


}
