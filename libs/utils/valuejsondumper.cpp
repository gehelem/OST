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
    json["min"] = qlonglong(pValue->min());
    json["max"] = qlonglong(pValue->max());
    json["step"] = qlonglong(pValue->step());
    json["format"] = pValue->format();

    mResult = json;
}
void ValueJsonDumper::visit(ValueFloat *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["value"] = pValue->value();
    json["min"] = pValue->min();
    json["max"] = pValue->max();
    json["step"] = pValue->step();
    json["format"] = pValue->format();

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
void ValueJsonDumper::visit(ValueImg *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    //json["value"] = pValue->value();
    json["urljpeg"] = pValue->urlJpeg();
    json["urlfits"] = pValue->urlFits();
    json["urlthumbnail"] = pValue->urlThumbnail();
    json["urloverlay"] = pValue->urlOverlay();
    json["min"] = qlonglong(pValue->min());
    json["max"] = qlonglong(pValue->max());
    json["width"] = qlonglong(pValue->width());
    json["height"] = qlonglong(pValue->height());
    json["mean"] = pValue->mean();
    json["median"] = qlonglong(pValue->median());
    json["stddev"] = pValue->stddev();
    json["snr"] = pValue->SNR();

    mResult = json;
}
void ValueJsonDumper::visit(ValueMessage *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    //json["value"] = pValue->value();
    mResult = json;
}

}
