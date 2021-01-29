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
    obj["propname"]=prop.propname;
    obj["label"]=prop.label;
    switch (prop.typ) {
    case PT_NUM:
        obj["type"]="INDI_NUMBER";
        break;
    case PT_TEXT:
        obj["type"]="INDI_TEXT";
        break;
    case PT_SWITCH:
        obj["type"]="INDI_SWITCH";
        break;
    case PT_LIGHT:
        obj["type"]="INDI_LIGHT";
        break;
    case PT_GRAPH:
        obj["type"]="INDI_GRAPH";
        break;
    case PT_MESSAGE:
        obj["type"]="INDI_MESSAGE";
        break;
    case PT_IMAGE:
        obj["type"]="INDI_IMAGE";
        break;
    }
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
    obj["categname"]=prop.categname;
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

QJsonObject OgroupToJ(QString groupname)
{
    QJsonObject obj;
    QJsonArray groups;

    return obj;
}
QJsonObject OcategToJ(QString categname)
{
    QJsonObject obj;
    return obj;
}
QJsonObject OmodToJ(Mod mod)
{
    QJsonObject obj;
    obj["modulename"]=mod.modulename;
    obj["modulelabel"]=mod.modulelabel;

    QJsonArray props0;
    for(auto p : mod.props){
        if (p.modulename==mod.modulename && p.categname=="" && p.groupname =="" ) {
            QJsonObject prop0 =OpropToJ(p);
            props0.append(prop0);
        }
    }
    if (props0.size() >0) {
        obj["properties"]=props0;
    }

    QJsonArray categs;
    for(auto c : mod.categs){
        if (c.modulename==mod.modulename ) {
            QJsonObject categ;
            categ["modulename"]=c.modulename;
            categ["categname"]=c.categname;

            /* ajout des propriétés de la catégorie */
            QJsonArray props1;
            for(auto p : mod.props){
                if (p.modulename==mod.modulename && p.categname==c.categname && p.groupname =="" ) {
                    QJsonObject prop1 =OpropToJ(p);
                    props1.append(prop1);
                }
            }
            if (props1.size() >0) {
                categ["properties"]=props1;
            }
            /* fin ajout des propriétés de la catégorie */

            /* ajout des groupes de la catégorie */
            QJsonArray groups1;
            for(auto g : mod.groups){
                if (g.modulename==mod.modulename && g.categname==c.categname && g.groupname !="" ) {
                    QJsonObject grp;
                    grp["modulename"]=g.modulename;
                    grp["groupname"]=g.groupname;
                    grp["categname"]=g.categname;
                    grp["grouplabel"]=g.grouplabel;

                    QJsonArray props2;
                    for(auto p2 : mod.props){
                        if (p2.modulename==mod.modulename && p2.categname==c.categname && p2.groupname == g.groupname ) {
                            QJsonObject prop2 =OpropToJ(p2);
                            props2.append(prop2);
                        }
                    }
                    if (props2.size() >0) {
                        grp["properties"]=props2;
                    }
                    groups1.append(grp);
                }

                if (groups1.size()>0) {
                    categ["groups"]=groups1;
                }

            }
            /* fin ajout des groupes de la catégorie */

            categs.append(categ);


            if (categs.size()>0) {
                obj["categories"]=categs;
            }
        }
    } // boucle categs

    return obj;
}

