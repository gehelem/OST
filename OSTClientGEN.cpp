#include "OSTClientGEN.h"
#include "OSTParser.h"

/**************************************************************************************
**
***************************************************************************************/
OSTClientGEN::OSTClientGEN(QObject *parent)
{
    QJsonArray props,texts,switchs,nums,lights;
    QJsonObject num,cat,prop,text,swit,group,light;


    thismodule["modulename"]="OSTClientGEN";
    thismodule["modulelabel"]="Generic example Module";
    thismodule["modulehasgroups"]="Y";
    thismodule["modulehascategories"]="Y";
    thismodule["groups"]=QJsonArray();

    categories=QJsonArray();
    groups=QJsonArray();
    properties=QJsonArray();

    cat=QJsonObject();
    cat["categoryname"]="OSTGENCAT_CAT1";
    cat["categorylabel"]="Category 1";
    cat["modulename"]=thismodule["modulename"];
    categories.append(cat);

    cat=QJsonObject();
    cat["categoryname"]="OSTGENCAT_CAT2";
    cat["categorylabel"]="Category 2";
    cat["modulename"]=thismodule["modulename"];
    categories.append(cat);

    groups=QJsonArray();

    group=QJsonObject();
    group["groupname"]="OSTGENGRP_GRP1";
    group["grouplabel"]="Group 1";
    group["categoryname"]="OSTGENCAT_CAT1";
    group["modulename"]=thismodule["modulename"];
    groups.append(group);

    group=QJsonObject();
    group["groupname"]="OSTGENGRP_GRP2";
    group["grouplabel"]="Group 2";
    group["categoryname"]="OSTGENCAT_CAT1";
    group["modulename"]=thismodule["modulename"];
    groups.append(group);

    prop=QJsonObject();
    texts=QJsonArray();
    text=QJsonObject();
    text=Otext("OSTGENPRO_TEXT1","Generic text 1","Value - Generic text 1");
    texts.append(text);
    text=QJsonObject();
    text=Otext("OSTGENPRO_TEXT2","Generic text 2","Value - Generic text 2");
    texts.append(text);
    prop=Oproperty("OSTGENPRO_TEXT","Generic text","INDI_TEXT","IP_RW","IPS_IDLE","G","OSTGENGRP_GRP1","",
                   texts,
                   thismodule["modulename"].toString(),"OSTGENCAT_CAT1","OSTGENCAT_GRP1");
    properties.append(prop);

    prop=QJsonObject();
    nums=QJsonArray();
    num=QJsonObject();
    num=Onumber("OSTGENPRO_NUM1","Generic number 1",10);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTGENPRO_NUM2","Generic number 2",0.5);
    nums.append(num);
    prop=Oproperty("OSTGENPRO_NUM","Generic number","INDI_NUMBER","IP_RW","IPS_IDLE","G","OSTGENGRP_GRP1","",
                   nums,
                   thismodule["modulename"].toString(),"OSTGENCAT_CAT1","OSTGENCAT_GRP1");
    properties.append(prop);

    prop=QJsonObject();
    switchs=QJsonArray();
    swit=QJsonObject();
    swit=Oswitch("OSTGENPRO_SWITCH1","Generic switch 1","ISS_ON");
    switchs.append(swit);
    swit=QJsonObject();
    swit=Oswitch("OSTGENPRO_SWITCH2","Generic switch 2","ISS_OFF");
    switchs.append(swit);
    prop=Oproperty("OSTGENPRO_SWITCH","Generic switch","INDI_SWITCH","IP_RW","IPS_IDLE","G","OSTGENGRP_GRP1","ISS_NORULE",
                   switchs,
                   thismodule["modulename"].toString(),"OSTGENCAT_CAT1","OSTGENCAT_GRP1");
    properties.append(prop);

    prop=QJsonObject();
    switchs=QJsonArray();
    swit=QJsonObject();
    swit=Oswitch("OSTGENPRO_SWITCHBIS1","Generic switchbis 1","ISS_ON");
    switchs.append(swit);
    swit=QJsonObject();
    swit=Oswitch("OSTGENPRO_SWITCHBIS2","Generic switchbis 2","ISS_OFF");
    switchs.append(swit);
    prop=Oproperty("OSTGENPRO_SWITCHBIS","Generic switchbis","INDI_SWITCH","IP_RW","IPS_IDLE","C","OSTGENCAT_CAT1","ISS_NORULE",
                   switchs,
                   thismodule["modulename"].toString(),"OSTGENCAT_CAT1","");
    properties.append(prop);

    prop=QJsonObject();
    switchs=QJsonArray();
    swit=QJsonObject();
    swit=Oswitch("OSTGENPRO_SWITCHTER1","Generic switchter 1","ISS_ON");
    switchs.append(swit);
    swit=QJsonObject();
    swit=Oswitch("OSTGENPRO_SWITCHTER2","Generic switchter 2","ISS_OFF");
    switchs.append(swit);
    prop=Oproperty("OSTGENPRO_SWITCHTER","Generic switchter","INDI_SWITCH","IP_RW","IPS_IDLE","M",thismodule["modulename"].toString(),"ISS_NORULE",
                   switchs,
                   thismodule["modulename"].toString(),"","");
    properties.append(prop);


}
OSTClientGEN::~OSTClientGEN()
{
    //clear();
}
