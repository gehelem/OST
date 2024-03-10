#include "elementjsondumper.h"
#include <sstream>
namespace  OST
{

QJsonObject ElementJsonDumper::dumpElementCommons(ElementBase *pElement)
{
    QJsonObject json;

    json["label"] = pElement->label();
    json["order"] = pElement->order();
    json["hint"] = pElement->hint();
    json["type"] = "undefined";
    json["autoupdate"] = pElement->autoUpdate();
    json["badge"] = pElement->getBadge();
    if ((pElement->getType() == "int") || (pElement->getType() == "float") || (pElement->getType() == "string")
            || (pElement->getType() == "bool"))
    {
        json["directedit"] = pElement->getDirectEdit();
    }

    return json;

}
void ElementJsonDumper::visit(ElementBool *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "bool";
    json["value"] = pElement->value();
    json["arrayLimit"] = pElement->arrayLimit();
    if (pElement->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (bool val, pElement->getGrid())
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }

    mResult = json;
}
void ElementJsonDumper::visit(ElementInt *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "int";
    json["value"] = qlonglong(pElement->value());
    json["min"] = qlonglong(pElement->min());
    json["max"] = qlonglong(pElement->max());
    json["step"] = qlonglong(pElement->step());
    json["format"] = pElement->format();
    json["arrayLimit"] = pElement->arrayLimit();
    if (pElement->getPreIcon() != "") json["preicon"] = pElement->getPreIcon();
    if (pElement->getPostIcon() != "") json["posticon"] = pElement->getPostIcon();
    if (pElement->getGlobalLov() != "")
    {
        json["globallov"] = pElement->getGlobalLov();
    }
    else
    {
        if (pElement->getLov().size() > 0)
        {
            QJsonObject lines = QJsonObject();
            foreach(const long &key, pElement->getLov().keys())
            {
                QString skey = QString::number(key);
                lines[skey] = pElement->getLov()[key];
            }
            json["listOfValues"] = lines;
        }

    }
    if (pElement->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (long val, pElement->getGrid())
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }
    mResult = json;
}
void ElementJsonDumper::visit(ElementFloat *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "float";
    json["value"] = pElement->value();
    json["min"] = pElement->min();
    json["max"] = pElement->max();
    json["step"] = pElement->step();
    json["format"] = pElement->format();
    json["arrayLimit"] = pElement->arrayLimit();
    if (pElement->getPreIcon() != "") json["preicon"] = pElement->getPreIcon();
    if (pElement->getPostIcon() != "") json["posticon"] = pElement->getPostIcon();
    if (pElement->getGlobalLov() != "")
    {
        json["globallov"] = pElement->getGlobalLov();
    }
    else
    {
        if (!pElement->getLov().isEmpty())
        {
            QJsonObject lines = QJsonObject();
            foreach(const double &key, pElement->getLov().keys())
            {
                QString skey = QString::number(key);
                lines[skey] = pElement->getLov()[key];
            }
            json["listOfValues"] = lines;
        }
    }
    if (pElement->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (double val, pElement->getGrid())
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }

    mResult = json;
}
void ElementJsonDumper::visit(ElementString *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "string";
    json["value"] = pElement->value();
    if (pElement->getPreIcon() != "") json["preicon"] = pElement->getPreIcon();
    if (pElement->getPostIcon() != "") json["posticon"] = pElement->getPostIcon();
    json["arrayLimit"] = pElement->arrayLimit();
    if (pElement->getGlobalLov() != "")
    {
        json["globallov"] = pElement->getGlobalLov();
    }
    else
    {
        if (pElement->getLov().size() > 0)
        {
            QJsonObject lines = QJsonObject();
            foreach(const QString &key, pElement->getLov().keys())
            {
                lines[key] = pElement->getLov()[key];
            }
            json["listOfValues"] = lines;
        }
    }
    if (pElement->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (QString val, pElement->getGrid())
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }

    mResult = json;
}
void ElementJsonDumper::visit(ElementLight *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "light";
    json["value"] = StateToInt(pElement->value());
    mResult = json;
}
void ElementJsonDumper::visit(ElementImg *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "img";

    json["arrayLimit"] = pElement->arrayLimit();
    if (pElement->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (ImgData val, pElement->getGrid())
        {
            QVariant v = QVariant::fromValue(ImgDataToVariantMap(val));
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }

    QJsonObject imgdata;
    imgdata["urljpeg"] = pElement->value().mUrlJpeg;
    imgdata["urlfits"] = pElement->value().mUrlFits;
    imgdata["urlthumbnail"] = pElement->value().mUrlThumbnail;
    imgdata["urloverlay"] = pElement->value().mUrlOverlay;
    imgdata["channels"] = pElement->value().channels;
    imgdata["width"] = pElement->value().width;
    imgdata["height"] = pElement->value().height;
    imgdata["snr"] = pElement->value().SNR;
    imgdata["hfravg"] = pElement->value().HFRavg;
    imgdata["stars"] = pElement->value().starsCount;
    imgdata["issolved"] = pElement->value().isSolved;
    imgdata["solverra"] = pElement->value().solverRA;
    imgdata["solverde"] = pElement->value().solverDE;


    QJsonArray arr;
    arr = QJsonArray();
    for (int i = 0; i < pElement->value().channels; i++)
    {
        arr.append(pElement->value().min[i]);
    }
    imgdata["min"] = arr;

    arr = QJsonArray();
    for (int i = 0; i < pElement->value().channels; i++)
    {
        arr.append(pElement->value().max[i]);
    }
    imgdata["max"] = arr;

    arr = QJsonArray();
    for (int i = 0; i < pElement->value().channels; i++)
    {
        arr.append(pElement->value().mean[i]);
    }
    imgdata["mean"] = arr;

    arr = QJsonArray();
    for (int i = 0; i < pElement->value().channels; i++)
    {
        arr.append(pElement->value().median[i]);
    }
    imgdata["median"] = arr;

    arr = QJsonArray();
    for (int i = 0; i < pElement->value().channels; i++)
    {
        arr.append(pElement->value().stddev[i]);
    }
    imgdata["stddev"] = arr;

    arr = QJsonArray();

    for (int i = 0; i <  pElement->value().channels; i++  )
    {
        QJsonArray oneChannel = QJsonArray();
        QJsonArray freq = QJsonArray();
        for (int j = 0; j < pElement->value().histogram[0].size(); j++  )
        {
            freq = QJsonArray();
            freq.append(pElement->value().histogram[i][j]);
            oneChannel.append(freq);
        }
        arr.append(oneChannel);
    }
    imgdata["histogram"] = arr;

    json["value"] = imgdata;
    mResult = json;
}
void ElementJsonDumper::visit(ElementVideo *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "video";
    json["url"] = pElement->value().url;
    json["arrayLimit"] = pElement->arrayLimit();
    if (pElement->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (VideoData val, pElement->getGrid())
        {
            QVariant v = QVariant::fromValue(VideoDataToVariantMap(val));
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }

    mResult = json;
}
void ElementJsonDumper::visit(ElementMessage *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "message";
    json["level"] = pElement->level();
    json["timestamp"] = pElement->timeStamp().toString("yyyy/MM/dd hh:mm:ss.zzz");
    mResult = json;
}
void ElementJsonDumper::visit(ElementGraph *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "graph";
    json["graphtype"] = GraphTypeToString(pElement->getGraphDefs().type);
    json["params"] = QJsonObject::fromVariantMap(pElement->getGraphDefs().params);

    mResult = json;
}
void ElementJsonDumper::visit(ElementPrg *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "prg";
    if (pElement->prgType() == bar) json["prgtype"] = "bar";
    if (pElement->prgType() == spinner) json["prgtype"] = "spinner";
    json["dynlabel"] = pElement->dynLabel();
    json["value"] = pElement->value();

    json["arrayLimit"] = pElement->arrayLimit();
    if (pElement->getGrid().size() > 0)
    {
        QJsonArray arr;
        foreach (double val, pElement->getGrid())
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }

    mResult = json;
}

}
