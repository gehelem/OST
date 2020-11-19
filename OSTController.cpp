#include "OSTController.h"


OSTController::OSTController()
{
    IDLog("OSTController instanciation1\n");
    MyOSTClientFOC.reset(new OSTClientFOC());

    MyOSTClientFOC->setClientname("focus");
    MyOSTClientFOC->setServer("localhost", 7624);
    MyOSTClientFOC->connectServer();
    usleep(500000);
    MyOSTClientFOC->setOSTDevices("CCD Simulator","Focuser Simulator","","","");
    usleep(500000);
    MyOSTClientFOC->startFocusing();
    IDLog("OSTController instanciation99\n");
}

OSTController::~OSTController()
{
    IDLog("OSTController delete\n");
    //delete MyOSTClientFOC;
    //delete MyOSTClientGEN;
}
