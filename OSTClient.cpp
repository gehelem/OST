#include "OSTParser.h"
#include "OSTClient.h"
#include <baseclientqt.h>

OSTClient::OSTClient(QObject *parent)
{
    properties = QJsonArray();
    groups = QJsonArray();
    categories = QJsonArray();

    thismodule =QJsonObject();
    thismodule["modulename"]="defaultmodule";
    thismodule["modulelabel"]="Default Module";
    thismodule["modulehasgroups"]="N";
    thismodule["modulehascategories"]="N";
    thismodule["properties"]=properties;
    thismodule["categories"]=categories;
    thismodule["groups"]=groups;


    setServer("localhost", 7624);
    connectServer();
    getDevices();

}
OSTClient::~OSTClient()
{
}
void OSTClient::serverConnected(void)
{
}
void OSTClient::serverDisconnected(int exit_code)
{
}
void OSTClient::newDevice(INDI::BaseDevice *dp)
{
}
void OSTClient::removeDevice(INDI::BaseDevice *dp)
{
}
void OSTClient::newProperty(INDI::Property *property)
{
}
void OSTClient::removeProperty(INDI::Property *property)
{
}
void OSTClient::newNumber(INumberVectorProperty *nvp)
{
    executecurrenttask(nvp,nullptr,nullptr,nullptr,nullptr);
}
void OSTClient::newText(ITextVectorProperty *tvp)
{
    executecurrenttask(nullptr,tvp,nullptr,nullptr,nullptr);
}
void OSTClient::newSwitch(ISwitchVectorProperty *svp)
{
    executecurrenttask(nullptr,nullptr,svp,nullptr,nullptr);
}
void OSTClient::newLight(ILightVectorProperty *lvp)
{
    executecurrenttask(nullptr,nullptr,nullptr,lvp,nullptr);
}
void OSTClient::newMessage(INDI::BaseDevice *dp, int messageID)
{
    messagelog(*dp->getDeviceName() + " - " + *dp->messageQueue(messageID).c_str());
}
void OSTClient::newBLOB(IBLOB *bp)
{
    executecurrenttask(nullptr,nullptr,nullptr,nullptr,bp);
}

