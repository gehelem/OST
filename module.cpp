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
    connect(props,&Properties::signalAppendGraph,this,&Module::slotAppendGraph);

    //connect(properties,&OSTProperties::signalpropCreated,this,&Module::slotpropCreated);
    //connect(properties,&OSTProperties::signalpropDeleted,this,&Module::slotpropDeleted);


}
Module::~Module()
{
}
void Module::slotvalueChanged(Prop prop)
{
   if (prop.modulename==modulename) emit signalvalueChanged(prop);
}
void Module::slotAppendGraph(Prop prop,OGraph gra,OGraphValue val)
{
   if (prop.modulename==modulename) emit signalAppendGraph(prop,gra,val);
}
void Module::slotpropCreated(Prop prop)
{
   if (prop.modulename==modulename) emit signalpropCreated(prop);
}
void Module::slotpropDeleted(Prop prop)
{
   if (prop.modulename==modulename) emit signalpropDeleted(prop);
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
    if (    (tasks.front().tasktype==TT_WAIT_FRAME_RESET)
         && (strcmp(tasks.front().devicename.toStdString().c_str(),svp->device)==0)
         && (svp->s==IPS_OK)
         && (strcmp("CCD_FRAME_RESET",svp->name)==0)
       )
    {
        if (tasks.front().specific) {
            executeTaskSpec(tasks.front(),svp);
        } else {
            popnext();
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

    if (    (tasks.front().tasktype==TT_WAIT_FRAME_SET)
        && (strcmp(tasks.front().devicename.toStdString().c_str(),nvp->device)==0)
        //&& (nvp->s==IPS_OK)
        && (strcmp("CCD_FRAME",nvp->name)==0)
       )
    {
        if (nvp->s==IPS_OK) {
            if (tasks.front().specific) {
                executeTaskSpec(tasks.front(),nvp);
            } else {
                popnext();
            }
        }
        if (nvp->s==IPS_ALERT) {
            //tasks =QQueue<Ttask>();
            setMyElt("messages","messageselt","ERROR" + tasks.front().taskname +"-"+ tasks.front().tasklabel);
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
    if (task.tasktype==TT_FRAME_SET) {
        INDI::BaseDevice *dp;
        dp = indiclient->getDevice(task.devicename.toStdString().c_str());
        if (dp== nullptr)
        {
            qDebug() << "Error - unable to find " << task.devicename << " device. Aborting.";
            tasks =QQueue<Ttask>();
            return;
        }

        INumberVectorProperty *prop = nullptr;
        prop = dp->getNumber("CCD_FRAME");
        if (prop == nullptr)
        {
            qDebug() << "Error - unable to find " << task.devicename << "/" << "CCD_FRAME" << " property. Aborting.";
            tasks =QQueue<Ttask>();
            return ;
        }

        for (int i=0;i<prop->nnp;i++) {
            if (strcmp(prop->np[i].name, "X") == 0) {
                prop->np[i].value=task.value0;
            }
            if (strcmp(prop->np[i].name, "Y") == 0) {
                prop->np[i].value=task.value1;
            }
            if (strcmp(prop->np[i].name, "WIDTH") == 0) {
                prop->np[i].value=task.value2;
            }
            if (strcmp(prop->np[i].name, "HEIGHT") == 0) {
                prop->np[i].value=task.value3;
            }
        }

        indiclient->sendNewNumber(prop);
        popnext();
    }
    if (task.tasktype==TT_FRAME_RESET) {
        INDI::BaseDevice *dp;
        dp = indiclient->getDevice(task.devicename.toStdString().c_str());
        if (dp== nullptr)
        {
            qDebug() << "Error - unable to find " << task.devicename << " device. Aborting.";
            tasks =QQueue<Ttask>();
            return;
        }

        ISwitchVectorProperty *prop = nullptr;
        prop = dp->getSwitch("CCD_FRAME_RESET");
        if (prop == nullptr)
        {
            qDebug() << "Error - unable to find " << task.devicename << "/" << "CCD_FRAME_RESET" << " property. Aborting.";
            tasks =QQueue<Ttask>();
            return ;
        }

        for (int i=0;i<prop->nsp;i++) {
            if (strcmp(prop->sp[i].name, "RESET") == 0) {
                prop->sp[i].s=ISS_ON;
            }
        }

        indiclient->sendNewSwitch(prop);
        popnext();
    }

}
void Module::popnext(void)
{
    if (tasks.size()<=1) {
        //qDebug() << "finished";
        setMyElt("statusprop","status","Idle");
        setMyElt("messages","messageselt","Idle");
        emit finished();
    }  else {
        //qDebug() << tasks.front().tasklabel << "finished";
        tasks.pop_front();
        setMyElt("statusprop","status",tasks.front().tasklabel);
        setMyElt("messages","messageselt",tasks.front().taskname +"-"+ tasks.front().tasklabel);

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
void Module::addnewtaskFrameSet (QString taskname, QString tasklabel,bool specific,
                 QString devicename, double x,double y,double width,double height)
{

    Ttask task;
    task.tasktype = TT_FRAME_SET;
    task.taskname = taskname;
    task.tasklabel = tasklabel;
    task.specific = specific;
    task.devicename= devicename;
    task.value0=x;
    task.value1=y;
    task.value2=width;
    task.value3=height;
    tasks.append(task);
}
void Module::addnewtaskFrameReset (QString taskname, QString tasklabel,bool specific,
                 QString devicename)
{
    Ttask task;
    task.tasktype = TT_FRAME_RESET;
    task.taskname = taskname;
    task.tasklabel = tasklabel;
    task.specific = specific;
    task.devicename= devicename;
    tasks.append(task);
}
void Module::addnewtaskWaitFrameSetOk (QString taskname, QString tasklabel,bool specific,
                 QString devicename)
{
    Ttask task;
    task.tasktype = TT_WAIT_FRAME_SET;
    task.taskname = taskname;
    task.tasklabel = tasklabel;
    task.specific = specific;
    task.devicename= devicename;
    tasks.append(task);
}
void Module::addnewtaskWaitFrameResetOk (QString taskname, QString tasklabel,bool specific,
                 QString devicename)
{
    Ttask task;
    task.tasktype = TT_WAIT_FRAME_RESET;
    task.taskname = taskname;
    task.tasklabel = tasklabel;
    task.specific = specific;
    task.devicename= devicename;
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


void    Module::createMyModule(QString modulelabel,int order)
{
    props->createModule(modulename,modulelabel,order);
}
void    Module::deleteMyModule(void)
{
    props->deleteModule(modulename);
}
void    Module::createMyCateg(QString categname,  QString categlabel,int order)
{
    props->createCateg(modulename,categname,categlabel,order);
}
void    Module::deleteMyCateg(QString categname)
{
    props->deleteCateg(modulename,categname);
}
void    Module::createMyGroup(QString categname, QString groupname,  QString grouplabel,int order)
{
    props->createGroup(modulename,categname,groupname,grouplabel,order);
}
void    Module::deleteMyGroup(QString categname, QString groupname)
{
    props->deleteGroup(modulename,categname,groupname);
}
void    Module::createMyProp (QString propname,Prop    prop)
{
    props->createProp(modulename,propname,prop);
}
void    Module::createMyProp (QString propname, QString label, propType typ, QString categname, QString groupname, OPerm perm, OSRule rule, double timeout, OPState state, QString aux0, QString aux1, int order)
{
    props->createProp(modulename,propname,label,typ,categname,groupname,perm,rule,timeout,state,aux0,aux1,order);
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
void    Module::appendMyElt  (QString propname,  QString imgname, OImgType imt     , QString label, QString aux0, QString aux1, QString url, QString file)
{
    props->appendElt(modulename, propname, imgname, imt, label, aux0, aux1,url,file);
}
void    Module::appendMyElt  (QString propname,  QString graname, OGraph graph     )
{
    props->appendElt(modulename, propname, graname, graph);
}
void    Module::appendMyGra  (QString propname,  QString graname, OGraphValue val  )
{
    props->appendGra(modulename,propname,graname,val);
}
void    Module::resetMyGra   (QString propname,  QString graname)
{
    props->resetGra(modulename,propname,graname);
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
void    Module::setMyElt     (QString propname,  QString imgname, QString url, QString file)
{
    props->setElt(modulename, propname, imgname, url,file);
}
void    Module::setMyElt     (QString propname,  QString graname, OGraph  graph)
{
    props->setElt(modulename, propname, graname,graph);
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
OImage  Module::getMyImg     (QString propname,  QString imgname)
{
    return props->getImg(modulename, propname, imgname);
}
OGraph  Module::getMyGraph   (QString propname,  QString graname)
{
    return props->getGraph(modulename, propname, graname);
}
