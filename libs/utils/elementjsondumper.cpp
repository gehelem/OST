#include "elementjsondumper.h"

namespace  OST
{
//enum class EvType
//{
//    aa,        /*!< dump all data */
//    zz = 0,    /*!< no dump       */
//    ee,        /*!< set one element value */
//    ea,        /*!< set all elements values (prop level) */
//    ev,        /*!< set one element value/min/max/format */
//    ps,        /*!< only property state */
//    gc,        /*!< grid new line */
//    gu,        /*!< grid update line  */
//    gd,        /*!< grid delete line  */
//    gr,        /*!< grid reset */
//};
QJsonObject ElementJsonDumper::dumpElementCommons(ElementBase *pElement)
{
    QJsonObject json;
    if (mEvent == EvType::ee || mEvent == EvType::ea || mEvent == EvType::ev) return json;

    json["label"] = pElement->label();
    json["order"] = pElement->order();
    json["hint"] = pElement->hint();
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
void ElementJsonDumper::visit(ElementBool *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);
    QJsonObject json = dumpElementCommons(pElement);
    json["value"] = pElement->value();
    mResult = json;
    if (mEvent == EvType::ee || mEvent == EvType::ea || mEvent == EvType::ev) return;

    json["type"] = "bool";
    if (pElement->getPreIcon() != "") json["preicon"] = pElement->getPreIcon();
    if (pElement->getPostIcon() != "") json["posticon"] = pElement->getPostIcon();
    mResult = json;
}
void ElementJsonDumper::visit(ElementInt *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);
    QJsonObject json = dumpElementCommons(pElement);
    json["value"] = pElement->value();
    mResult = json;
    if (mEvent == EvType::ee || mEvent == EvType::ea) return;

    json["type"] = "int";
    json["value"] = qlonglong(pElement->value());
    json["min"] = qlonglong(pElement->min());
    json["max"] = qlonglong(pElement->max());
    json["step"] = qlonglong(pElement->step());
    json["format"] = pElement->format();
    json["slider"] = pElement->slider();
    mResult = json;
    if (mEvent == EvType::ev) return;

