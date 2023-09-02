#include "maincontrol.h"

Maincontrol *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Maincontrol *basemodule = new Maincontrol(name, label, profile, availableModuleLibs);
    return basemodule;
}

Maincontrol::Maincontrol(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : Basemodule(name, label, profile, availableModuleLibs)
{

    Q_INIT_RESOURCE(maincontrol);
    setClassName(metaObject()->className());

    loadOstPropertiesFromFile(":maincontrol.json");
    setOstElementValue("moduleInfo", "moduleLabel", "Main control", false);
    setOstElementValue("moduleInfo", "moduleDescription", "Maincontrol module - this one should always be there", false);
    setOstElementValue("moduleInfo", "moduleVersion", 0.1, false);
    deleteOstProperty("saveprofile");
    deleteOstProperty("loadprofile");
    deleteOstProperty("moduleactions");
}

Maincontrol::~Maincontrol()
{
    Q_CLEANUP_RESOURCE(maincontrol);
}

void Maincontrol::OnMyExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                    const QVariantMap &pEventData)
{
    Q_UNUSED(pEventType);
    Q_UNUSED(pEventKey);
    //sendMessage("mainctl OnMyExternalEvent - recv : " + getModuleName()+ "-" +eventType +"-" + eventKey);
    if (getModuleName() == pEventModule)
    {
        if (pEventType == "refreshConfigurations")
        {
            setConfigurations();
        }
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (pEventData[keyprop].toMap().contains("value"))
            {
                QVariant val = pEventData[keyprop].toMap()["value"];
                //setOstPropertyValue(keyprop, val, true);
                if (keyprop == "loadconf")
                {
                    //setOstPropertyValue("saveconf", val, true);
                }
            }
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (keyelt == "name" && keyprop == "loadconf")
                {
                    QString val = pEventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"].toString();
                    getValueString("loadconf", "name")->setValue(val, true);
                    getValueString("saveconf", "name")->setValue(val, true);
                }
                if (keyelt == "load" && keyprop != "loadconf")
                {
                    if (setOstElementValue(keyprop, keyelt, false, true))
                    {
                        QString pp = keyprop;
                        QString elt = getString(keyprop, "name");
                        QString eltwithoutblanks = getString(keyprop, "name");
                        eltwithoutblanks.replace(" ", "");
                        QString prof = "default";
                        pp.replace("loadlibost", "");

                        emit loadOtherModule(pp,
                                             eltwithoutblanks,
                                             elt,
                                             prof);
                        setOstPropertyAttribute(keyprop, "status", 1, true);
                    }
                    else
                    {
                        setOstPropertyAttribute(keyprop, "status", 3, true);
                    }

                }
                if (keyelt == "load" && keyprop == "loadconf")
                {
                    bool val = pEventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"].toBool();
                    if (val) emit mainCtlEvent("loadconf", QString(), getString("loadconf", "name"), QVariantMap());
                }
                if (keyelt == "refresh" && keyprop == "loadconf")
                {
                    setConfigurations();
                    setOstPropertyAttribute(keyprop, "status", 1, true);
                }
                if (keyelt == "save" && keyprop == "saveconf")
                {

                    emit mainCtlEvent("saveconf", QString(), getString("saveconf", "value"), QVariantMap());
                }
                if (keyelt == "kill" && keyprop == "killall")
                {
                    emit mainCtlEvent("killall", QString(), QString(), QVariantMap());
                }
            }
        }
    }
}
void Maincontrol::setConfigurations(void)
{
    QVariantMap confs;
    if (!getDbConfigurations( confs))
    {
        sendError("Can't refresh available configurations");
        return;
    }

    getValueString("loadconf", "name")->lov.clear();
    for(QVariantMap::const_iterator iter = confs.begin(); iter != confs.end(); ++iter)
    {
        getValueString("loadconf", "name")->lov.add(iter.key(), iter.key());
    }
    sendMessage("Available configurations refreshed");
}

void Maincontrol::sendMainMessage(const QString &pMessage)
{
    sendMessage(pMessage);
}
void Maincontrol::sendMainError(const QString &pMessage)
{
    sendError(pMessage);
}
void Maincontrol::sendMainWarning(const QString &pMessage)
{
    sendWarning(pMessage);
}
void Maincontrol::sendMainConsole(const QString &pMessage)
{
    sendConsole(pMessage);
}
void Maincontrol::setAvailableModuleLibs(const QVariantMap libs)
{
    foreach(QString key, libs.keys())
    {
        QVariantMap info = libs[key].toMap()["elements"].toMap();
        QString lab = info["moduleDescription"].toMap()["value"].toString();
        OST::PropertyMulti *dynprop = new OST::PropertyMulti("load" + key, lab, OST::Permission::ReadWrite, "Available modules",
                "", "", false, false);
        OST::ValueBool* dynbool = new OST::ValueBool("Load", "", "");
        OST::ValueString* dyntext = new OST::ValueString("Name", "", "");
        dynbool->setValue(false);
        dyntext->setValue("My " + key, false);
        dynprop->addValue("name", dyntext);
        dynprop->addValue("load", dynbool);
        createProperty("load" + key, dynprop);

    }
}
void Maincontrol::addModuleData(const QString  &pName, const QString  &pLabel, const QString  &pType,
                                const QString  &pProfile)
{
    setOstElementValue("modules", "name", pName, false);
    setOstElementValue("modules", "label", pLabel, false);
    setOstElementValue("modules", "type", pType, false);
    setOstElementValue("modules", "profile", pProfile, false);
    getStore()["modules"]->push();
}
void Maincontrol::setModuleData(const QString  &pName, const QString  &pLabel, const QString  &pType,
                                const QString  &pProfile)
{
    Q_UNUSED(pLabel);
    Q_UNUSED(pType);
    QVariantList l = getOstElementGrid("modules", "name");
    for (int i = 0; i < l.count(); i++)
    {
        if( l[i].toString() == pName)
        {
            setOstElementLineValue("modules", "profile", i, pProfile);
        }
    }

}
void Maincontrol::deldModuleData(const QString  &pName)
{
    QVariantList l = getOstElementGrid("modules", "name");
    for (int i = 0; i < l.count(); i++)
    {
        if( l[i].toString() == pName)
        {
            getStore()["modules"]->deleteLine(i);
        }
    }

}
