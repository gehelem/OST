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
    setOstPropertyValue("moduleLabel", "Main control", false);
    setOstPropertyValue("moduleDescription", "Maincontrol module - this one should always be there", false);
    setOstPropertyValue("moduleVersion", 0.1, false);
    deleteOstProperty("saveprofile");
    deleteOstProperty("loadprofile");
    deleteOstProperty("moduleactions");

    foreach(QString key, getAvailableModuleLibs().keys())
    {
        QVariantMap info = getAvailableModuleLibs()[key].toMap();
        createOstProperty( "load" + key, info["moduleDescription"].toMap()["value"].toString(), 2, "Available modules", "");
        setOstPropertyValue("load" + key, "My " + key, false);
        //createOstElement(  "load" + key, "instance", "Instance name", false);
        //setOstElementValue("load" + key, "instance", "My " + key, false);
        createOstElement(  "load" + key, "load", "Load", false);
        setOstElementValue("load" + key, "load", false, false);

    }
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
                setOstPropertyValue(keyprop, val, true);
                setOstPropertyValue("saveconf", val, true);

            }
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (keyelt == "load" && keyprop == "loadconf")
                {
                    emit loadConf(getOstPropertyValue("loadconf").toString());

                }
                if (keyelt == "refresh" && keyprop == "loadconf")
                {
                    setConfigurations();
                    setOstPropertyAttribute(keyprop, "status", 1, true);
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

    clearOstLov("loadconf");
    for(QVariantMap::const_iterator iter = confs.begin(); iter != confs.end(); ++iter)
    {
        //qDebug() << iter.key() << iter.value();
        addOstLov("loadconf", iter.key(), iter.key() );
    }
    sendMessage("Available configurations refreshed");
}

