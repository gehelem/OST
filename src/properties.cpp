#include "properties.h"
#include "module.h"
#include "jsonparser.h"

/* singleton  : getting unique instance if exists, create it if not */
Properties* Properties::getInstance()
{
    if (instance == 0)
    {
        instance = new Properties();
    }

    return instance;
}
/* Null, because instance will be initialized on demand. */
Properties* Properties::instance = 0;


Properties::Properties(QObject *parent)
{
    Q_UNUSED(parent);
}
void    Properties::createModule(QString modulename,  QString modulelabel,int order)
{
    Mod mod;
    mod.modulelabel=modulelabel;
    mod.modulename=modulename;
    mod.order=order;
    modules[modulename]=mod;
}
void    Properties::deleteModule(QString modulename)
{
    modules.remove(modulename);
}
void    Properties::createCateg(QString modulename, QString categname,  QString categlabel,int order)
{
    Categ cat;
    cat.categname=categname;
    cat.modulename=modulename;
    cat.categlabel=categlabel;
    cat.order=order;
    modules[modulename].categs[categname]=cat;

}
void    Properties::deleteCateg(QString modulename, QString categname)
{
    modules[modulename].categs.remove(categname);
}
void    Properties::createGroup(QString modulename, QString categname, QString groupname,  QString grouplabel,int order)
{
    Group gro;
    gro.grouplabel=grouplabel;
    gro.categname=categname;
    gro.modulename=modulename;
    gro.groupname=groupname;
    gro.order=order;
    modules[modulename].groups[groupname]=gro;
}
void    Properties::deleteGroup(QString modulename, QString categname, QString groupname)
{
    Q_UNUSED(categname);
    modules[modulename].groups.remove(groupname);
}
void    Properties::createProp (QString modulename, QString propname, Prop    prop)
{
    modules[modulename].props[propname]=prop;
}

void    Properties::createProp (QString modulename, QString propname, QString label,propType typ,QString categname,QString groupname,OPerm perm,OSRule rule,double timeout,OPState state,QString aux0,QString aux1,int order)
{
    Prop prop;
    prop.modulename     = modulename;
    prop.propname       = propname;
    prop.label          = label;
    prop.typ            = typ;
    prop.categname      = categname;
    prop.groupname      = groupname;
    prop.perm           = perm;
    prop.rule           = rule;
    prop.timeout        = timeout;
    prop.state          = state;
    prop.aux0           = aux0;
    prop.aux1           = aux1;
    prop.order          = order;
    modules[modulename].props[propname]= prop;
}

