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
    deleteOstProperty("profileactions");
    deleteOstProperty("moduleactions");

    foreach(QString key, getAvailableModuleLibs().keys())
    {
        QVariantMap info = getAvailableModuleLibs()[key].toMap();
        createOstProperty( "load" + key, info["moduleDescription"].toMap()["value"].toString(), 2, "Available modules", "");
        createOstElement(  "load" + key, "instance", "Instance name", false);
        setOstElementValue("load" + key, "instance", "My " + key, false);
        createOstElement(  "load" + key, "load", "Load", false);
        setOstElementValue("load" + key, "load", false, false);

    }



}

Maincontrol::~Maincontrol()
{
    Q_CLEANUP_RESOURCE(maincontrol);
}

void Maincontrol::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                                    const QVariantMap &eventData)
{
    Q_UNUSED(eventType);
    Q_UNUSED(eventKey);
    //sendMessage("mainctl OnMyExternalEvent - recv : " + getModuleName()+ "-" +eventType +"-" + eventKey);
    if (getModuleName() == eventModule)
    {
        foreach(const QString &keyprop, eventData.keys())
        {
            foreach(const QString &keyelt, eventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (keyelt == "instance")
                {
                    if (setOstElementValue(keyprop, keyelt, eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"], true))
                    {
                    }
                }
                if (keyelt == "load")
                {
                    if (setOstElementValue(keyprop, keyelt, eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"], true))
                    {
                        QString pp = keyprop;
                        QString elt = getOstElementValue(keyprop, "instance").toString();
                        QString eltwithoutblanks = getOstElementValue(keyprop, "instance").toString();
                        eltwithoutblanks.replace(" ", "");
                        QString prof = "default";
                        pp.replace("load", "");

                        emit loadOtherModule(pp,
                                             eltwithoutblanks,
                                             elt,
                                             prof);
                    }
                }


                if (keyprop == "devices")
                {
                }

            }

        }

    }
}
