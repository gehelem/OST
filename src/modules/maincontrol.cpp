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
    //sendMessage("mainctl OnMyExternalEvent - recv : " + getModuleName() + "-" + pEventType + "-" + pEventKey);

    if (getModuleName() == pEventModule)
    {
        if (pEventType == "refreshConfigurations")
        {
            setConfigurations();
        }
        if ((pEventType == "Fposticon") && (pEventData.contains("load")))
        {
            QVariantMap m = pEventData["load"].toMap()["elements"].toMap();
            QString pp = m.firstKey();
            pp.replace("libost", "");
            QString elt = getString("load", m.firstKey());
            QString eltwithoutblanks = getString("load", m.firstKey());
            eltwithoutblanks.replace(" ", "");
            QString prof = "default";


            emit loadOtherModule(pp,
                                 eltwithoutblanks,
                                 elt,
                                 prof);
            getProperty("load")->setState(OST::Ok);
        }
        if ((pEventType == "Fposticon") && (pEventData.contains("saveconf")))
        {
            emit mainCtlEvent("saveconf", QString(), getString("saveconf", "name"), QVariantMap());
            setConfigurations();
        }
        if ((pEventType == "Fposticon") && (pEventData.contains("loadconf")))
        {
            emit mainCtlEvent("loadconf", QString(), getString("loadconf", "name"), QVariantMap());
        }
        if ((pEventType == "Fpreicon") && (pEventData.contains("loadconf")))
        {
            setConfigurations();
        }
        foreach(const QString &keyprop, pEventData.keys())
        {
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (keyelt == "name" && keyprop == "loadconf" && pEventType == "Fsetproperty")
                {

                    QString val = pEventData[keyprop].toMap()["elements"].toMap()["name"].toMap()["value"].toString();
                    getValueString("loadconf", "name")->setValue(val, true);
                    getValueString("saveconf", "name")->setValue(val, true);
                }
                if (keyelt == "kill" && keyprop == "killall")
                {
                    emit mainCtlEvent("killall", QString(), QString(), QVariantMap());
                }
                if (keyprop == "indidrivers" && pEventType == "Fposticon")
                {
                    emit mainCtlEvent("startindidriver", QString(), keyelt, QVariantMap());
                }
                if (keyprop == "indidrivers" && pEventType == "Fpreicon")
                {
                    emit mainCtlEvent("stopindidriver", QString(), keyelt, QVariantMap());
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

    getValueString("loadconf", "name")->lovClear();
    for(QVariantMap::const_iterator iter = confs.begin(); iter != confs.end(); ++iter)
    {
        getValueString("loadconf", "name")->lovAdd(iter.key(), iter.key());
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

    OST::PropertyMulti *dynprop = new OST::PropertyMulti("load", "Available modules", OST::Permission::ReadWrite,
            "Available modules",
            "", "", false, false);
    foreach(QString key, libs.keys())
    {
        QVariantMap info = libs[key].toMap()["elements"].toMap();
        QString lab = info["moduleDescription"].toMap()["value"].toString();
        OST::ValueString* dyntext = new OST::ValueString(lab, "", "");
        dyntext->setValue("My " + key, false);
        dyntext->setAutoUpdate(true);
        dyntext->setDirectEdit(true);
        dyntext->setPostIcon("forward");
        dynprop->addValue(key, dyntext);

    }
    createProperty("load", dynprop);

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
            getValueString("modules", "profile")->getGrid()[i] = pProfile;
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
void Maincontrol::addIndiServerProperties(const QStringList  pDrivers)
{
    qDebug() << pDrivers;
    OST::PropertyMulti *dynprop = new OST::PropertyMulti("indidrivers", "Available indi drivers", OST::Permission::ReadOnly,
            "Indi server",
            "", "", false, false);
    foreach(QString key, pDrivers)
    {
        OST::ValueString* dyntext = new OST::ValueString(key, "", "");
        dyntext->setValue("", false);
        dyntext->setAutoUpdate(true);
        dyntext->setDirectEdit(true);
        dyntext->setPreIcon("stop");
        dyntext->setPostIcon("play_arrow");
        dynprop->addValue(key, dyntext);

    }
    createProperty("indidrivers", dynprop);
}
