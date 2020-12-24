#include <OSTParser.h>
#include <baseclientqt.h>
#include <basedevice.h>
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>

QJsonObject Otext(QString name, QString label,QString text)
{
    QJsonObject obj;
    obj["name"]=name;
    obj["label"]=label;
    obj["text"]=text;
    return obj;
}
QJsonObject Onumber(QString name, QString label,double value)
{
    QJsonObject obj;
    obj["name"]=name;
    obj["label"]=label;
    obj["value"]=value;
    //obj["format"]="";
    //obj["min"]=0;
    //obj["max"]=99;
    //obj["step"]=1;
    return obj;
}
QJsonObject Onumber(QString name, QString label,double value,QString format,double min,double max,double step)
{
    QJsonObject obj;
    obj["name"]=name;
    obj["label"]=label;
    obj["value"]=value;
    obj["format"]=format;
    obj["min"]=min;
    obj["max"]=max;
    obj["step"]=step;
    return obj;
}

QJsonObject Oswitch(QString name, QString label, QString swit)
{
    QJsonObject obj;
    obj["name"]=name;
    obj["label"]=label;
    obj["switch"]=swit;
    return obj;
}
QJsonObject Oswitch(QString name, QString label,ISState swit)
{
    QJsonObject obj;
    obj["name"]=name;
    obj["label"]=label;
    switch (swit) {
    case ISS_ON:
        obj["switch"]="ISS_ON";
        break;
    case ISS_OFF:
        obj["switch"]="ISS_OFF";
        break;
    }
    return obj;
}
QJsonObject Oproperty(QString name, QString label, QString type, QString permission,
                      QString state, QString parentType, QString parentName, QString switchrule, QJsonArray details,
                      QString modulename,QString categoryname,QString groupname)
{
    QJsonObject obj;
    obj["name"]=name;
    obj["label"]=label;
    obj["type"]=type;
    obj["permission"]=permission;
    obj["state"]=state;
    obj["parenttype"]=parentType; // M=module/ C=Category / G=group
    obj["parentname"]=parentName;
    obj["modulename"]=modulename;
    obj["categoryname"]=categoryname;
    obj["groupname"]=groupname;


    if (type=="INDI_TEXT") {
        obj["texts"]=details;
    }
    if (type=="INDI_NUMBER") {
        obj["numbers"]=details;
    }
    if (type=="INDI_SWITCH") {
        obj["rule"]=switchrule;
        obj["switchs"]=details;
    }
    if (type=="INDI_LIGHT") {
        obj["lights"]=details;
    }
    if (type=="INDI_MESSAGE") {
    }
    return obj;

}

QString OGetText(QString name, QJsonArray properties)
{
    foreach (const QJsonValue & val, properties) {
         QJsonObject property = val.toObject();
         if (property["type"].toString()=="INDI_TEXT") {
             QJsonArray texts = property["texts"].toArray();
             foreach (const QJsonValue & txt, texts) {
                 QJsonObject t = txt.toObject();
                 if (t["name"].toString().toStdString().c_str()==name) {
                     return t["text"].toString().toStdString().c_str();
                 }
             }
         }
    }
    return nullptr;
}

double OGetNumber(QString name, QJsonArray properties)
{
    foreach (const QJsonValue & val, properties) {
         QJsonObject property = val.toObject();
         if (property["type"].toString()=="INDI_NUMBER") {
             QJsonArray numbers = property["numbers"].toArray();
             foreach (const QJsonValue & nb, numbers) {
                 QJsonObject n = nb.toObject();
                 if (n["name"].toString().toStdString().c_str()==name) {
                     return n["value"].toDouble();
                 }
             }
         }
    }
    return 0;
}

QJsonArray OSetNumber(std::string name, QJsonArray properties,double value)
{
    QJsonArray props;
    foreach (const QJsonValue & val, properties) {
         QJsonObject property = val.toObject();
         if (property["type"].toString()=="INDI_NUMBER") {
             QJsonArray numbers = property["numbers"].toArray();
             QJsonArray wnumbers = QJsonArray();
             foreach (const QJsonValue & nb, numbers) {
                 QJsonObject n = nb.toObject();
                 if (n["name"].toString().toStdString().c_str()==name) {
                     n["value"]=value;
                 }
                 wnumbers.append(n);
             }
             property["numbers"]=wnumbers;
         }
         props.append(property);
    }
    return props;
}


std::set<std::string> GroupsArray(INDI::BaseDevice *dev) {
    std::vector<INDI::Property *> allprops = *dev->getProperties();
    std::set<std::string> groups;
    for (int j=0;j<allprops.size();j++) {
        //IDLog("--list %s %s %s\n",allprops[j]->getDeviceName(),allprops[j]->getGroupName(),allprops[j]->getName());
        groups.insert(allprops[j]->getGroupName());
    }
    for (std::string gr : groups) {
//        IDLog("idevtojson %s %s\n",dev->getDeviceName(),gr.c_str());
    }
    return groups;
}

