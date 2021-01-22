#include "properties.h"


OSTProperties::OSTProperties(QObject *parent)
{
}

void    OSTProperties::createElem(elem elt)
{
    elems.append(elt);
    emit signalElemCreated(elt);

}
void    OSTProperties::createGroup(QString name, QString label, QString categ)
{
    group gro;
    gro.name=name;
    gro.label=label;
    gro.module=module;
    gro.category=categ;
    groups.append(gro);
}
void    OSTProperties::createCategory(QString name, QString label)
{
    categ cat;
    cat.name=name;
    cat.label=label;
    cat.module=module;
    categs.append(cat);
}

void    OSTProperties::createProp(QString name, QString label, QString rule,  QString category, QString permission, QString state, QString group)
{
    pro prop;
    prop.name=name;
    prop.label=label;
    prop.rule=rule;
    prop.permission=permission;
    prop.state=state;
    prop.module=module;
    prop.category = category;
    prop.group =group;
    props.append(prop);

}
void    OSTProperties::createText(QString name,QString label,QString text,QString prop)
{
    elem elt;
    elt.name=name;
    elt.label=label;
    elt.text=text;
    elt.type=ET_TEXT;
    elt.module=module;
    elt.property=prop;
    createElem(elt);
}
void    OSTProperties::createNum(QString name,QString label,double num,QString prop)
{
    elem elt;
    elt.name=name;
    elt.label=label;
    elt.num=num;
    elt.type=ET_NUM;
    elt.module=module;
    elt.property=prop;
    elt.property=prop;
    createElem(elt);
}
void    OSTProperties::createBool(QString name, QString label, bool sw,QString prop)
{
    elem elt;
    elt.name=name;
    elt.label=label;
    elt.sw=sw;
    elt.type=ET_BOOL;
    elt.module=module;
    elt.property=prop;
    createElem(elt);
}
void    OSTProperties::createBTN(QString name, QString label,QString prop)
{
    elem elt;
    elt.name=name;
    elt.label=label;
    elt.type=ET_BTN;
    elt.module=module;
    elt.property=prop;
    createElem(elt);
}

void    OSTProperties::deleteProp(QString name)
{
    for (int i=0;i<elems.count();i++)
    {
        if (elems[i].name==name)
        {
            emit signalElemDeleted(elems[i]);
            elems.remove(i);
        }
    }
}
elem    OSTProperties::getElem(QString name)
{
    for (int i=0;i<elems.count();i++)
    {
        if (elems[i].name==name)
        {
            return elems[i];
        }
    }
    return elem();
}
QString OSTProperties::getText(QString name)
{
    for (int i=0;i<elems.count();i++)
    {
        if (elems[i].name==name)
        {
            return elems[i].text;
        }
    }
    return QString();

}
double  OSTProperties::getNum(QString name)
{
    for (int i=0;i<elems.count();i++)
    {
        if (elems[i].name==name)
        {
            return elems[i].num;
        }
    }
    return double();
}
bool    OSTProperties::getBool(QString name)
{
    for (int i=0;i<elems.count();i++)
    {
        if (elems[i].name==name)
        {
            return elems[i].sw;
        }
    }
    return bool();

}
void    OSTProperties::setText(QString name,QString text)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].name==name)&&(elems[i].text!=text))
        {
            elems[i].text=text;
            emit signalvalueChanged(elems[i]);
        }
    }
}
void    OSTProperties::setNum(QString name,double num)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].name==name)&&(elems[i].num!=num))
        {
            elems[i].num=num;
            emit signalvalueChanged(elems[i]);
        }
    }

}
void    OSTProperties::setBool(QString name,bool sw)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].name==name)&&(elems[i].sw!=sw))
        {
            elems[i].sw=sw;
            emit signalvalueChanged(elems[i]);
        }
    }

}

QJsonObject OSTProperties::getJsonProp(QString propname)
{
    bool found= false;
    pro prop;
    for (int i=0;i<props.count();i++) {
        if (props[i].name==propname) {
            found=true;
            prop=props[i];
        }
    }
    if (!found) return QJsonObject();

    QJsonObject mess,obj,det;
    QJsonArray texts,nums,swis;
    obj["propertyname"]=prop.name;
    obj["label"]=prop.label;
    obj["permission"]=prop.permission;
    obj["state"]=prop.state;
    obj["rule"]=prop.rule;
    obj["parenttype"]=""; // M=module/ C=Category / G=group
    obj["parentname"]="";
    obj["modulename"]=module;
    obj["categoryname"]=prop.category;
    obj["groupname"]=prop.group;

    elem elt;
    for (int i=0;i<elems.count();i++){
        if (elems[i].property==prop.name) {
            elt=elems[i];
            if (elt.type==ET_TEXT) {
                obj["type"]="INDI_TEXT";
                det["label"]=elt.label;
                det["textname"]=elt.name;
                det["text"]=elt.text;
                texts.append(det);
            }
            if (elt.type==ET_NUM) {
                obj["type"]="INDI_NUMBER";
                det["label"]=elt.label;
                det["numbername"]=elt.name;
                det["value"]=elt.num;
                det["format"]="";
                det["min"]=0;
                det["max"]=0;
                det["step"]=0;
                nums.append(det);
            }
            if (elt.type==ET_BOOL) {
                obj["type"]="INDI_SWITCH";
                det["label"]=elt.label;
                det["switchname"]=elt.name;
                if (elt.sw) det["switch"]="ISS_ON";
                if (!elt.sw) det["switch"]="ISS_OFF";
                swis.append(det);
            }

        }

    }
    if (obj["type"]=="INDI_TEXT") obj["texts"]=texts;
    if (obj["type"]=="INDI_NUMBER") obj["texts"]=nums;
    if (obj["type"]=="INDI_SWITCH") obj["texts"]=swis;

    return obj;
}
