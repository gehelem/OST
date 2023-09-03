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
    json["type"] = "undefined";
    json["autoupdate"] = pValue->autoUpdate();

    return json;

}
void ValueJsonDumper::visit(ValueBool *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["type"] = "bool";
    json["value"] = pValue->value();
    mResult = json;
}
void ValueJsonDumper::visit(ValueInt *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["type"] = "int";
    json["value"] = qlonglong(pValue->value());
    json["min"] = qlonglong(pValue->min());
    json["max"] = qlonglong(pValue->max());
    json["step"] = qlonglong(pValue->step());
    json["format"] = pValue->format();
    if (pValue->lov.getList().size() > 0)
    {
        QJsonObject lines = QJsonObject();
        foreach(const long &key, pValue->lov.getList().keys())
        {
            lines[QString::number(key)] = pValue->lov.getList()[key];
        }
        json["listOfValues"] = lines;
    }
    if (pValue->grid.getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (long val, pValue->grid.getGrid())
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }
    mResult = json;
}
void ValueJsonDumper::visit(ValueFloat *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["type"] = "float";
    json["value"] = pValue->value();
    json["min"] = pValue->min();
    json["max"] = pValue->max();
    json["step"] = pValue->step();
    json["format"] = pValue->format();
    if (pValue->lov.getList().size() > 0)
    {
        QJsonObject lines = QJsonObject();
        foreach(const float &key, pValue->lov.getList().keys())
        {
            lines[QString::number(key)] = pValue->lov.getList()[key];
        }
        json["listOfValues"] = lines;
    }
    if (pValue->grid.getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (double val, pValue->grid.getGrid())
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }

    mResult = json;
}
void ValueJsonDumper::visit(ValueString *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["type"] = "string";
    json["value"] = pValue->value();
    if (pValue->lov.getList().size() > 0)
    {
        QJsonObject lines = QJsonObject();
        foreach(const QString &key, pValue->lov.getList().keys())
        {
            lines[key] = pValue->lov.getList()[key];
        }
        json["listOfValues"] = lines;
    }
    if (pValue->grid.getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (QString val, pValue->grid.getGrid())
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }

    mResult = json;
}
void ValueJsonDumper::visit(ValueLight *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["type"] = "light";
    mResult = json;
}
void ValueJsonDumper::visit(ValueImg *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["type"] = "img";
    QJsonObject imgdata;

    imgdata["urljpeg"] = pValue->value().mUrlJpeg;
    imgdata["urlfits"] = pValue->value().mUrlFits;
    imgdata["urlthumbnail"] = pValue->value().mUrlThumbnail;
    imgdata["urloverlay"] = pValue->value().mUrlOverlay;
    imgdata["min"] = qlonglong(pValue->value().min);
    imgdata["max"] = qlonglong(pValue->value().max);
    imgdata["width"] = qlonglong(pValue->value().width);
    imgdata["height"] = qlonglong(pValue->value().height);
    imgdata["mean"] = qlonglong(pValue->value().mean);
    imgdata["median"] = qlonglong(pValue->value().median);
    imgdata["stddev"] = qlonglong(pValue->value().stddev);
    imgdata["snr"] = qlonglong(pValue->value().SNR);
    json["value"] = imgdata;
    mResult = json;
}
void ValueJsonDumper::visit(ValueMessage *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["type"] = "message";
    json["level"] = pValue->level();
    json["timestamp"] = pValue->timeStamp().toString("yyyy/MM/dd hh:mm:ss.zzz");
    mResult = json;
}

}
