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
    json["type"] = "undefined";
    json["autoupdate"] = pValue->autoUpdate();
    if ((pValue->getType() == "int") || (pValue->getType() == "float") || (pValue->getType() == "string")
            || (pValue->getType() == "bool"))
    {
        json["directedit"] = pValue->getDirectEdit();
    }

    return json;

}
void ValueJsonDumper::visit(ValueBool *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["type"] = "bool";
    json["value"] = pValue->value();
    if (pValue->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (bool val, pValue->getGrid())
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }

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
    if (pValue->getGlobalLov() != "")
    {
        json["globallov"] = pValue->getGlobalLov();
    }
    else
    {
        if (pValue->getLov().size() > 0)
        {
            QJsonObject lines = QJsonObject();
            foreach(const long &key, pValue->getLov().keys())
            {
                QString skey = QString::number(key);
                lines[skey] = pValue->getLov()[key];
            }
            json["listOfValues"] = lines;
        }

    }
    if (pValue->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (long val, pValue->getGrid())
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
    if (pValue->getGlobalLov() != "")
    {
        json["globallov"] = pValue->getGlobalLov();
    }
    else
    {
        if (!pValue->getLov().isEmpty())
        {
            QJsonObject lines = QJsonObject();
            foreach(const double &key, pValue->getLov().keys())
            {
                QString skey = QString::number(key);
                lines[skey] = pValue->getLov()[key];
            }
            json["listOfValues"] = lines;
        }
    }
    if (pValue->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (double val, pValue->getGrid())
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
    if (pValue->getGlobalLov() != "")
    {
        json["globallov"] = pValue->getGlobalLov();
    }
    else
    {
        if (pValue->getLov().size() > 0)
        {
            QJsonObject lines = QJsonObject();
            foreach(const QString &key, pValue->getLov().keys())
            {
                lines[key] = pValue->getLov()[key];
            }
            json["listOfValues"] = lines;
        }
    }
    if (pValue->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (QString val, pValue->getGrid())
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
    json["value"] = StateToInt(pValue->value());
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
void ValueJsonDumper::visit(ValueVideo *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["type"] = "video";
    json["value"] = pValue->value();
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
void ValueJsonDumper::visit(ValueGraph *pValue)
{
    QJsonObject json = dumpValueCommons(pValue);
    json["type"] = "graph";
    json["graphtype"] = GraphTypeToString(pValue->getGraphDefs().type);
    json["params"] = QJsonObject::fromVariantMap(pValue->getGraphDefs().params);

    mResult = json;
}

}