void OSTClient::processTextMessage(QString message)
{
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8()); // garder
    QJsonObject  obj = jsonResponse.object(); // garder

        if (obj["module"].toString()==thismodule["modulename"].toString()) {
            IDLog("%s message received-------------------------------------\n",thismodule["modulename"].toString().toStdString().c_str());
            IDLog("%s\n",QJsonDocument(obj).toJson(QJsonDocument::Indented).toStdString().c_str());
            if (obj["message"].toString()=="readmodule") {
                updatemodule(thismodule["modulename"].toString());
            }

        }

}
void OSTClient::switchstate(std::string newstate)
{
    //IDLog("%s switchstate %s->%s\n",thismodule["modulename"].toString().toStdString().c_str(),newstate.c_str());
    state = newstate;
    //emit s_newstate();

}
QJsonObject OSTClient::getmodule  (QString module)
{
    QJsonObject result;
    QJsonArray props,cats;

    foreach (const QJsonValue & pro , properties) {
        if ((pro.toObject()["parenttype"].toString()=="M")&&(pro.toObject()["parentname"].toString()==module)) {
            props.append(pro.toObject()); //FIXME
        }
    }
    cats = QJsonArray();
    foreach (const QJsonValue & cat , categories) {
        if (cat.toObject()["modulename"].toString()==module) {
            cats.append(getcategory(module,cat.toObject()["categoryname"].toString()));
        }
    }
    result["modulename"]=thismodule["modulename"];
    result["modulelabel"]=thismodule["modulelabel"];
    result["modulehasgroups"]=thismodule["modulehasgroups"];
    result["modulehascategories"]=thismodule["modulehascategories"];

    if (props.size() > 0) result["properties"]=props;
    if (cats.size() > 0)  result["categories"]=cats;

    return result;


}
QJsonObject OSTClient::getcategory(QString module,QString category)
{
    QJsonObject result;
    foreach (const QJsonValue & cat , categories) {
        QJsonArray props;
        props=QJsonArray();
        if ((cat.toObject()["categoryname"].toString()==category)&&(cat.toObject()["modulename"].toString()==module)) {
            foreach (const QJsonValue & pro , properties) {
                if ((pro.toObject()["parenttype"].toString()=="C")&&(pro.toObject()["categoryname"].toString()==category)) {
                    props.append(pro.toObject());
                }
            }
            QJsonArray gros;
            gros=QJsonArray();
            foreach (const QJsonValue & gro , groups) {
                if ((gro.toObject()["categoryname"].toString()==category)&&(gro.toObject()["modulename"].toString()==module)) {
                    gros.append(getgroup(module,category,gro.toObject()["groupname"].toString()));
                }
            }


        result["modulename"]=cat.toObject()["modulename"];
        result["categoryname"]=cat.toObject()["categoryname"];
        result["categorylabel"]=cat.toObject()["categorylabel"];
        if (props.size()>0) result["properties"]=props;
        if (gros.size()>0) result["groups"]=gros;
        }
    }
    return result;

}
QJsonObject OSTClient::getgroup   (QString module,QString category,QString group)
{
    QJsonObject result;
    foreach (const QJsonValue & gro , groups) {
        if ((gro.toObject()["groupname"].toString()==group)&&(gro.toObject()["modulename"].toString()==module)) {
            QJsonArray props;
            foreach (const QJsonValue & pro , properties) {
                if ((pro.toObject()["parenttype"].toString()=="G")&&(pro.toObject()["parentname"].toString()==group)) {
                    props.append(getproperty(module,category,group,pro.toObject()["name"].toString()));
                }
            }
            result["modulename"]=gro.toObject()["modulename"];
            result["categoryname"]=gro.toObject()["categoryname"];
            result["groupname"]=gro.toObject()["groupname"];
            result["grouplabel"]=gro.toObject()["grouplabel"];
            if (props.size()>0) result["properties"]=props;
        }
    }
    return result;
}
QJsonObject OSTClient::getproperty   (QString module,QString category,QString group,QString property)
{
    foreach (const QJsonValue & pro , properties) {
        if ((pro.toObject()["name"].toString()==property)&&(pro.toObject()["modulename"].toString()==module)) {
            return pro.toObject();
        }
    }
    return QJsonObject();
}
void OSTClient::updateproperty(QString module,QString category,QString group,QString property)
{
    QJsonObject result;
    result["message"]="updateproperty";
    result["property"]=getproperty(module,category,group,property);
    //result =QJsonObject();
    emit sendjson(result);
}
void OSTClient::updategroup(QString module, QString category, QString group)
{
    QJsonObject result;
    result["message"]="updategroup";
    result["group"]=getgroup(module,category,group);
    emit sendjson(result);
}
void OSTClient::updatecategory(QString module, QString category)
{
    QJsonObject result;
    result["message"]="updatecategory";
    result["category"]=getcategory(module,category);
    emit sendjson(result);
}
void OSTClient::updatemodule(QString module)
{
    QJsonObject result;
    result["message"]="updatemodule";
    result["module"]=getmodule(module);
    emit sendjson(result);
}
void OSTClient::appendmessage(QString module, QString messagename,QString message)
{
    QJsonObject result;
    result["message"]="appendmessage";
    result["modulename"]=module;
    result["messagename"]=messagename;
    result["messagecontent"]=message;
    emit sendjson(result);

}
void OSTClient::messagelog(QString message) {
    QString sDate = QDateTime::currentDateTime().toString("yyyy MM dd hh:mm:ss.zzz");
    appendmessage(thismodule["modulename"].toString(),thismodule["modulename"].toString()+"MESSAGE", sDate + " - "+ message);
}
void OSTClient::addnewtask (    Tasktype tasktype,    bool executeimmediate,    QString jobname,    QString groupname,    QString taskname,    QString tasklabel,    QString modulename,    QString propertyname,    QString detailname)
{
    Ttask task;
    task.tasktype = tasktype;
    task.executeimmediate = executeimmediate;
    task.jobname = jobname;
    task.groupname = groupname;
    task.taskname = taskname;
    task.tasklabel = tasklabel;
    task.modulename = modulename;
    task.propertyname = propertyname;
    task.detailname = detailname;
    tasks.append(task);

}
void OSTClient::addnewtask (Tasktype tasktype,    bool executeimmediate,    QString jobname,    QString groupname,    QString taskname,    QString tasklabel,    QString modulename,    QString propertyname,    QString detailname,    double value,    QString text,    ISState sw)
{
    Ttask task;
    task.tasktype = tasktype;
    task.executeimmediate= executeimmediate;
    task.jobname = jobname;
    task.groupname = groupname;
    task.taskname = taskname;
    task.tasklabel = tasklabel;
    task.modulename = modulename;
    task.propertyname = propertyname;
    task.detailname = detailname;
    task.value = value;
    task.text = text;
    task.sw = sw;
    tasks.append(task);

}
void OSTClient::popnext(void)
{
    if (tasks.size()<=1) {
        messagelog(tasks.front().tasklabel + " finished - Idle");
        tasks=QQueue<Ttask>();
        return;
    }
    tasks.pop_front();
    if (tasks.front().executeimmediate) executecurrenttask();


}
void OSTClient::executecurrenttask(INumberVectorProperty *nvp,ITextVectorProperty *tvp,ISwitchVectorProperty *svp,ILightVectorProperty *lvp,IBLOB *bp)
{
    //IDLog("executecurrenttask\n");
    if (!tasks.size()) return;
    Ttask task=tasks.front();
    messagelog(task.tasklabel);

    switch (task.tasktype) {
        case TT_SPEC: {
            //IDLog("TT_SPEC executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());
            executespecificcurrenttask(nvp,tvp,svp,lvp,bp);
            //popnext();
            break;
        }

        case TT_SEND_NUMBER: {
            //IDLog("**********TT_SEND_NUMBER executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());
            INumberVectorProperty *number = nullptr;
            number = getDevice(task.modulename.toStdString().c_str())->getNumber(task.propertyname.toStdString().c_str());
            if (number == nullptr)
            {
                IDLog("Error - unable to find %s - %s property : aborting\n",task.modulename.toStdString().c_str(),task.propertyname.toStdString().c_str());
                tasks =QQueue<Ttask>();
                break;
            }
            bool found = false;
            for (int i=0;i<number->nnp;i++) {
                if (strcmp(number->np[i].name, task.detailname.toStdString().c_str()) == 0) {
                    number->np[i].value=task.value;
                    sendNewNumber(number);
                    popnext();
                    found = true;
                }
            }
            if (found) break;
            IDLog("Error - unable to find %s - %s - %s number : aborting\n",task.modulename.toStdString().c_str(),task.propertyname.toStdString().c_str(),task.detailname.toStdString().c_str());
            tasks =QQueue<Ttask>();
            break;
        }

        case TT_SEND_TEXT: {
            //IDLog("**********TT_SEND_TEXT executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());
            ITextVectorProperty *text = nullptr;
            text = getDevice(task.modulename.toStdString().c_str())->getText(task.propertyname.toStdString().c_str());
            if (text == nullptr)
            {
                IDLog("Error - unable to find %s - %s property : aborting\n",task.modulename.toStdString().c_str(),task.propertyname.toStdString().c_str());
                tasks =QQueue<Ttask>();
                break;
            }
            bool found = false;
            for (int i=0;i<text->ntp;i++) {
                if (strcmp(text->tp[i].name, task.detailname.toStdString().c_str()) == 0) {
                    text->tp[i].text=(char *)task.text.toStdString().c_str();
                    sendNewText(text);
                    popnext();
                    found = true;
                }
            }
            if (found) break;
            IDLog("Error - unable to find %s - %s - %s text : aborting\n",task.modulename.toStdString().c_str(),task.propertyname.toStdString().c_str(),task.detailname.toStdString().c_str());
            tasks =QQueue<Ttask>();
            break;
        }

        case TT_SEND_SWITCH: {
            //IDLog("**********TT_SEND_SWITCH executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());

            ISwitchVectorProperty *sw = nullptr;
            sw = getDevice(task.modulename.toStdString().c_str())->getSwitch(task.propertyname.toStdString().c_str());
            if (sw == nullptr)
            {
                IDLog("Error - unable to find %s - %s property : aborting\n",task.modulename.toStdString().c_str(),task.propertyname.toStdString().c_str());
                tasks =QQueue<Ttask>();
                break;
            }
            bool found = false;
            for (int i=0;i<sw->nsp;i++) {
                if (strcmp(sw->sp[i].name, task.detailname.toStdString().c_str()) == 0) {
                    sw->sp[i].s=task.sw;
                    sendNewSwitch(sw);
                    popnext();
                    found = true;
                }
            }
            if (found) break;
            IDLog("Error - unable to find %s - %s - %s switch : aborting\n",task.modulename.toStdString().c_str(),task.propertyname.toStdString().c_str(),task.detailname.toStdString().c_str());
            tasks =QQueue<Ttask>();
            break;
        }

        case TT_ANALYSE_SEP: {
        //IDLog("**********TT_ANALYSE_SEP executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());

            image->FindStars();
            popnext();
            break;
        }

        case TT_ANALYSE_SOLVE: {
        //IDLog("**********TT_ANALYSE_SOLVE executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());
            image->FindStars();
            popnext();
            break;
        }

        case TT_WAIT_NUMBER: {
        //IDLog("**********TT_WAIT_NUMBER executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());
            bool found = false;
            if ((strcmp(nvp->device, task.modulename.toStdString().c_str()) == 0)&&
                (strcmp(nvp->name, task.propertyname.toStdString().c_str()) == 0)){
                for (int i=0;i<nvp->nnp;i++) {
                    if ((strcmp(nvp->np[i].name, task.detailname.toStdString().c_str()) == 0)
                      &&(nvp->np[i].value==task.value)){
                        found=true;
                    }
                }
            }
            if (found) popnext();
            break;
        }

        case TT_WAIT_TEXT: {
        //IDLog("********** TT_WAIT_TEXTexecuting %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());
            bool found = false;
            if ((strcmp(tvp->device, task.modulename.toStdString().c_str()) == 0)&&
                (strcmp(tvp->name, task.propertyname.toStdString().c_str()) == 0)){
                for (int i=0;i<tvp->ntp;i++) {
                    if ((strcmp(tvp->tp[i].name, task.detailname.toStdString().c_str()) == 0)
                      &&(strcmp(tvp->tp[i].text, task.text.toStdString().c_str()) == 0)) {
                        found=true;
                    }
                }
            }
            if (found) popnext();
            break;
        }

        case TT_WAIT_SWITCH: {
        //IDLog("**********TT_WAIT_SWITCH executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());
            bool found = false;
            if ((strcmp(svp->device, task.modulename.toStdString().c_str()) == 0)&&
                (strcmp(svp->name, task.propertyname.toStdString().c_str()) == 0)){
                for (int i=0;i<svp->nsp;i++) {
                    if ((strcmp(svp->sp[i].name, task.detailname.toStdString().c_str()) == 0)
                            &&(svp->sp[i].s==task.sw)){
                        found=true;
                    }
                }
            }
            if (found) popnext();
            break;
        }

    case TT_WAIT_SEP: {
        //IDLog("**********TT_WAIT_SEP executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());
            /* dostuff */
            popnext();
            break;
        }
    case TT_WAIT_SOLVE: {
        //IDLog("**********TT_WAIT_SOLVE executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());
            /* dostuff */
            popnext();
            break;
        }

    case TT_WAIT_BLOB: {
        //IDLog("**********TT_WAIT_BLOB executing %s %s %s\n",task.jobname.toStdString().c_str(),task.groupname.toStdString().c_str(),task.taskname.toStdString().c_str());
        if (bp == nullptr) break;
            if (strcmp(bp->bvp->name, task.modulename.toStdString().c_str()) == 0) {
                image.reset(new OSTImage());
                connect(image.get(),&OSTImage::success,this,&OSTClient::analysefinished);
                image->LoadFromBlob(bp);
                popnext();
                break;
            }
            break;
        }

    }


}


void OSTClient::analysefinished()
{
    if (image->FindStarsFinished) executecurrenttask();
}