QJsonArray IDevToJson(INDI::BaseDevice *dev) {

    std::vector<INDI::Property *> allprops = *dev->getProperties();
    QJsonObject group,prop;
    QJsonArray groups,props;
    for (std::string gr : GroupsArray(dev)) {
//        IDLog("idevtojson %s %s\n",dev->getDeviceName(),gr.c_str());
        group=QJsonObject();
        group["GroupName"]=gr.c_str();
        props=QJsonArray();
        for (int i=0;i<allprops.size();i++) {
            if (strcmp(allprops[i]->getGroupName(), gr.c_str())==0) {
                props.append(IProToJson(allprops[i]));
            }
        }
        group["properties"]=props;
        groups.append(group);
    }

    return groups;
}


QJsonObject IProToJson(INDI::Property *Iprop) {


    QJsonObject pro,propdet;
    QJsonArray propdets;


    pro=QJsonObject();
    pro["name"]=Iprop->getName();
    pro["label"]=Iprop->getLabel();
    pro["module"]="OSTClientPAN";
    pro["categoryname"]=Iprop->getDeviceName();
    pro["groupname"]=Iprop->getGroupName();
    pro["parentType"]="G";
    pro["parentName"]=Iprop->getGroupName();
    pro["modulename"]="OSTClientPAN"; // FIXME

    switch(Iprop->getState()){
    case IPS_IDLE:
        pro["state"]="IPS_IDLE";
        break;
    case IPS_OK:
        pro["state"]="IPS_OK";
        break;
    case IPS_BUSY:
        pro["state"]="IPS_BUSY";
        break;
    case IPS_ALERT:
        pro["state"]="IPS_ALERT";
        break;
    }

    switch(Iprop->getPermission()){
    case IP_RO:
        pro["permission"]="IP_RO";
        break;
    case IP_WO:
        pro["permission"]="IP_WO";
        break;
    case IP_RW:
        pro["permission"]="IP_RW";
        break;
    }

    switch (Iprop->getType()) {

    case INDI_NUMBER:
        pro["type"]="INDI_NUMBER";
        propdets=QJsonArray();
        for (int k=0;k<Iprop->getNumber()->nnp;k++) {
            propdet=QJsonObject();
            propdet["value"]=Iprop->getNumber()->np[k].value;
            propdet["name"]=Iprop->getNumber()->np[k].name;
            propdet["label"]=Iprop->getNumber()->np[k].label;
            propdet["format"]=Iprop->getNumber()->np[k].format;
            propdet["min"]=Iprop->getNumber()->np[k].min;
            propdet["max"]=Iprop->getNumber()->np[k].max;
            propdet["step"]=Iprop->getNumber()->np[k].step;
            //propdet["aux0"]=Iprop->getNumber()->np[k].aux0;
            //propdet["aux1"]=Iprop->getNumber()->np[k].aux1;
            propdets.append(propdet);
        }
        pro["numbers"]=propdets;
        break;

    case INDI_TEXT:
        pro["type"]="INDI_TEXT";
        propdets=QJsonArray();
        for (int k=0;k<Iprop->getText()->ntp;k++) {
            propdet=QJsonObject();
            propdet["name"]=Iprop->getText()->tp[k].name;
            propdet["label"]=Iprop->getText()->tp[k].label;
            propdet["text"]=Iprop->getText()->tp[k].text;
            //propdet["aux0"]=Iprop->getText()->tp[k].aux0;
            //propdet["aux1"]=Iprop->getText()->tp[k].aux1;
            propdets.append(propdet);
        }
        pro["texts"]=propdets;
        break;

    case INDI_LIGHT:
        pro["type"]="INDI_LIGHT";
        propdets=QJsonArray();
        for (int k=0;k<Iprop->getLight()->nlp;k++) {
            propdet=QJsonObject();
            propdet["name"]=Iprop->getLight()->lp[k].name;
            propdet["label"]=Iprop->getLight()->lp[k].label;
            //propdet["aux"]=Iprop->getLight()->lp[k].aux;
            switch (Iprop->getLight()->lp[k].s) {
            case IPS_IDLE:
                propdet["light"]="IPS_IDLE";
                break;
            case IPS_OK:
                propdet["light"]="IPS_OK";
                break;
            case IPS_BUSY:
                propdet["light"]="IPS_BUSY";
                break;
            case IPS_ALERT:
                propdet["light"]="IPS_ALERT";
                break;
            }
            propdets.append(propdet);
        }
        pro["lights"]=propdets;
        break;

    case INDI_SWITCH:
        pro["type"]="INDI_SWITCH";
        switch (Iprop->getSwitch()->r){
        case ISR_1OFMANY:
            pro["rule"] ="ISR_1OFMANY";
            break;
        case ISR_ATMOST1:
            pro["rule"] ="ISR_ATMOST1";
            break;
        case ISR_NOFMANY:
            pro["rule"] ="ISR_NOFMANY";
            break;
        }
        propdets=QJsonArray();
        for (int k=0;k<Iprop->getSwitch()->nsp;k++) {
            propdet=QJsonObject();
            propdet["name"]=Iprop->getSwitch()->sp[k].name;
            propdet["label"]=Iprop->getSwitch()->sp[k].label;
            //propdet["aux"]=Iprop->getSwitch()->sp[k].aux;
            switch (Iprop->getSwitch()->sp[k].s) {
            case ISS_ON:
                propdet["switch"]="ISS_ON";
                break;
            case ISS_OFF:
                propdet["switch"]="ISS_OFF";
                break;
            }
            propdets.append(propdet);
        }
        pro["switchs"]=propdets;
        break;
    }
    return pro;
}
