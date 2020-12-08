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


std::set<std::string> GroupsArray(INDI::BaseDevice *dev) {
    std::vector<INDI::Property *> allprops = *dev->getProperties();
    std::set<std::string> groups;
    for (int j=0;j<allprops.size();j++) {
        groups.insert(allprops[j]->getGroupName());
    }
    return groups;
}

QJsonArray IDevToJson(INDI::BaseDevice *dev) {

    std::vector<INDI::Property *> allprops = *dev->getProperties();
    QJsonObject group,prop;
    QJsonArray groups,props;
    for (std::string gr : GroupsArray(dev)) {
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
    pro["Name"]=Iprop->getName();
    pro["Label"]=Iprop->getLabel();
    //pro["GroupName"]=Iprop->getGroupName();
    switch(Iprop->getState()){
    case IPS_IDLE:
        pro["State"]="IPS_IDLE";
        break;
    case IPS_OK:
        pro["State"]="IPS_OK";
        break;
    case IPS_BUSY:
        pro["State"]="IPS_BUSY";
        break;
    case IPS_ALERT:
        pro["State"]="IPS_ALERT";
        break;
    }

    switch(Iprop->getPermission()){
    case IP_RO:
        pro["Permission"]="IP_RO";
        break;
    case IP_WO:
        pro["Permission"]="IP_WO";
        break;
    case IP_RW:
        pro["Permission"]="IP_RW";
        break;
    }

    switch (Iprop->getType()) {

    case INDI_NUMBER:
        pro["Type"]="INDI_NUMBER";
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
        pro["Numbers"]=propdets;
        break;

    case INDI_TEXT:
        pro["Type"]="INDI_TEXT";
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
        pro["Texts"]=propdets;
        break;

    case INDI_LIGHT:
        pro["Type"]="INDI_LIGHT";
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
        pro["Lights"]=propdets;
        break;

    case INDI_SWITCH:
        pro["Type"]="INDI_SWITCH";
        switch (Iprop->getSwitch()->r){
        case ISR_1OFMANY:
            pro["Rule"] ="ISR_1OFMANY";
            break;
        case ISR_ATMOST1:
            pro["Rule"] ="ISR_ATMOST1";
            break;
        case ISR_NOFMANY:
            pro["Rule"] ="ISR_NOFMANY";
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
        pro["Switchs"]=propdets;
        break;
    }
    return pro;
}
