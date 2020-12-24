#include "OSTClientCTL.h"
#include "OSTParser.h"

/**************************************************************************************
**
***************************************************************************************/
OSTClientCTL::OSTClientCTL(QObject *parent)
{
    QJsonArray props,texts,switchs,cats,nums;
    QJsonObject prop,text,swit,cat,num;

    thismodule["modulename"]="OSTClientCTL";
    thismodule["modulelabel"]="Main control";
    thismodule["modulehasgroups"]="N";
    thismodule["modulehascategories"]="Y";
    thismodule["groups"]=QJsonArray();

    categories=QJsonArray();
    properties=QJsonArray();

    cat=QJsonObject();
    cat["categoryname"]="OSTCTLCAT_PARAMS";
    cat["categorylabel"]="Parameters";
    cat["modulename"]=thismodule["modulename"];
    categories.append(cat);

    cat=QJsonObject();
    cat["categoryname"]="OSTCTLCAT_CONNECT";
    cat["categorylabel"]="Connection";
    cat["modulename"]=thismodule["modulename"];
    categories.append(cat);
    thismodule["categories"]=categories;

    prop=QJsonObject();
    nums=QJsonArray();
    num=QJsonObject();
    num=Onumber("OSTCTLPRO_SCOPE_FL","Scope focal length",800);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTCTLPRO_SCOPE_DI","Scope diameter",200);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTCTLPRO_GUIDE_FL","Guider Focal length",225);
    nums.append(num);
    num=QJsonObject();
    num=Onumber("OSTCTLPRO_GUIDE_DI","Guider diameter",60);
    nums.append(num);
    prop=Oproperty("OSTCTLPRO_FLDI", "Optics","INDI_NUMBER","IP_RW","IPS_IDLE","C","OSTCTLCAT_PARAMS","",
                   nums,thismodule["modulename"].toString(),"OSTCTLCAT_PARAMS","");
    properties.append(prop);

    prop=QJsonObject();
    switchs=QJsonArray();
    swit=QJsonObject();
    swit=Oswitch("OSTCTLPRO_CONNECT_CONNECT","Connect","ISS_OFF");
    switchs.append(swit);
    swit=QJsonObject();
    swit=Oswitch("OSTCTLPRO_CONNECT_DISCONNECT","Disconnect","ISS_ON");
    switchs.append(swit);
    prop=Oproperty("OSTCTLPRO_CONNECT", "Connection","INDI_SWITCH","IP_RW","IPS_IDLE","C","OSTCTLCAT_CONNECT","",
                   switchs,thismodule["modulename"].toString(),"OSTCTLCAT_CONNECT","");
    properties.append(prop);


    thismodule["properties"]=properties;
    groups=QJsonArray();


}
OSTClientCTL::~OSTClientCTL()
{
    //clear();
}
