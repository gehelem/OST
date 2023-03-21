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
        foreach(const QString &keyprop, pEventData.keys())
        {
            if (pEventData[keyprop].toMap().contains("value"))
            {
                QVariant val = pEventData[keyprop].toMap()["value"];
                setOstPropertyValue(keyprop, val, true);
            }
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (keyelt == "load")
                {
                    setOstPropertyAttribute(keyprop, "status", 2, true);
                    if (setOstElementValue(keyprop, keyelt, false, true))
                    {
                        QString pp = keyprop;
                        QString elt = getOstPropertyValue(keyprop).toString();
                        QString eltwithoutblanks = getOstPropertyValue(keyprop).toString();
                        eltwithoutblanks.replace(" ", "");
                        QString prof = "default";
                        pp.replace("load", "");

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
            }
        }
    }
}
