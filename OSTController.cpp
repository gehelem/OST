#include "OSTController.h"


OSTController::OSTController()
{
    MyOSTClientFOC.reset(new OSTClientFOC());

    MyOSTClientFOC->setClientname("FOC");
    MyOSTClientFOC->setOSTDevices("CCD Simulator","Focuser Simulator","","","");
    MyOSTClientFOC->setServer("localhost", 7624);
    MyOSTClientFOC->connectServer();
    usleep(500000);
    MyOSTClientFOC->startFocusing(30000,100,200,10);
    //MyOSTClientFOC->askNewJob("coarse");
}

OSTController::~OSTController()
{
    IDLog("OSTController delete\n");
    //delete MyOSTClientFOC;
    //delete MyOSTClientGEN;
}
