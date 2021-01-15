#include <QtCore>
#include <basedevice.h>
#include "module.h"

Module::Module(MyClient *cli)
{
    //qDebug() << "Instanciation";
    initProperties();
    client=cli;
    connect(client,&MyClient::gotserverConnected,this,&Module::gotserverConnected);
    connect(client,&MyClient::gotserverDisconnected,this,&Module::gotserverDisconnected);
    connect(client,&MyClient::gotnewDevice,this,&Module::gotnewDevice);
    connect(client,&MyClient::gotremoveDevice,this,&Module::gotremoveDevice);
    connect(client,&MyClient::gotnewProperty,this,&Module::gotnewProperty);
    connect(client,&MyClient::gotremoveProperty,this,&Module::gotremoveProperty);
    connect(client,&MyClient::gotnewText,this,&Module::gotnewText);
    connect(client,&MyClient::gotnewSwitch,this,&Module::gotnewSwitch);
    connect(client,&MyClient::gotnewLight,this,&Module::gotnewLight);
    connect(client,&MyClient::gotnewBLOB,this,&Module::gotnewBLOB);
    connect(client,&MyClient::gotnewNumber,this,&Module::gotnewNumber);
    connect(client,&MyClient::gotnewMessage,this,&Module::gotnewMessage);

}
Module::~Module()
{
}

void Module::initProperties(void)
{
    props.setText("modulename",QString("DefaultModule"));
    props.setText("modulelabel",QString("DefaultModuleDoNotUseAsIs"));
    props.setText("dummy",QString("Quand on a pas de technique faut y aller à la zob."));
}

void Module::gotserverConnected()
{
    //qDebug() << "gotserverConnected";

}
void Module::gotserverDisconnected(int exit_code)
{
    //qDebug() << "gotserverDisconnected";
    Q_UNUSED(exit_code);
}
void Module::gotnewDevice(INDI::BaseDevice *dp)
{
    //qDebug() << "gotnewDevice : " << dp->getDeviceName();
    Q_UNUSED(dp);
}
void Module::gotremoveDevice(INDI::BaseDevice *dp)
{
    //qDebug() << "gotremoveDevice";
    Q_UNUSED(dp);
}
void Module::gotnewProperty(INDI::Property *property)
{
    //qDebug() << "gotnewProperty" << tasks.size();
    Q_UNUSED(property);

}
void Module::gotremoveProperty(INDI::Property *property)
{
    //qDebug() << "gotremoveProperty";
    Q_UNUSED(property);
}


void Module::gotnewText(ITextVectorProperty *tvp)
{
    //qDebug() << "gotnewText";
    if (!tasks.size()) return;

    if (    (tasks.front().tasktype==TT_WAIT_PROP)
         && (strcmp(tasks.front().devicename,tvp->device)==0)
         && (strcmp(tasks.front().propertyname,tvp->name)==0)
         )
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),tvp);
        } else {
            popnext();
        }
    }

    if (    (tasks.front().tasktype==TT_WAIT_TEXT)
         && (strcmp(tasks.front().devicename,tvp->device)==0)
         && (strcmp(tasks.front().propertyname,tvp->name)==0) ) {
        for (int i=0;i<tvp->ntp;i++) {
            if ((strcmp(tasks.front().elementname,tvp->tp[i].name)==0)
               && (tasks.front().text==tvp->tp[i].text) ){
                if (tasks.front().specific) {
                    executeTaskSpec(tasks.front(),tvp);
                } else {
                    popnext();
                }
            }
        }
    }
}

void Module::gotnewSwitch(ISwitchVectorProperty *svp)
{
   // qDebug() << "gotnewSwitch";
    if (!tasks.size()) return;

    if (    (tasks.front().tasktype==TT_WAIT_PROP)
         && (strcmp(tasks.front().devicename,svp->device)==0)
         && (strcmp(tasks.front().propertyname,svp->name)==0)
         )
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),svp);
        } else {
            popnext();
        }
    }

    if (    (tasks.front().tasktype==TT_WAIT_SWITCH)
         && (strcmp(tasks.front().devicename,svp->device)==0)
         && (strcmp(tasks.front().propertyname,svp->name)==0) ) {
        for (int i=0;i<svp->nsp;i++) {
            if ((strcmp(tasks.front().elementname,svp->sp[i].name)==0)
               && (tasks.front().sw==svp->sp[i].s) ){
                if (tasks.front().specific) {
                    executeTaskSpec(tasks.front(),svp);
                } else {
                    popnext();
                }
            }
        }
    }

}

