#include "maincontrol.h"

Maincontrol *initialize(QString name,QString label,QString profile,QVariantMap availableModuleLibs)
{
    Maincontrol *basemodule = new Maincontrol(name,label,profile,availableModuleLibs);
    return basemodule;
}

Maincontrol::Maincontrol(QString name, QString label, QString profile,QVariantMap availableModuleLibs)
    : Basemodule(name,label,profile,availableModuleLibs)
{

    Q_INIT_RESOURCE(maincontrol);

    loadPropertiesFromFile(":maincontrol.json");
    setOstProperty("moduleDescription","Maincontrol",true);
    setOstProperty("version",0.1,true);

    foreach(QString key,getAvailableModuleLibs().keys()) {
        QVariantMap info = getAvailableModuleLibs()[key].toMap();
        createOstProperty("mod"+key,"mod"+key,0,"Available modules",info["moduleLabel"].toString());
    }



}

Maincontrol::~Maincontrol()
{
    Q_CLEANUP_RESOURCE(maincontrol);
}

void Maincontrol::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{
        //BOOST_LOG_TRIVIAL(debug) << "OnMyExternalEvent - recv : " << getName().toStdString() << "-" << eventType.toStdString() << "-" << eventKey.toStdString();
        foreach(const QString& keyprop, eventData.keys()) {
            foreach(const QString& keyelt, eventData[keyprop].toMap()["elements"].toMap().keys()) {


                if (keyprop=="devices") {
                    if (keyelt=="camera") {
                        if (setOstElement(keyprop,keyelt,eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"],false)) {
                        }
                    }
                }




            }

        }
}
