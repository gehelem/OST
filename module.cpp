#include <QtCore>
#include <basedevice.h>
#include "module.h"

Module::Module(MyClient *cli,Properties *properties)
{
    //qDebug() << "Instanciation";
    indiclient=cli;
    props=properties;
    initProperties();
    connect(indiclient,&MyClient::gotserverConnected,this,&Module::gotserverConnected);
    connect(indiclient,&MyClient::gotserverDisconnected,this,&Module::gotserverDisconnected);
    connect(indiclient,&MyClient::gotnewDevice,this,&Module::gotnewDevice);
    connect(indiclient,&MyClient::gotremoveDevice,this,&Module::gotremoveDevice);
    connect(indiclient,&MyClient::gotnewProperty,this,&Module::gotnewProperty);
    connect(indiclient,&MyClient::gotremoveProperty,this,&Module::gotremoveProperty);
    connect(indiclient,&MyClient::gotnewText,this,&Module::gotnewText);
    connect(indiclient,&MyClient::gotnewSwitch,this,&Module::gotnewSwitch);
    connect(indiclient,&MyClient::gotnewLight,this,&Module::gotnewLight);
    connect(indiclient,&MyClient::gotnewBLOB,this,&Module::gotnewBLOB);
    connect(indiclient,&MyClient::gotnewNumber,this,&Module::gotnewNumber);
    connect(indiclient,&MyClient::gotnewMessage,this,&Module::gotnewMessage);

    connect(props,&Properties::signalvalueChanged,this,&Module::slotvalueChanged);
    //connect(properties,&OSTProperties::signalpropCreated,this,&Module::slotpropCreated);
    //connect(properties,&OSTProperties::signalpropDeleted,this,&Module::slotpropDeleted);


}
Module::~Module()
{
}
void Module::slotvalueChanged(Prop prop)
{
   emit signalvalueChanged(prop);
}
void Module::slotpropCreated(Prop prop)
{
   emit signalpropCreated(prop);
}
void Module::slotpropDeleted(Prop prop)
{
   emit signalpropDeleted(prop);
}
void Module::initProperties(void)
{


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
         && (strcmp(tasks.front().devicename.toStdString().c_str(),tvp->device)==0)
         && (strcmp(tasks.front().propertyname.toStdString().c_str(),tvp->name)==0)
         )
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),tvp);
        } else {
            popnext();
        }
    }

    if (    (tasks.front().tasktype==TT_WAIT_TEXT)
         && (strcmp(tasks.front().devicename.toStdString().c_str(),tvp->device)==0)
         && (strcmp(tasks.front().propertyname.toStdString().c_str(),tvp->name)==0) ) {
        for (int i=0;i<tvp->ntp;i++) {
            if ((strcmp(tasks.front().elementname.toStdString().c_str(),tvp->tp[i].name)==0)
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
         && (strcmp(tasks.front().devicename.toStdString().c_str(),svp->device)==0)
         && (strcmp(tasks.front().propertyname.toStdString().c_str(),svp->name)==0)
         )
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),svp);
        } else {
            popnext();
        }
    }

    if (    (tasks.front().tasktype==TT_WAIT_SWITCH)
         && (strcmp(tasks.front().devicename.toStdString().c_str(),svp->device)==0)
         && (strcmp(tasks.front().propertyname.toStdString().c_str(),svp->name)==0) ) {
        for (int i=0;i<svp->nsp;i++) {
            if ((strcmp(tasks.front().elementname.toStdString().c_str(),svp->sp[i].name)==0)
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
         && (strcmp(tasks.front().devicename.toStdString().c_str(),nvp->device)==0)
         && (strcmp(tasks.front().propertyname.toStdString().c_str(),nvp->name)==0)
         )
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),nvp);
        } else {
            popnext();
        }
    }

    if (    (tasks.front().tasktype==TT_WAIT_NUMBER)
         && (strcmp(tasks.front().devicename.toStdString().c_str(),nvp->device)==0)
         && (strcmp(tasks.front().propertyname.toStdString().c_str(),nvp->name)==0) ) {
        for (int i=0;i<nvp->nnp;i++) {
            if ((strcmp(tasks.front().elementname.toStdString().c_str(),nvp->np[i].name)==0)
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
         && (strcmp(tasks.front().devicename.toStdString().c_str(),lvp->device)==0)
         && (strcmp(tasks.front().propertyname.toStdString().c_str(),lvp->name)==0)
         )
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),lvp);
        } else {
            popnext();
        }
    }

    if (    (tasks.front().tasktype==TT_WAIT_LIGHT)
         && (strcmp(tasks.front().devicename.toStdString().c_str(),lvp->device)==0)
         && (strcmp(tasks.front().propertyname.toStdString().c_str(),lvp->name)==0) )
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
       && (strcmp(tasks.front().devicename.toStdString().c_str(),bp->bvp->device)==0) ) {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),bp);
        } else {
            popnext();
        }
    }

    if (  (tasks.front().tasktype==TT_ANALYSE_SEP)
       && (strcmp(tasks.front().devicename.toStdString().c_str(),bp->bvp->device)==0) ) {
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
       && (strcmp(tasks.front().devicename.toStdString().c_str(),bp->bvp->device)==0) ) {
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

bool Module::taskSendNewNumber(QString deviceName, QString propertyName,QString  elementName, double value)
{
    //qDebug() << "taskSendNewNumber" << " " << deviceName << " " << propertyName<< " " << elementName;
    sleep(1);
    INDI::BaseDevice *dp;
    dp = indiclient->getDevice(deviceName.toStdString().c_str());

    if (dp== nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << " device. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }
    INumberVectorProperty *prop = nullptr;
    prop = dp->getNumber(propertyName.toStdString().c_str());
    if (prop == nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << " property. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }

    for (int i=0;i<prop->nnp;i++) {
        if (strcmp(prop->np[i].name, elementName.toStdString().c_str()) == 0) {
            prop->np[i].value=value;
            indiclient->sendNewNumber(prop);
            return true;
        }
    }
    qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << "/" << elementName << " element. Aborting.";
    tasks =QQueue<Ttask>();
    return false;

}
bool Module::taskSendNewText  (QString deviceName,QString propertyName,QString elementName, QString text)
{
    //qDebug() << "taskSendNewText";
    INDI::BaseDevice *dp;
    dp = indiclient->getDevice(deviceName.toStdString().c_str());

    if (dp== nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << " device. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }
    ITextVectorProperty *prop = nullptr;
    prop= dp->getText(propertyName.toStdString().c_str());
    if (prop == nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << " property. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }

    for (int i=0;i<prop->ntp;i++) {
        if (strcmp(prop->tp[i].name, elementName.toStdString().c_str()) == 0) {
            indiclient->sendNewText(deviceName.toStdString().c_str(),propertyName.toStdString().c_str(),elementName.toStdString().c_str(),text.toStdString().c_str());
            return true;
        }
    }
    qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << "/" << elementName << " element. Aborting.";
    tasks =QQueue<Ttask>();
    return false;
}
bool Module::taskSendNewSwitch(QString deviceName,QString propertyName,QString elementName, ISState sw)
{
    //qDebug() << "taskSendNewSwitch";

    INDI::BaseDevice *dp;
    dp = indiclient->getDevice(deviceName.toStdString().c_str());

    if (dp== nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << " device. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }
    ISwitchVectorProperty *prop = nullptr;
    prop= dp->getSwitch(propertyName.toStdString().c_str());
    if (prop == nullptr)
    {
        qDebug() << "Error - unable to find " << deviceName << "/" << propertyName << " property. Aborting.";
        tasks =QQueue<Ttask>();
        return false;
    }

    for (int i=0;i<prop->nsp;i++) {
        if (strcmp(prop->sp[i].name, elementName.toStdString().c_str()) == 0) {
            prop->sp[i].s=sw;
            indiclient->sendNewSwitch(prop);
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
    if (tasks.size()<=1) {
        //qDebug() << "finished";
        setMyElt("statusprop","status","Idle");
        emit finished();
    }  else {
        //qDebug() << tasks.front().tasklabel << "finished";
        tasks.pop_front();
        setMyElt("statusprop","status",tasks.front().tasklabel);
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
                QString devicename,QString propertyname,QString elementname)
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
                QString devicename, QString propertyname, QString elementname,
                double value,QString text,ISState sw)
{
    //qDebug() << "addnewtask2";

    Ttask task;
    task.tasktype = tasktype;
    task.taskname = taskname;
    task.tasklabel = tasklabel;
    task.specific = specific;
    task.devicename = devicename;
    task.propertyname = propertyname;
    task.elementname = elementname;
    task.value= value;
    task.text = text;
    task.sw = sw;
    tasks.append(task);
}

void Module::dumpTasks(void)
{
    for (int i=0;i<tasks.size();i++) {
        qDebug() << tasks[i].tasktype << "-0-" << tasks[i].taskname << "-0-"  << tasks[i].tasklabel << "-0-"  << tasks[i].specific << \
                    "-0-"  << tasks[i].devicename << "-0-"  << tasks[i].propertyname << "-0-"  << tasks[i].elementname << "-0-"  \
                    "-0-"  << tasks[i].value << "-0-"  << tasks[i].text << "-0-"  << tasks[i].sw ;
    }
}


void    Module::createMyModule(QString modulelabel)
{
    props->createModule(modulename,modulelabel);
}
void    Module::deleteMyModule(void)
{
    props->deleteModule(modulename);
}
void    Module::createMyCateg(QString categname,  QString categlabel)
{
    props->createCateg(modulename,categname,categlabel);
}
void    Module::deleteMyCateg(QString categname)
{
    props->deleteCateg(modulename,categname);
}
void    Module::createMyGroup(QString categname, QString groupname,  QString grouplabel)
{
    props->createGroup(modulename,categname,groupname,grouplabel);
}
void    Module::deleteMyGroup(QString categname, QString groupname)
{
    props->deleteGroup(modulename,categname,groupname);
}
void    Module::createMyProp (QString propname,Prop    prop)
{
    props->createProp(modulename,propname,prop);
}
void    Module::createMyProp (QString propname,QString label,propType typ,QString categname,QString groupname,OPerm perm,OSRule rule,double timeout,OPState state,QString aux0,QString aux1)
{
    props->createProp(modulename,propname,label,typ,categname,groupname,perm,rule,timeout,state,aux0,aux1);
}
void    Module::deleteMyProp (QString propname)
{
    props->deleteProp(modulename,propname);
}
Prop    Module::getMyProp    (QString propname)
{
    return props->getProp(modulename,propname);
}
void    Module::setMyProp    (QString propname,Prop    prop)
{
    props->setProp(modulename ,propname,prop);
}
void    Module::appendMyElt  (QString propname,  QString txtname, QString text     , QString label, QString aux0,QString aux1)
{
    props->appendElt(modulename, propname, txtname, text, label, aux0, aux1);
}
void    Module::appendMyElt  (QString propname,  QString numname, double  num      , QString label, QString aux0,QString aux1)
{
    props->appendElt(modulename, propname, numname, num, label, aux0, aux1);
}
void    Module::appendMyElt  (QString propname,  QString swtname, OSState swt      , QString label, QString aux0,QString aux1)
{
    props->appendElt(modulename, propname, swtname, swt, label, aux0, aux1);
}
void    Module::appendMyElt  (QString propname,  QString lgtname, OPState lgt      , QString label, QString aux0,QString aux1)
{
    props->appendElt(modulename, propname, lgtname, lgt, label, aux0, aux1);
}
void    Module::deleteMyElt  (QString propname,  QString eltname)
{
    props->deleteElt(modulename,propname,eltname);
}
void    Module::setMyElt     (QString propname,  QString txtname, QString text)
{
    props->setElt(modulename, propname, txtname, text);
}
void    Module::setMyElt     (QString propname,  QString numname, double  num)
{
    props->setElt(modulename, propname, numname, num);
}
void    Module::setMyElt     (QString propname,  QString swtname, OSState swt)
{
    props->setElt(modulename, propname, swtname, swt);
}
void    Module::setMyElt     (QString propname,  QString lgtname, OPState lgt)
{
    props->setElt(modulename, propname, lgtname, lgt);
}
QString Module::getMyTxt     (QString propname,  QString txtname)
{
    return props->getTxt(modulename, propname, txtname);
}
double  Module::getMyNum     (QString propname,  QString numname)
{
    return props->getNum(modulename, propname, numname);
}
OSState Module::getMySwt     (QString propname,  QString swtname)
{
    return props->getSwt(modulename, propname, swtname);
}
OPState Module::getMyLgt     (QString propname,  QString lgtname)
{
    return props->getLgt(modulename, propname, lgtname);
}
