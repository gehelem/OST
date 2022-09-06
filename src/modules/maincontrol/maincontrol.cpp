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
    setOstProperty("moduleLabel","Main control",false);
    setOstProperty("moduleDescription","Maincontrol",false);
    setOstProperty("moduleVersion",0.1,false);

    foreach(QString key,getAvailableModuleLibs().keys()) {
        QVariantMap info = getAvailableModuleLibs()[key].toMap();
        BOOST_LOG_TRIVIAL(debug) << "lst : " << key.toStdString();
        QString err;
        createOstProperty("desc"+key,"Description",0,"Available modules",info["moduleLabel"].toString(),err);
        createOstProperty("desc"+key,"Description",0,"Available modules",info["moduleLabel"].toString(),err);
        foreach (QString ii,info.keys()) {
            QVariant val=info[ii];
            createOstElement("desc"+key,ii,ii,false);
            setOstElement("desc"+key,ii,val,false);
        }
        createOstProperty("load"+key,"",2,"Available modules",info["moduleLabel"].toString(),err);
        createOstElement("load"+key,"instance","Instance name",false);
        setOstElement("load"+key,"instance","my"+key,false);
        createOstElement("load"+key,"load","Load",false);
        setOstElement("load"+key,"load",false,false);

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
                if (keyelt=="instance") {
                    if (setOstElement(keyprop,keyelt,eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"],true)) {
                    }
                }
                if (keyelt=="load") {
                    if (setOstElement(keyprop,keyelt,eventData[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"],true)) {
                        QString pp = keyprop;
                        QString elt = getOstElementValue(keyprop,"instance").toString();
                        QString prof = "default";
                        pp.replace("load","");

                        emit loadOtherModule(pp,
                                             elt.replace(" ",""),
                                             elt,
                                             prof);
                    }
                }


                if (keyprop=="devices") {
                }




            }

        }
}
