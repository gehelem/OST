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
    setOstPropertyValue("moduleDescription", "Maincontrol", false);
    setOstPropertyValue("moduleVersion", 0.1, false);
    deleteOstProperty("profileactions");

    foreach(QString key, getAvailableModuleLibs().keys())
    {
        QVariantMap info = getAvailableModuleLibs()[key].toMap();
        createOstProperty("desc" + key, "Description", 0, "Available modules", info["label"].toString());
        foreach (QString ii, info.keys())
        {
            QVariant val = info[ii];
            createOstElement("desc" + key, ii, ii, false);
            setOstElementValue("desc" + key, ii, val, false);
        }
        createOstProperty("load" + key, "", 2, "Available modules", info["label"].toString());
        createOstElement("load" + key, "instance", "Instance name", false);
        setOstElementValue("load" + key, "instance", "my" + key, false);
        createOstElement("load" + key, "load", "Load", false);
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
    //BOOST_LOG_TRIVIAL(debug) << "mainctl OnMyExternalEvent - recv : " << getName().toStdString() << "-" <<
    //                         eventType.toStdString() <<
    //                         "-" << eventKey.toStdString();
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
