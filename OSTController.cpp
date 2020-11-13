#include "OSTClientSEQ.h"
#include "OSTClientFOC.h"


#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <dlfcn.h>
#include <QPointer>
#include <QObject>


static std::unique_ptr<OSTClientFOC> MyOSTClientFOC(new OSTClientFOC());
static std::unique_ptr<OSTClientGEN> MyOSTClientGEN(new OSTClientGEN());


//int main(int /*argc*/, char **/*argv*/)
int main(int argc, char *argv[]) {

    MyOSTClientGEN->setClientname("controller");
    MyOSTClientGEN->setServer("localhost", 7624);
    MyOSTClientGEN->connectServer();

    MyOSTClientFOC->setClientname("focus");
    MyOSTClientFOC->setServer("localhost", 7624);
    MyOSTClientFOC->connectServer();
    MyOSTClientGEN->setOSTDevices("CCD Simulator","Focuser Simulator","Telescope Simulator","Filter Simulator","Guide Simulator");
    usleep(500000);
    MyOSTClientFOC->setOSTDevices("CCD Simulator","Focuser Simulator","","","");
    usleep(500000);
    MyOSTClientFOC->connectallOSTDevices();

    usleep(500000);

    MyOSTClientFOC->startFocusing();
    while(1);
}


