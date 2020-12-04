#include "OSTClientGEN.h"
#include "OSTClientPAN.h"
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
    IDLog("%s OSTClientPAN instanciations\n",client_name.c_str());

}
OSTClientPAN::~OSTClientPAN()
{
    //clear();
}

/**************************************************************************************
**
*************************************************************************************/
void OSTClientPAN::newDevice(INDI::BaseDevice *dp) {
    emit emitnewdevice(dp);
}
void OSTClientPAN::newProperty(INDI::Property *property) {
    emit emitnewprop(property);
}
void OSTClientPAN::newText(ITextVectorProperty *prop) {
    emit emitnewtext(prop);
}
void OSTClientPAN::newSwitch(ISwitchVectorProperty *prop) {
    emit emitnewswitch(prop);
}
void OSTClientPAN::newNumber(INumberVectorProperty *prop) {
    emit emitnewnumber(prop);
}
void OSTClientPAN::newLight(ILightVectorProperty *prop) {
    emit emitnewlight(prop);
}
void OSTClientPAN::newMessage(INDI::BaseDevice *dp, int messageID) {
    //IDLog("%s Recveing message from Server:\n\n########################\n%s\n########################\n\n",client_name.c_str(),dp->messageQueue(messageID).c_str());
}
void OSTClientPAN::newBLOB(IBLOB *bp) {
    //IDLog("%s newblob from %s\n",client_name.c_str(),bp->name);
}
void OSTClientPAN::givemeall(void) {

    for (int i=0;i<getDevices().size();i++) {
        INDI::BaseDevice *d = getDevices()[i];
        emit emitnewdevice(d);
        std::vector<INDI::Property *> allprops = *d->getProperties();
        for (int j=0;j<allprops.size();j++) {
            emit emitnewprop(allprops[j]);
            switch (allprops[j]->getType()) {
                case INDI_NUMBER:
                    emit emitnewnumber(allprops[j]->getNumber());
                    break;
                case INDI_TEXT:
                    emit emitnewtext(allprops[j]->getText());
                    break;
                case INDI_LIGHT:
                    emit emitnewlight(allprops[j]->getLight());
                    break;
                case INDI_SWITCH:
                    emit emitnewswitch(allprops[j]->getSwitch());
                    break;

                ;
            }

        }
    }

}
