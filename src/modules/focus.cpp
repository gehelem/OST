#include <QtCore>
#include <QtConcurrent>
#include <basedevice.h>
#include "focus.h"



FocusModule::FocusModule() : Module()
{
    _modulename = "focus";
/*    indiclient=IndiCLient::getInstance();
    connect(indiclient,&IndiCLient::SigNewNumber,this,&FocusModule::IndiNewNumber);
    connect(indiclient,&IndiCLient::SigNewBLOB,this,&FocusModule::IndiNewBLOB);*/

}
FocusModule::~FocusModule()
{
}
void FocusModule::test0(QString txt)
{
    if (txt=="c")  connectIndi();
    if (txt=="a")  connectAllDevices();
    if (txt=="l")  loadDevicesConfs();
    if (txt=="d")  disconnectAllDevices();
    if (txt=="x")  disconnectIndi();
    if (txt=="f")
    {
        //startCoarse();
        QFuture<void> future = QtConcurrent::run(this,&FocusModule::startCoarse);
    }

}
void FocusModule::IndiNewNumber(INumberVectorProperty *nvp)
{
    //qDebug() << "gotnewNumber";
    if (QString(nvp->name)=="CCD Simulator")
    sendMessage("New number " + QString(nvp->device) + " - " + QString(nvp->name) + " - " + QString::number(nvp->np[0].value));
    indiclient->setBLOBMode(B_ALSO,"CCD Simulator",nullptr);
}
void FocusModule::IndiNewBLOB(IBLOB *bp)
{
    //qDebug() << "gotnewNumber";
    sendMessage("New blob " + QString(bp->bvp->device) + " - " + QString(bp->name) + " - " + QString::number(bp->bloblen));
    _newblob = true;
}
void FocusModule::CallStartCoarse(void)
{
    QThread t1;
    //t1.create(startCoarse());
    //t1.start();
    //QtConcurrent::run(this,&FocusModule::startCoarse);

}

void FocusModule::startCoarse(void)
{
    sendMessage("start coarse");
    qDebug() << "-1";
    _newblob = false;
    qDebug() << "-2";
    indiclient->setBLOBMode(B_ALSO,"CCD Simulator",nullptr);
    qDebug() << "-3";
    sendNewNumber("CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",2);
    qDebug() << "-4";
    while (!_newblob )
    {
        //qDebug() << indiclient->getDevice("CCD Simulator")->getNumber("CCD_EXPOSURE")->s;
        sendMessage("waiting blob");
        QThread::msleep(1000);
    }
    _newblob = false;
}