    if (pElement->getPreIcon() != "") json["preicon"] = pElement->getPreIcon();
    if (pElement->getPostIcon() != "") json["posticon"] = pElement->getPostIcon();
    if (pElement->getGlobalLov() != "")
    {
        json["globallov"] = pElement->getGlobalLov();
        json["lovScope"] = (pElement->getLovScope() == LovScope::Controller) ? "controller" : "module";
        json["lovConstrained"] = pElement->getLovConstrained();
    }
    else
    {
        if (pElement->getLov().size() > 0)
        {
            QJsonObject lines = QJsonObject();
            for(const long &key : pElement->getLov().keys()) {
                lines[QString::number(key)] = pElement->getLov()[key];
            }
            json["listOfValues"] = lines;
        }

    }
    mResult = json;
}
void ElementJsonDumper::visit(ElementFloat *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);
    QJsonObject json = dumpElementCommons(pElement);
    json["value"] = pElement->value();
    mResult = json;
    if (mEvent == EvType::ee || mEvent == EvType::ea) return;

    json["type"] = "float";
    json["min"] = pElement->min();
    json["max"] = pElement->max();
    json["step"] = pElement->step();
    json["format"] = pElement->format();
    json["slider"] = pElement->slider();
    mResult = json;
    if (mEvent == EvType::ev) return;

    if (pElement->getPreIcon() != "") json["preicon"] = pElement->getPreIcon();
    if (pElement->getPostIcon() != "") json["posticon"] = pElement->getPostIcon();
    if (pElement->getGlobalLov() != "")
    {
        json["globallov"] = pElement->getGlobalLov();
        json["lovScope"] = (pElement->getLovScope() == LovScope::Controller) ? "controller" : "module";
        json["lovConstrained"] = pElement->getLovConstrained();
    }
    else
    {
        if (!pElement->getLov().isEmpty())
        {
            QJsonObject lines = QJsonObject();
            for(const double &key : pElement->getLov().keys()) {
                lines[QString::number(key)] = pElement->getLov()[key];
            }
            json["listOfValues"] = lines;
        }
    }
    mResult = json;
}
void ElementJsonDumper::visit(ElementString *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);
    QJsonObject json = dumpElementCommons(pElement);
    json["value"] = pElement->value();
    mResult = json;
    if (mEvent == EvType::ee || mEvent == EvType::ea || mEvent == EvType::ev) return;

    json["type"] = "string";
    if (pElement->getPreIcon() != "") json["preicon"] = pElement->getPreIcon();
    if (pElement->getPostIcon() != "") json["posticon"] = pElement->getPostIcon();
    if (pElement->getGlobalLov() != "")
    {
        json["globallov"] = pElement->getGlobalLov();
        json["lovScope"] = (pElement->getLovScope() == LovScope::Controller) ? "controller" : "module";
        json["lovConstrained"] = pElement->getLovConstrained();
    }
    else
    {
        if (pElement->getLov().size() > 0)
        {
            QJsonObject lines = QJsonObject();
            for(const QString &key : pElement->getLov().keys())            {
                lines[key] = pElement->getLov()[key];
            }
            json["listOfValues"] = lines;
        }
    }
    mResult = json;
}
void ElementJsonDumper::visit(ElementLight *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);
    QJsonObject json = dumpElementCommons(pElement);
    json["value"] = pElement->value();
    mResult = json;
    if (mEvent == EvType::ee || mEvent == EvType::ea || mEvent == EvType::ev) return;

    json["type"] = "light";
    mResult = json;
}
void ElementJsonDumper::visit(ElementImg *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);

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
    imgdata["hfravgdev"] = pElement->value().HFRavgDev;
    imgdata["stars"] = pElement->value().starsCount;
    imgdata["issolved"] = pElement->value().isSolved;
    imgdata["solverra"] = pElement->value().solverRA;
    imgdata["solverde"] = pElement->value().solverDE;
    imgdata["solverorientation"] = pElement->value().solverOrientation;


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
    json["value"] = imgdata;
    mResult = json;
    if (mEvent == EvType::ee || mEvent == EvType::ea || mEvent == EvType::ev) return;

    json["type"] = "img";

    mResult = json;
}
void ElementJsonDumper::visit(ElementVideo *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);
    QJsonObject json = dumpElementCommons(pElement);
    json["type"] = "video";
    json["url"] = pElement->value().url;
    mResult = json;
}
void ElementJsonDumper::visit(ElementPrg *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);
    QJsonObject json = dumpElementCommons(pElement);
    QJsonObject o;
    o["dynlabel"] = pElement->value().dynlabel;
    o["value"] = pElement->value().value;
    json["value"] = o;
    mResult = json;
    if (mEvent == EvType::ee || mEvent == EvType::ea || mEvent == EvType::ev) return;

    json["type"] = "prg";
    if (pElement->prgType() == bar) json["prgtype"] = "bar";
    if (pElement->prgType() == spinner) json["prgtype"] = "spinner";
    mResult = json;
}
void ElementJsonDumper::visit(ElementDate *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);
    QJsonObject json = dumpElementCommons(pElement);
    QJsonObject v;
    v["year"] = pElement->value().year();
    v["month"] = pElement->value().month();
    v["day"] = pElement->value().day();
    json["value"] = v;
    mResult = json;
    if (mEvent == EvType::ee || mEvent == EvType::ea || mEvent == EvType::ev) return;

    json["type"] = "date";
    mResult = json;
}
void ElementJsonDumper::visit(ElementTime *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);
    QJsonObject json = dumpElementCommons(pElement);
    QJsonObject v;
    v["hh"] = pElement->value().hour();
    v["mm"] = pElement->value().minute();
    v["ss"] = pElement->value().second();
    if (pElement->getUseMs()) v["ms"] = pElement->value().msec();
    json["value"] = v;
    mResult = json;
    if (mEvent == EvType::ee || mEvent == EvType::ea || mEvent == EvType::ev) return;

    json["type"] = "time";
    json["usems"] = pElement->getUseMs();
    mResult = json;

}
void ElementJsonDumper::visit(ElementDateTime *pElement, QVariantMap &data, bool &emitEvent)
{
    Q_UNUSED(data);
    Q_UNUSED(emitEvent);
    QJsonObject json = dumpElementCommons(pElement);
    QJsonObject v;
    v["year"] = pElement->value().date().year();
    v["month"] = pElement->value().date().month();
    v["day"] = pElement->value().date().day();
    v["hh"] = pElement->value().time().hour();
    v["mm"] = pElement->value().time().minute();
    v["ss"] = pElement->value().time().second();
    v["ms"] = pElement->value().time().msec();
    json["value"] = v;
    mResult = json;
    if (mEvent == EvType::ee || mEvent == EvType::ea || mEvent == EvType::ev) return;

    json["type"] = "datetime";
    mResult = json;
}

}
