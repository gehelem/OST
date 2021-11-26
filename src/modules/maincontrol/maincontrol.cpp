#include "maincontrol.h"

MainControl *initialize(QString name,QString label)
{
    MainControl *basemodule = new MainControl(name,label);
    return basemodule;
}

MainControl::MainControl(QString name,QString label)
    : Basemodule(name,label)
{

}

MainControl::~MainControl()
{

}







