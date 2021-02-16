#include <QtCore>
#include <basedevice.h>
#include "focus.h"

FocusModule::FocusModule() : Module()
{

}
FocusModule::~FocusModule()
{
}
void FocusModule::test0(QString txt)
{
    if (txt=="C")  connectIndi();
    if (txt=="A")  connectAllDevices();
    if (txt=="L")  loadDevicesConfs();
    if (txt=="D")  disconnectAllDevices();

}
