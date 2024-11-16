#include "elementjsondumper.h"
#include "translate.h"

namespace  OST
{

QJsonObject ElementJsonDumper::dumpElementCommons(ElementBase *pElement)
{
    QJsonObject json;

    Translate* t = Translate::GetInstance();
    json["label"] = t->translate(pElement->label());
    json["order"] = pElement->order();
    json["hint"] = t->translate(pElement->hint());
    json["type"] = "undefined";
    json["autoupdate"] = pElement->autoUpdate();
    json["badge"] = pElement->getBadge();
    if ((pElement->getType() == "int") || (pElement->getType() == "float") || (pElement->getType() == "string")
            || (pElement->getType() == "bool") || (pElement->getType() == "date") || (pElement->getType() == "time"))
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
    json["slider"] = pElement->slider();
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
    json["slider"] = pElement->slider();
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
    mResult = json;
}
void ElementJsonDumper::visit(ElementString *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "string";
    json["value"] = pElement->value();
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
            foreach(const QString &key, pElement->getLov().keys())
            {
                lines[key] = pElement->getLov()[key];
            }
            json["listOfValues"] = lines;
        }
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

    QJsonObject imgdata;
    imgdata["showstats"] =  pElement->getShowStats();
    imgdata["urljpeg"] =  pElement->value().mUrlJpeg;
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

    //i=;


    if (pElement->value().mAlternates.size() > 0)
    {
        arr = QJsonArray();
        for (int i = 0; i < pElement->value().mAlternates.size(); i++)
        {
            arr.append(pElement->value().mAlternates[i]);
        }
        imgdata["alternates"] = arr;
    }

    for (auto it = imgdata.constBegin(); it != imgdata.constEnd(); it++)
    {
        json.insert(it.key(), it.value());
    }
    json["type"] = "img";

    mResult = json;
}
void ElementJsonDumper::visit(ElementVideo *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "video";
    json["url"] = pElement->value().url;
    mResult = json;
}
void ElementJsonDumper::visit(ElementMessage *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "message";
    json["level"] = pElement->value().level;
    json["ts"] = pElement->value().ts.toString("yyyy/MM/dd hh:mm:ss.zzz");
    json["message"] = pElement->value().message;
    mResult = json;
}
void ElementJsonDumper::visit(ElementPrg *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "prg";
    if (pElement->prgType() == bar) json["prgtype"] = "bar";
    if (pElement->prgType() == spinner) json["prgtype"] = "spinner";
    json["dynlabel"] = pElement->value().dynlabel;
    json["value"] = pElement->value().value;

    mResult = json;
}
void ElementJsonDumper::visit(ElementDate *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "date";
    json["year"] = pElement->value().year();
    json["month"] = pElement->value().month();
    json["day"] = pElement->value().day();

    mResult = json;
}
void ElementJsonDumper::visit(ElementTime *pElement)
{
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "time";
    json["hh"] = pElement->value().hour();
    json["mm"] = pElement->value().minute();
    json["ss"] = pElement->value().second();
    json["usems"] = pElement->getUseMs();
    if (pElement->getUseMs()) json["ms"] = pElement->value().msec();


    mResult = json;
}

}
