#include <QCoreApplication>
#include "jsonparser.h"


QJsonObject OtextToJ(OText txt)
{
    QJsonObject obj;
    obj["name"]=txt.name;
    obj["label"]=txt.label;
    obj["text"]=txt.text;
    obj["aux0"]=txt.aux0;
    obj["aux1"]=txt.aux1;
    return obj;
}

QJsonObject OnumToJ(ONumber num)
{
    QJsonObject obj;
    obj["name"]=num.name;
    obj["label"]=num.label;
    obj["value"]=num.value;
    obj["format"]=num.format;
    obj["min"]=num.min;
    obj["max"]=num.max;
    obj["step"]=num.step;
    obj["aux0"]=num.aux0;
    obj["aux1"]=num.aux1;
    return obj;
}

QJsonObject OswitchToJ(OSwitch swt)
{
    QJsonObject obj;
    obj["name"]=swt.name;
    obj["label"]=swt.label;
    switch (swt.s) {
    case OSS_ON:
        obj["switch"]="ISS_ON";
        break;
    case OSS_OFF:
        obj["switch"]="ISS_OFF";
        break;
    }
    obj["aux0"]=swt.aux0;
    obj["aux1"]=swt.aux1;
    return obj;
}
QJsonObject OimageToJ(OImage img)
{
    QJsonObject obj;
    obj["name"]=img.name;
    obj["label"]=img.label;
    obj["f"]=img.f;
    obj["url"]=img.url;
    switch (img.imgtype) {
    case IM_FULL:
        obj["imgtype"]="IM_FULL";
        break;
    case IM_MINI:
        obj["imgtype"]="IM_MINI";
        break;
    case IM_OVERLAY:
        obj["imgtype"]="IM_OVERLAY";
        break;    }
    obj["aux0"]=img.aux0;
    obj["aux1"]=img.aux1;
    return obj;
}
QJsonObject OlightToJ (OLight  lgt)
{
    QJsonObject obj;
    obj["name"]=lgt.name;
    obj["label"]=lgt.label;
    switch (lgt.l) {
    case OPS_IDLE:
        obj["light"]="IPS_IDLE";
        break;
    case OPS_OK:
        obj["light"]="IPS_OK";
        break;
    case OPS_ALERT:
        obj["light"]="IPS_ALERT";
        break;
    case OPS_BUSY:
        obj["light"]="IPS_BUSY";
        break;
    }
    obj["aux0"]=lgt.aux0;
    obj["aux1"]=lgt.aux1;
    return obj;

}
QJsonObject OgraphToJ (OGraph  gra)
{
    QJsonObject obj,val;
    QJsonArray vals;
    obj["name"]=gra.name;
    obj["label"]=gra.label;
    obj["gtype"]=gra.gtype;
    obj["aux0"]=gra.aux0;
    obj["aux1"]=gra.aux1;
    obj["V0label"]=gra.V0label;
    obj["V1label"]=gra.V1label;
    obj["V2label"]=gra.V2label;
    obj["V3label"]=gra.V3label;
    obj["V4label"]=gra.V4label;

    for (int i=0;i<gra.values.size();i++) {
        val["v0"]=gra.values[i].v0;
        val["v1"]=gra.values[i].v1;
        val["v2"]=gra.values[i].v2;
        val["v3"]=gra.values[i].v3;
        val["v4"]=gra.values[i].v4;
        vals.append(val);
    }
    obj["values"]=vals;

    return obj;

    //QVector<OGraphValue> values;

}
QJsonObject OmessToJ(OMessage m)
{
    QJsonObject obj;
    obj["name"]=m.name;
    obj["label"]=m.label;
    obj["text"]=m.text;
    obj["aux0"]=m.aux0;
    obj["aux1"]=m.aux1;
    return obj;
}

QJsonObject OgroupToJ(QString groupname)
{
    Q_UNUSED(groupname);
    QJsonObject obj;
    QJsonArray groups;

    return obj;
}
QJsonObject OcategToJ(QString categname)
{
    Q_UNUSED(categname);
    QJsonObject obj;
    return obj;
}


