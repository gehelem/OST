#include <OSTCommon.h>
#include <baseclientqt.h>


QJsonDocument  INDItoJSON(INDI::Property *prop) {
    QJsonObject json;
    json["Basedevice"]="turlututu";
    json["Name"]      =prop->getName();
    /*json["Label"]     =prop->getLabel();
    json["GroupName"] =prop->getGroupName();
    json["DeviceName"]=prop->getDeviceName();
    json["Timestamp"] =prop->getTimestamp();*/
    //switch prop.getTimestamp();
    QJsonDocument jsondoc(json);
    return jsondoc;
}


/*INDI::Property  JSONtoINDI(QJsonObject json) {
    return nullptr;
}*/
