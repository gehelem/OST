#include "properties.h"
#include "basemodule.h"
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
void    Properties::createDevcat(QString modulename, QString categname,  QString categlabel,int order)
{
    Devcat cat;
    cat.categname=categname;
    cat.modulename=modulename;
    cat.categlabel=categlabel;
    cat.order=order;
    modules[modulename].devcats[categname]=cat;

}
void    Properties::deleteDevcat(QString modulename, QString categname)
{
    modules[modulename].devcats.remove(categname);
}
void    Properties::createGroup(QString modulename, QString categname, QString groupname,  QString grouplabel,int order)
{
    if (modules[modulename].groups.contains(groupname)) //avoids group duplication for indi properties
    {
        BOOST_LOG_TRIVIAL(debug) << "Group duplicate " << modulename.toStdString() << "-" << modules[modulename].groups.value(groupname).groupname.toStdString();
    }
    else
    {
        Group gro;
        gro.grouplabel=grouplabel;
        gro.categname=categname;
        gro.modulename=modulename;
        gro.groupname=groupname;
        gro.order=order;
        modules[modulename].groups.insert(groupname,gro);
    }
}
void    Properties::deleteGroup(QString modulename, QString categname, QString groupname)
{
    Q_UNUSED(categname);
    emit signalGroupDeleted(modules[modulename].groups[groupname]);
    modules[modulename].groups.remove(groupname);

}
void    Properties::createProp (QString modulename, QString propname, Prop    prop)
{
    modules[modulename].props[propname]=prop;
    emit signalPropCreated(modules[modulename].props[propname]);
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
    emit signalPropCreated(modules[modulename].props[propname]);

}

void    Properties::deleteProp (QString modulename, QString propname)
{
    QString groupname = modules[modulename].props[propname].groupname;
    QString categname = modules[modulename].props[propname].categname;

    modules[modulename].props.remove(propname);
    emit signalPropDeleted(modules[modulename].props[propname]);

    //look for empty groups
    bool delgroup = true;

    for(auto p : modules[modulename].props)
    {
        if (p.groupname==groupname) delgroup=false;
    }
    if (delgroup) deleteGroup(modulename,categname,groupname);

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


void    Properties::dumproperties(void)
{
    qDebug() << "nbmodules = " << modules.size();
    for(auto m : modules)
    {
      qDebug() << "##########################" << m.modulename << "---" << m.modulelabel <<"###################################";
      for(auto c : m.devcats)
      {
          qDebug() <<"##### categ  ="  << c.modulename << c.categname << c.categlabel;

      }
      for(auto g : m.groups)
      {
          qDebug() <<"##### group  =" << g.modulename << g.categname << g.groupname  << g.grouplabel ;

      }

      for(auto p : m.props)
      {
       qDebug() <<"##### property ="  << p.modulename << p.categname << p.groupname << p.propname << p.label << p.typ ;
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