void    Properties::deleteProp (QString modulename, QString propname)
{
    modules[modulename].props.remove(propname);
}
Prop    Properties::getProp    (QString modulename, QString propname)
{
    return modules[modulename].props[propname];
}
void    Properties::setProp    (QString modulename, QString propname, Prop prop)
{
    modules[modulename].props[propname]=prop;
    emit signalvalueChanged(modules[modulename].props[propname]);
}
void    Properties::emitProp    (QString modulename, QString propname)
{
    emit signalvalueChanged(modules[modulename].props[propname]);
}
void    Properties::appendElt  (QString modulename, QString propname,  QString txtname, QString text     , QString label, QString aux0,QString aux1)
{
    if (modules[modulename].props[propname].typ==PT_TEXT) {
        OText txt;
        txt.name=txtname;
        txt.text=text;
        txt.label=label;
        txt.aux0=aux0;
        txt.aux1=aux1;
        modules[modulename].props[propname].t[txtname]=txt;
    }
    if (modules[modulename].props[propname].typ==PT_MESSAGE) {
        OMessage m;
        m.name=txtname;
        m.text=text;
        m.label=label;
        m.aux0=aux0;
        m.aux1=aux1;
        modules[modulename].props[propname].m[txtname]=m;
    }

}
void    Properties::appendElt  (QString modulename, QString propname,  QString numname, double  num      , QString label, QString aux0,QString aux1)
{
    ONumber nbr;
    nbr.name=numname;
    nbr.value=num;
    nbr.label=label;
    nbr.aux0=aux0;
    nbr.aux1=aux1;
    modules[modulename].props[propname].n[numname]=nbr;
}
void    Properties::appendElt  (QString modulename, QString propname,  QString swtname, OSState swt      , QString label, QString aux0,QString aux1)
{
    OSwitch sw;
    sw.name=swtname;
    sw.s=swt;
    sw.label=label;
    sw.aux0=aux0;
    sw.aux1=aux1;
    modules[modulename].props[propname].s[swtname]=sw;
}
void    Properties::appendElt  (QString modulename, QString propname,  QString lgtname, OPState lgt      , QString label, QString aux0,QString aux1)
{
    OLight l;
    l.name=lgtname;
    l.l=lgt;
    l.label=label;
    l.aux0=aux0;
    l.aux1=aux1;
    modules[modulename].props[propname].l[lgtname]=l;
}
void    Properties::appendElt  (QString modulename,QString propname,  QString imgname, OImgType imt     , QString label, QString aux0,QString aux1,QString url,QString file)
{
    OImage i;
    i.name=imgname;
    i.label=label;
    i.url=url;
    i.f=file;
    i.imgtype=imt;
    i.aux0=aux0;
    i.aux1=aux1;
    modules[modulename].props[propname].i[imgname]=i;

}
void    Properties::appendElt  (QString modulename,QString propname,  QString graname, OGraph graph    )
{
    modules[modulename].props[propname].g[graname]=graph;
}
void    Properties::appendGra  (QString modulename,QString propname,  QString graname, OGraphValue val  )
{
    modules[modulename].props[propname].g[graname].values.append(val);
    emit signalAppendGraph (modules[modulename].props[propname],modules[modulename].props[propname].g[graname],val);
}
void    Properties::resetGra   (QString modulename,QString propname,  QString graname)
{
    modules[modulename].props[propname].g[graname].values.clear();
    emit signalvalueChanged(modules[modulename].props[propname]);
}
void    Properties::deleteElt  (QString modulename, QString propname,  QString eltname)
{
    modules[modulename].props[propname].t.remove(eltname);
    modules[modulename].props[propname].n.remove(eltname);
    modules[modulename].props[propname].s.remove(eltname);
    modules[modulename].props[propname].l.remove(eltname);
    modules[modulename].props[propname].g.remove(eltname);
    modules[modulename].props[propname].i.remove(eltname);
    modules[modulename].props[propname].m.remove(eltname);
}
void    Properties::setElt     (QString modulename, QString propname,  QString txtname, QString text)
{
    if (modules[modulename].props[propname].typ==PT_TEXT)    modules[modulename].props[propname].t[txtname].text=text;
    if (modules[modulename].props[propname].typ==PT_MESSAGE) {
        QString mess = QDateTime::currentDateTime().toString("[yyyyMMdd hh:mm:ss.zzz]") + "-"+ modulename +"-"+text;
        modules[modulename].props[propname].m[txtname].text=mess;
    }
    //emit signalvalueChanged(modules[modulename].props[propname]);
}
void    Properties::setElt     (QString modulename, QString propname,  QString numname, double  num)
{
    modules[modulename].props[propname].n[numname].value=num;
    //emit signalvalueChanged(modules[modulename].props[propname]);
}
void    Properties::setElt     (QString modulename, QString propname,  QString swtname, OSState swt)
{
    modules[modulename].props[propname].s[swtname].s=swt;
    //emit signalvalueChanged(modules[modulename].props[propname]);
}
void    Properties::setElt     (QString modulename, QString propname,  QString lgtname, OPState lgt)
{
    modules[modulename].props[propname].l[lgtname].l=lgt;
    //emit signalvalueChanged(modules[modulename].props[propname]);
}
void    Properties::setElt     (QString modulename,QString propname,  QString imgname, QString url, QString file)
{
    modules[modulename].props[propname].i[imgname].url=url;
    modules[modulename].props[propname].i[imgname].f=file;
    //emit signalvalueChanged(modules[modulename].props[propname]);
}
void    Properties::setElt     (QString modulename,QString propname,  QString graname, OGraph  graph)
{
    modules[modulename].props[propname].g[graname]=graph;
    //emit signalvalueChanged(modules[modulename].props[propname]);
}

QString Properties::getTxt     (QString modulename, QString propname,  QString txtname)
{
    return modules[modulename].props[propname].t[txtname].text;
}
double  Properties::getNum     (QString modulename, QString propname,  QString numname)
{
    return modules[modulename].props[propname].n[numname].value;
}
OSState Properties::getSwt     (QString modulename, QString propname,  QString swtname)
{
    return modules[modulename].props[propname].s[swtname].s;
}
OPState Properties::getLgt     (QString modulename, QString propname,  QString lgtname)
{
    return modules[modulename].props[propname].l[lgtname].l;
}
OImage Properties::getImg    (QString modulename, QString propname,  QString lgtname)
{
    return modules[modulename].props[propname].i[lgtname];
}
OGraph  Properties::getGraph   (QString modulename,QString propname,  QString graname)
{
    return modules[modulename].props[propname].g[graname];
}

/*

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
    obj["categname"]=prop.categname;
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
*/

void    Properties::dumproperties(void)
{
    qDebug() << "nbmodules = " << modules.size();
    for(auto m : modules)
    {
      qDebug() << "##########################" << m.modulename << "---" << m.modulelabel <<"###################################";
      for(auto c : m.categs)
      {
          qDebug() <<"##### categ  ="  << c.categname << c.categlabel << c.modulename;

      }

      for(auto p : m.props)
      {
       qDebug() <<"##### property ="  << p.label << p.typ;
        if (p.typ==PT_TEXT)  {
            qDebug() <<"-- texts";
            for(auto t : p.t)
            {
              qDebug() <<"---- " << t.label << t.text;
            }
        }
       if (p.typ==PT_NUM)  {
           qDebug() <<"-- numbers";
           for(auto n : p.n)
           {
             qDebug() <<"---- " << n.label << n.value;
           }
       }
       if (p.typ==PT_SWITCH)  {
           qDebug() <<"-- switchs";
           for(auto s : p.s)
           {
             qDebug() <<"---- " << s.label << s.s;
           }
       }
      }

    }




}
