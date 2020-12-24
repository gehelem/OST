#include "OSTClientGEN.h"
#include "OSTClientPAN.h"
#include "OSTParser.h"
#include <basedevice.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <inttypes.h>
#include <baseclientqt.h>

//using namespace std;

//QT Includes
//QT Includes
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>

/**************************************************************************************
**
***************************************************************************************/
OSTClientPAN::OSTClientPAN(QObject *parent)// : OSTClientGEN(parent)
{
    thismodule["modulename"]="OSTClientPAN";
    thismodule["modulelabel"]="Indi Control Panel";
    thismodule["modulehasgroups"]="Y";
    thismodule["modulehascategories"]="Y";
}
OSTClientPAN::~OSTClientPAN()
{
    //clear();
}

void OSTClientPAN::serverConnected(void)
{
    updatemodule("OSTClientPAN");

}
void OSTClientPAN::serverDisconnected(int exit_code)
{
}
void OSTClientPAN::newDevice(INDI::BaseDevice *dp)
{
    updatecategory("OSTClientPAN",dp->getDeviceName());
}
void OSTClientPAN::removeDevice(INDI::BaseDevice *dp)
{
}
void OSTClientPAN::newProperty(INDI::Property *property)
{
    updateproperty("OSTClientPAN",property->getDeviceName(),property->getGroupName(),property->getName());
}
void OSTClientPAN::removeProperty(INDI::Property *property)
{
}
void OSTClientPAN::newNumber(INumberVectorProperty *nvp)
{
    //updateproperty("OSTClientPAN",nvp->device,nvp->group,nvp->name);
}
void OSTClientPAN::newText(ITextVectorProperty *tvp)
{
    updateproperty("OSTClientPAN",tvp->device,tvp->group,tvp->name);
}
void OSTClientPAN::newSwitch(ISwitchVectorProperty *svp)
{
    IDLog("New switch %s\n",svp->name);
    updateproperty("OSTClientPAN",svp->device,svp->group,svp->name);
}
void OSTClientPAN::newLight(ILightVectorProperty *lvp)
{
    updateproperty("OSTClientPAN",lvp->device,lvp->group,lvp->name);
}
void OSTClientPAN::newMessage(INDI::BaseDevice *dp, int messageID)
{
    QString tt = dp->getDeviceName();
    appendmessage("OSTClientPAN",
                  "OSTClientPAN"+tt+"MESSAGE",
                  dp->messageQueue(messageID).c_str()
                  );

}
void OSTClientPAN::newBLOB(IBLOB *bp)
{
}

QJsonObject OSTClientPAN::getmodule  (QString module)
{
    QJsonObject result;
    QJsonObject categ;
    QJsonArray categories;
    if (!(module=="OSTClientPAN")) {
        return result;
        IDLog("OSTClientPAN::getmodule - wrong module\n");
    };

    for (int i=0;i<getDevices().size();i++) {
        categ = getcategory(module,getDevices()[i]->getDeviceName());
        categories.append(categ);
    }
    result["categories"]=categories;

    result["modulename"]=thismodule["modulename"];
    result["modulelabel"]=thismodule["modulelabel"];
    result["modulehasgroups"]=thismodule["modulehasgroups"];
    result["modulehascategories"]=thismodule["modulehascategories"];

    return result;
}


QJsonObject OSTClientPAN::getcategory(QString module,QString category)
{
    QJsonObject result;
    if (!(module=="OSTClientPAN")) {
        return result;
        IDLog("OSTClientPAN::getcategory - wrong module\n");
    };

    QJsonObject group;
    QJsonArray groups;
    for (std::string gr : GroupsArray(getDevice(category.toUtf8()))) {
        group=QJsonObject();
        group=getgroup(module,category,gr.c_str());
        groups.append(group);
    }
    QJsonObject prop = Oproperty("OSTClientPAN"+category+"MESSAGE","Message","INDI_MESSAGE","IP_RW","IPS_IDLE","C",category,"",
                   QJsonArray(),
                   thismodule["modulename"].toString(),category,"");
    QJsonArray props;
    props.append(prop);
    result["properties"]=props;
    result["modulename"]=module;
    result["categoryname"]=category;
    result["categorylabel"]=category;
    result["groups"]=groups;
    return result;
}
QJsonObject OSTClientPAN::getgroup   (QString module,QString category,QString group)
{
    QJsonObject result;
    if (!(module=="OSTClientPAN")) {
        return result;
        IDLog("OSTClientPAN::getgroup - wrong module\n");
    };

    QJsonArray allprops;
    for (int idev=0;idev <getDevices().size();idev++){
        INDI::BaseDevice *Idev = getDevices()[idev];
        for (int ipro=0;ipro<Idev->getProperties()->size();ipro++) {
            INDI::Property *Iprop;
            Iprop = Idev->getProperties()->data()[ipro];
            if ((Iprop->getGroupName()==group)&&(Iprop->getDeviceName()==category)) {
                QJsonObject prop;
                prop=IProToJson(Iprop);
                allprops.append(prop);
            }
        }
    }
    result["modulename"]=module;
    result["categoryname"]=category;
    result["groupname"]=group;
    result["grouplabel"]=group;
    result["properties"]=allprops;
    return result;

}
QJsonObject OSTClientPAN::getproperty   (QString module,QString category,QString group,QString property)
{
    QJsonObject result;
    if (!(module=="OSTClientPAN")) {
        return result;
        IDLog("OSTClientPAN::getproperty - wrong module\n");
    };

    INDI::Property *Iprop;
    Iprop = getDevice(category.toUtf8())->getProperty(property.toUtf8());
    result = IProToJson(Iprop);
    return result;
}