void Module::gotnewNumber(INumberVectorProperty *nvp)
{
    //qDebug() << "gotnewNumber";
    if (!tasks.size()) return;

    if (    (tasks.front().tasktype==TT_WAIT_PROP)
         && (strcmp(tasks.front().devicename,nvp->device)==0)
         && (strcmp(tasks.front().propertyname,nvp->name)==0)
         )
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),nvp);
        } else {
            popnext();
        }
    }

    if (    (tasks.front().tasktype==TT_WAIT_NUMBER)
         && (strcmp(tasks.front().devicename,nvp->device)==0)
         && (strcmp(tasks.front().propertyname,nvp->name)==0) ) {
        for (int i=0;i<nvp->nnp;i++) {
            if ((strcmp(tasks.front().elementname,nvp->np[i].name)==0)
               && (tasks.front().value==nvp->np[i].value) ){
                if (tasks.front().specific) {
                    executeTaskSpec(tasks.front(),nvp);
                } else {
                    popnext();
                }
            }
        }
    }
}
void Module::gotnewLight(ILightVectorProperty *lvp)
{
    //qDebug() << "gotnewLight";
    if (!tasks.size()) return;

    if (    (tasks.front().tasktype==TT_WAIT_PROP)
         && (strcmp(tasks.front().devicename,lvp->device)==0)
         && (strcmp(tasks.front().propertyname,lvp->name)==0)
         )
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),lvp);
        } else {
            popnext();
        }
    }

    if (    (tasks.front().tasktype==TT_WAIT_LIGHT)
         && (strcmp(tasks.front().devicename,lvp->device)==0)
         && (strcmp(tasks.front().propertyname,lvp->name)==0) )
    {
        for (int i=0;i<lvp->nlp;i++) {
/*            if ((strcmp(tasks.front().elementname,lvp->lp[i].name)==0)
               && (tasks.front().XXXX==lvp->lp[i].s) ){
                if (tasks.front().specific) {
                    executeTaskSpec(tasks.front(),lvp);
                } else {
                    popnext();
                }
            }
*/
        }
    }
}
void Module::gotnewBLOB(IBLOB *bp)
{
    //qDebug() << "gotnewBLOB";
    if (!tasks.size()) return;

    if (  (tasks.front().tasktype==TT_WAIT_BLOB)
       && (strcmp(tasks.front().devicename,bp->bvp->device)==0) ) {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),bp);
        } else {
            popnext();
        }
    }

    if (  (tasks.front().tasktype==TT_ANALYSE_SEP)
       && (strcmp(tasks.front().devicename,bp->bvp->device)==0) ) {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),bp);
        } else {
            image.reset(new Image());
            disconnect(image.get(),&Image::successSEP, 0, 0);
            connect(image.get(),&Image::successSEP,this,&Module::finishedSEP);
            image->LoadFromBlob(bp);
            image->FindStars();
            popnext();
        }
    }

    if (  (tasks.front().tasktype==TT_ANALYSE_SOLVE)
       && (strcmp(tasks.front().devicename,bp->bvp->device)==0) ) {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),bp);
        } else {
            image.reset(new Image());
            disconnect(image.get(),&Image::successSolve, 0, 0);
            connect(image.get(),&Image::successSolve,this,&Module::finishedSolve);
            image->LoadFromBlob(bp);
            image->SolveStars();
            popnext();
        }
    }
}
void Module::gotnewMessage(INDI::BaseDevice *dp, int messageID)
{
    //qDebug() << "gotnewMessage";
    if (!tasks.size()) return;

    Q_UNUSED(dp);
    Q_UNUSED(messageID);
}

