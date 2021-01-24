#include "properties.h"
#include "module.h"

OSTProperties::OSTProperties(QObject *parent)
{
}

void    OSTProperties::createElem(elem elt)
{
    elems.append(elt);
    emit signalElemCreated(elt);

}
void    OSTProperties::createGroup(QString modulename,QString groupname, QString grouplabel, QString categname)
{
    group gro;
    gro.groupname=groupname;
    gro.grouplabel=grouplabel;
    gro.modulename=modulename;
    gro.categname=categname;
    groups.append(gro);
}
void    OSTProperties::createCateg(QString modulename,QString categname, QString categlabel)
{
    categ cat;
    cat.categname=categname;
    cat.categlabel=categlabel;
    cat.modulename=modulename;
    categs.append(cat);
}

void    OSTProperties::createProp(QString modulename, QString propname, QString proplabel, QString groupname, QString categname, QString rule,  QString permission, QString state)
{
    pro prop;
    prop.propname=propname;
    prop.proplabel=proplabel;
    prop.modulename=modulename;
    prop.categname = categname;
    prop.groupname =groupname;
    prop.rule=rule;
    prop.permission=permission;
    prop.state=state;
    props.append(prop);

}
void    OSTProperties::createText(QString modulename,QString elemname,QString elemlabel,QString propname, QString groupname, QString categname,QString text)
{
    elem elt;
    elt.elemname=elemname;
    elt.elemlabel=elemlabel;
    elt.propname=propname;
    elt.groupname=groupname;
    elt.categname=categname;
    elt.modulename=modulename;
    elt.text=text;
    elt.type=ET_TEXT;
    createElem(elt);
}
void    OSTProperties::createNum(QString modulename,QString elemname,QString elemlabel,QString propname, QString groupname, QString categname,double num)
{
    elem elt;
    elt.elemname=elemname;
    elt.elemlabel=elemlabel;
    elt.propname=propname;
    elt.groupname=groupname;
    elt.categname=categname;
    elt.modulename=modulename;
    elt.num=num;
    elt.type=ET_NUM;
    createElem(elt);
}
void    OSTProperties::createBool(QString modulename,QString elemname,QString elemlabel,QString propname, QString groupname, QString categname,bool sw)
{
    elem elt;
    elt.elemname=elemname;
    elt.elemlabel=elemlabel;
    elt.propname=propname;
    elt.groupname=groupname;
    elt.categname=categname;
    elt.modulename=modulename;
    elt.sw=sw;
    elt.type=ET_BOOL;
    createElem(elt);
}
void    OSTProperties::createBTN(QString modulename, QString elemname,QString elemlabel,QString propname, QString groupname, QString categname)
{
    elem elt;
    elt.elemname=elemname;
    elt.elemlabel=elemlabel;
    elt.propname=propname;
    elt.groupname=groupname;
    elt.categname=categname;
    elt.modulename=modulename;
    elt.type=ET_BTN;
    createElem(elt);
}

void    OSTProperties::deleteProp(QString modulename,QString propname)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].propname==propname)&&(elems[i].modulename==modulename))
        {
            emit signalElemDeleted(elems[i]);
            elems.remove(i);
        }
    }
    for (int i=0;i<props.count();i++)
    {
        if ((props[i].propname==propname)&&(elems[i].modulename==modulename))
        {
            emit signalPropDeleted(props[i]);
            props.remove(i);
        }
    }
}
elem    OSTProperties::getElem(QString modulename,QString elemname)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].elemname==elemname)&&(elems[i].modulename==modulename))
        {
            return elems[i];
        }
    }
    return elem();
}
QString OSTProperties::getText(QString modulename,QString elemname)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].elemname==elemname)&&(elems[i].modulename==modulename))
        {
            return elems[i].text;
        }
    }
    return QString();

}
double  OSTProperties::getNum(QString modulename,QString elemname)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].elemname==elemname)&&(elems[i].modulename==modulename))
        {
            return elems[i].num;
        }
    }
    return double();
}
bool    OSTProperties::getBool(QString modulename,QString elemname)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].elemname==elemname)&&(elems[i].modulename==modulename))
        {
            return elems[i].sw;
        }
    }
    return bool();

}
void    OSTProperties::setText(QString modulename,QString elemname,QString text)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].elemname==elemname)&&(elems[i].modulename==modulename)&&(elems[i].text!=text))
        {
            elems[i].text=text;
            emit signalvalueChanged(elems[i]);
        }
    }
}
void    OSTProperties::setNum(QString modulename,QString elemname,double num)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].elemname==elemname)&&(elems[i].modulename==modulename)&&(elems[i].num!=num))
        {
            elems[i].num=num;
            emit signalvalueChanged(elems[i]);
        }
    }

}
void    OSTProperties::setBool(QString modulename,QString elemname, bool sw)
{
    for (int i=0;i<elems.count();i++)
    {
        if ((elems[i].elemname==elemname)&&(elems[i].modulename==modulename)&&(elems[i].sw!=sw))
        {
            elems[i].sw=sw;
            emit signalvalueChanged(elems[i]);
        }
    }

}

QJsonObject OSTProperties::getJsonProp(QString propname,QString modulename)
{
    bool found= false;
    pro prop;
    for (int i=0;i<props.count();i++) {
        if ((props[i].propname==propname)&&(props[i].modulename==modulename) ){
            found=true;
            prop=props[i];
        }
    }
    if (!found) return QJsonObject();

    QJsonObject mess,obj,det;
    QJsonArray texts,nums,swis;
    obj["propertyname"]=prop.propname;
    obj["label"]=prop.proplabel;
    obj["permission"]=prop.permission;
    obj["state"]=prop.state;
    obj["rule"]=prop.rule;
    //obj["parenttype"]=""; // M=module/ C=Category / G=group
    //obj["parentname"]="";
    obj["modulename"]=modulename;
    obj["categoryname"]=prop.categname;
    obj["groupname"]=prop.groupname;

    elem elt;
    for (int i=0;i<elems.count();i++){
        if (elems[i].propname==prop.propname) {
            elt=elems[i];
            if (elt.type==ET_TEXT) {
                obj["type"]="INDI_TEXT";
                det["label"]=elt.elemlabel;
                det["textname"]=elt.elemname;
                det["text"]=elt.text;
                texts.append(det);
            }
            if (elt.type==ET_NUM) {
                obj["type"]="INDI_NUMBER";
                det["label"]=elt.elemlabel;
                det["numbername"]=elt.elemname;
                det["value"]=elt.num;
                det["format"]="";
                det["min"]=0;
                det["max"]=0;
                det["step"]=0;
                nums.append(det);
            }
            if (elt.type==ET_BOOL) {
                obj["type"]="INDI_SWITCH";
                det["label"]=elt.elemlabel;
                det["switchname"]=elt.elemname;
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
