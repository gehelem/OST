#include "OSTParser.h"
#include "OSTClient.h"
#include <baseclientqt.h>

OSTClient::OSTClient(QObject *parent)
{
    properties = QJsonArray();
    groups = QJsonArray();
    categories = QJsonArray();

    thismodule =QJsonObject();
    thismodule["modulename"]="defaultmodule";
    thismodule["modulelabel"]="Default Module";
    thismodule["modulehasgroups"]="N";
    thismodule["modulehascategories"]="N";
    thismodule["properties"]=properties;
    thismodule["categories"]=categories;
    thismodule["groups"]=groups;


    setServer("localhost", 7624);
    connectServer();
    getDevices();

}
OSTClient::~OSTClient()
{
}
void OSTClient::serverConnected(void)
{
}
void OSTClient::serverDisconnected(int exit_code)
{
}
void OSTClient::newDevice(INDI::BaseDevice *dp)
{
}
void OSTClient::removeDevice(INDI::BaseDevice *dp)
{
}
void OSTClient::newProperty(INDI::Property *property)
{
}
void OSTClient::removeProperty(INDI::Property *property)
{
}
void OSTClient::newNumber(INumberVectorProperty *nvp)
{
}
void OSTClient::newText(ITextVectorProperty *tvp)
{
}
void OSTClient::newSwitch(ISwitchVectorProperty *svp)
{
}
void OSTClient::newLight(ILightVectorProperty *lvp)
{
}
void OSTClient::newMessage(INDI::BaseDevice *dp, int messageID)
{
}
void OSTClient::newBLOB(IBLOB *bp)
{
}

void OSTClient::processTextMessage(QString message)
{
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8()); // garder
    QJsonObject  obj = jsonResponse.object(); // garder

        if (obj["module"].toString()==thismodule["modulename"].toString()) {
            IDLog("%s message received-------------------------------------\n",thismodule["modulename"].toString().toStdString().c_str());
            IDLog("%s\n",QJsonDocument(obj).toJson(QJsonDocument::Indented).toStdString().c_str());
            if (obj["message"].toString()=="readmodule") {
                updatemodule(thismodule["modulename"].toString());
            }

        }

}
void OSTClient::switchstate(std::string newstate)
{
    //IDLog("%s switchstate %s->%s\n",thismodule["modulename"].toString().toStdString().c_str(),newstate.c_str());
    state = newstate;
    //emit s_newstate();

}
QJsonObject OSTClient::getmodule  (QString module)
{
    QJsonObject result;
    QJsonArray props,cats;

    foreach (const QJsonValue & pro , properties) {
        if ((pro.toObject()["parenttype"].toString()=="M")&&(pro.toObject()["parentname"].toString()==module)) {
            props.append(pro.toObject()); //FIXME
        }
    }
    cats = QJsonArray();
    foreach (const QJsonValue & cat , categories) {
        if (cat.toObject()["modulename"].toString()==module) {
            cats.append(getcategory(module,cat.toObject()["categoryname"].toString()));
        }
    }
    result["modulename"]=thismodule["modulename"];
    result["modulelabel"]=thismodule["modulelabel"];
    result["modulehasgroups"]=thismodule["modulehasgroups"];
    result["modulehascategories"]=thismodule["modulehascategories"];

    if (props.size() > 0) result["properties"]=props;
    if (cats.size() > 0)  result["categories"]=cats;

    return result;


}
QJsonObject OSTClient::getcategory(QString module,QString category)
{
    QJsonObject result;
    foreach (const QJsonValue & cat , categories) {
        QJsonArray props;
        props=QJsonArray();
        if ((cat.toObject()["categoryname"].toString()==category)&&(cat.toObject()["modulename"].toString()==module)) {
            foreach (const QJsonValue & pro , properties) {
                if ((pro.toObject()["parenttype"].toString()=="C")&&(pro.toObject()["categoryname"].toString()==category)) {
                    props.append(pro.toObject());
                }
            }
            QJsonArray gros;
            gros=QJsonArray();
            foreach (const QJsonValue & gro , groups) {
                if ((gro.toObject()["categoryname"].toString()==category)&&(gro.toObject()["modulename"].toString()==module)) {
                    gros.append(getgroup(module,category,gro.toObject()["groupname"].toString()));
                }
            }


        result["modulename"]=cat.toObject()["modulename"];
        result["categoryname"]=cat.toObject()["categoryname"];
        result["categorylabel"]=cat.toObject()["categorylabel"];
        if (props.size()>0) result["properties"]=props;
        if (gros.size()>0) result["groups"]=gros;
        }
    }
    return result;

}
QJsonObject OSTClient::getgroup   (QString module,QString category,QString group)
{
    QJsonObject result;
    foreach (const QJsonValue & gro , groups) {
        if ((gro.toObject()["groupname"].toString()==group)&&(gro.toObject()["modulename"].toString()==module)) {
            QJsonArray props;
            foreach (const QJsonValue & pro , properties) {
                if ((pro.toObject()["parenttype"].toString()=="G")&&(pro.toObject()["parentname"].toString()==group)) {
                    props.append(getproperty(module,category,group,pro.toObject()["name"].toString()));
                }
            }
            result["modulename"]=gro.toObject()["modulename"];
            result["categoryname"]=gro.toObject()["categoryname"];
            result["groupname"]=gro.toObject()["groupname"];
            result["grouplabel"]=gro.toObject()["grouplabel"];
            if (props.size()>0) result["properties"]=props;
        }
    }
    return result;
}
QJsonObject OSTClient::getproperty   (QString module,QString category,QString group,QString property)
{
    foreach (const QJsonValue & pro , properties) {
        if ((pro.toObject()["name"].toString()==property)&&(pro.toObject()["modulename"].toString()==module)) {
            return pro.toObject();
        }
    }
    return QJsonObject();
}

void OSTClient::updateproperty(QString module,QString category,QString group,QString property)
{
    QJsonObject result;
    result["message"]="updateproperty";
    result["property"]=getproperty(module,category,group,property);
    //result =QJsonObject();
    emit sendjson(result);
}
void OSTClient::updategroup(QString module, QString category, QString group)
{
    QJsonObject result;
    result["message"]="updategroup";
    result["group"]=getgroup(module,category,group);
    emit sendjson(result);
}
void OSTClient::updatecategory(QString module, QString category)
{
    QJsonObject result;
    result["message"]="updatecategory";
    result["category"]=getcategory(module,category);
    emit sendjson(result);
}
void OSTClient::updatemodule(QString module)
{
    QJsonObject result;
    result["message"]="updatemodule";
    result["module"]=getmodule(module);
    emit sendjson(result);
}
void OSTClient::appendmessage(QString module, QString messagename,QString message)
{
    QJsonObject result;
    result["message"]="appendmessage";
    result["modulename"]=module;
    result["messagename"]=messagename;
    result["messagecontent"]=message;
    emit sendjson(result);

}
;
void OSTClient::messagelog(QString message) {
    appendmessage(thismodule["modulename"].toString(),thismodule["modulename"].toString()+"MESSAGE",message);
}