bool Module::taskSendNewNumber(const char *deviceName, const char *propertyName, const char *elementName, double value)
{
    //qDebug() << "taskSendNewNumber";
    sleep(1);
    INDI::BaseDevice *dp;
    dp = client->getDevice(deviceName);

    if (dp== nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << " device. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }
    INumberVectorProperty *prop = nullptr;
    prop = dp->getNumber(propertyName);
    if (prop == nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << " property. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }

    for (int i=0;i<prop->nnp;i++) {
        if (strcmp(prop->np[i].name, elementName) == 0) {
            prop->np[i].value=value;
            client->sendNewNumber(prop);
            return true;
        }
    }
    qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << "/" << elementName << " element. Aborting.";
    tasks =QQueue<Ttask>();
    return false;

}
bool Module::taskSendNewText  (const char *deviceName, const char *propertyName, const char *elementName, const char *text)
{
    //qDebug() << "taskSendNewText";
    INDI::BaseDevice *dp;
    dp = client->getDevice(deviceName);

    if (dp== nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << " device. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }
    ITextVectorProperty *prop = nullptr;
    prop= dp->getText(propertyName);
    if (prop == nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << " property. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }

    for (int i=0;i<prop->ntp;i++) {
        if (strcmp(prop->tp[i].name, elementName) == 0) {
            client->sendNewText(deviceName,propertyName,elementName,text);
            return true;
        }
    }
    qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << "/" << elementName << " element. Aborting.";
    tasks =QQueue<Ttask>();
    return false;
}
bool Module::taskSendNewSwitch(const char *deviceName, const char *propertyName, const char *elementName, ISState sw)
{
    //qDebug() << "taskSendNewSwitch";

    INDI::BaseDevice *dp;
    dp = client->getDevice(deviceName);

    if (dp== nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << " device. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }
    ISwitchVectorProperty *prop = nullptr;
    prop= dp->getSwitch(propertyName);
    if (prop == nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << " property. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }

    for (int i=0;i<prop->nsp;i++) {
        if (strcmp(prop->sp[i].name, elementName) == 0) {
            prop->sp[i].s=sw;
            client->sendNewSwitch(prop);
            return true;
        }
    }
    qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << "/" << elementName << " element. Aborting.";
    tasks =QQueue<Ttask>();
    return false;

}
void Module::executeTask(Ttask task)
{
    //qDebug() << "executeTask";

    if (task.tasktype==TT_SEND_NUMBER) {
        taskSendNewNumber(task.devicename,task.propertyname,task.elementname,task.value);
        popnext();
    }
    if (task.tasktype==TT_SEND_TEXT) {
        taskSendNewText(task.devicename,task.propertyname,task.elementname,task.text);
        popnext();
    }
    if (task.tasktype==TT_SEND_SWITCH) {
        taskSendNewSwitch(task.devicename,task.propertyname,task.elementname,task.sw);
        popnext();
    }
    if (task.tasktype==TT_SPEC) {
        executeTaskSpec(task);
    }

}
void Module::popnext(void)
{
    //qDebug() << "popnext";
    if (tasks.size()<=1) {
        qDebug() << "finished";
        emit finished();
        return;
    }  else {
        qDebug() << tasks.front().tasklabel << "finished";
        tasks.pop_front();
        executeTask(tasks.front());
    }
}


void Module::finishedSEP(void)
{
    //qDebug() << "finishedSEP";
    if (!tasks.size()) return;

    if (tasks.front().tasktype==TT_WAIT_SEP)
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front());
        } else {
            stars=image->stars;
            popnext();
        }
    }
}
void Module::finishedSolve(void)
{
    //qDebug() << "finishedSolve";
    if (!tasks.size()) return;

    if  (tasks.front().tasktype==TT_WAIT_SOLVE)
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front());
        } else {
            stars=image->stars;
            popnext();
        }
    }

}
void Module::addnewtask (Tasktype tasktype,  QString taskname, QString tasklabel,bool specific,
                const char *devicename,const char *propertyname,const char *elementname)
{
    //qDebug() << "addnewtask1";

    Ttask task;
    task.tasktype = tasktype;
    task.taskname = taskname;
    task.tasklabel = tasklabel;
    task.specific = specific;
    task.devicename= devicename;
    task.propertyname = propertyname;
    task.elementname = elementname;
    tasks.append(task);
}
void Module::addnewtask (Tasktype tasktype,  QString taskname, QString tasklabel,bool specific,
                const char *devicename,const char *propertyname,const char *elementname,
                double value,const char *text,ISState sw)
{
    //qDebug() << "addnewtask2";

    Ttask task;
    task.tasktype = tasktype;
    task.taskname = taskname;
    task.tasklabel = tasklabel;
    task.specific = specific;
    task.devicename= devicename;
    task.propertyname = propertyname;
    task.elementname = elementname;
    task.value= value;
    task.text = text;
    task.sw = sw;
    tasks.append(task);
}
