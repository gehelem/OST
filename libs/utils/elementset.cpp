#include "elementset.h"
#include <sstream>
namespace  OST
{
void ElementSet::visit(ElementInt *pElement, QVariantMap &data )
{
    pElement->setValue(QVariant(data).toInt(), true);
}
void ElementSet::visit(ElementFloat *pElement, QVariantMap &data )
{
    pElement->setValue(QVariant(data).toDouble(), true);
}
void ElementSet::visit(ElementBool *pElement, QVariantMap &data )
{
    pElement->setValue(QVariant(data).toBool(), true);
}
void ElementSet::visit(ElementString *pElement, QVariantMap &data )
{
    pElement->setValue(QVariant(data).toString(), true);
}
void ElementSet::visit(ElementLight *pElement, QVariantMap &data )
{
    pElement->setValue(IntToState(QVariant(data).toInt()), true);
}
void ElementSet::visit(ElementImg *pElement, QVariantMap &data )
{
    pElement->setValue(QVariantMapToImgData(data), true);
}
void ElementSet::visit(ElementVideo *pElement, QVariantMap &data )
{
    pElement->setValue(VariantMapToVideoData(data), true);
}
void ElementSet::visit(ElementMessage *pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}
void ElementSet::visit(ElementPrg* pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}


}
