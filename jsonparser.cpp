#include <QApplication>
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
QJsonObject OpropToJ(Prop prop)
{
    QJsonObject obj;
    obj["name"]=prop.propname;
    obj["label"]=prop.label;
    obj["type"]=prop.typ;
    switch (prop.perm) {
    case OP_RO:
        obj["switch"]="IP_RO";
        break;
    case OP_WO:
        obj["switch"]="IP_WO";
        break;
    case OP_RW:
        obj["switch"]="IP_RW";
        break;
    }
    switch (prop.state) {
    case OPS_IDLE:
        obj["state"]="IPS_IDLE";
        break;
    case OPS_OK:
        obj["state"]="IPS_OK";
        break;
    case OPS_ALERT:
        obj["state"]="IPS_ALERT";
        break;
    case OPS_BUSY:
        obj["state"]="IPS_BUSY";
        break;
    }
    obj["modulename"]=prop.modulename;
    obj["categoryname"]=prop.categoryname;
    obj["groupname"]=prop.groupname;

    QJsonArray details;
    QJsonObject det;
    if (prop.typ==PT_TEXT) {
        for(auto t : prop.t)
        {
            det=OtextToJ(t);
            details.append(det);
        }
        obj["texts"]=details;
    }
    if (prop.typ==PT_NUM) {
        for(auto n : prop.n)
        {
            det=OnumToJ(n);
            details.append(det);
        }
        obj["numbers"]=details;
    }
    if (prop.typ==PT_SWITCH) {
        for(auto s : prop.s)
        {
            det=OswitchToJ(s);
            details.append(det);
        }
        obj["switchs"]=details;
    }
    if (prop.typ==PT_LIGHT) {
        for(auto l : prop.l)
        {
            det=OlightToJ(l);
            details.append(det);
        }
        obj["texts"]=details;
    }
    if (prop.typ==PT_MESSAGE) {
        //TBD
    }
    if (prop.typ==PT_IMAGE) {
        //TBD
    }
    if (prop.typ==PT_GRAPH) {
        //TBD
    }

    return obj;

}
