#include "valuejsondumper.h"
#include <sstream>
namespace  OST
{

QJsonObject ValueJsonDumper::dumpValueCommons(ValueBase *pValue)
{
    QJsonObject json;

    json["label"] = pValue->label();
    json["order"] = pValue->order();
    json["hint"] = pValue->hint();
    json["state"] = pValue->state();

    return json;

}
void ValueJsonDumper::visit(ValueBool *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["value"] = pValue->value();
    mResult = json;
}
void ValueJsonDumper::visit(ValueInt *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["value"] = qlonglong(pValue->value());
    mResult = json;
}
void ValueJsonDumper::visit(ValueFloat *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["value"] = pValue->value();
    mResult = json;
}
void ValueJsonDumper::visit(ValueString *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["value"] = pValue->value();
    mResult = json;
}
void ValueJsonDumper::visit(ValueLight *pValue)
{
    mResult = dumpValueCommons(pValue);;
}

}
