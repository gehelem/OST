#include "dummy.h"

Dummy *initialize(QString name,QString label,QString profile)
{
    Dummy *basemodule = new Dummy(name,label,profile);
    return basemodule;
}

Dummy::Dummy(QString name, QString label, QString profile)
    : Basemodule(name,label,profile)
{

    Q_INIT_RESOURCE(dummy);
    _moduletype="dummy";

    loadPropertiesFromFile(":dummy.json");

    setOstProperty("moduleDescription","Dummy module to show what we can do",true);

    //createOstProperty("version","Version",0);
    setOstProperty("version",0.1,true);


    setOstProperty("message","Dummy module init finished",true);
    setOstElement("extextRO","extext1","Texte read only 1",false);
    setOstElement("extextRO","extext2","Texte read only 2",false);
    setOstElement("extextRO","extext3","Texte read only 3",false);
    setOstElement("extextRO","extext4","Texte read only 4",true);
    setOstElement("boolsRW","b1",true,false);
    setOstElement("boolsRW","b2",false,false);
    setOstElement("boolsRW","b3",true,false);
    setOstElement("boolsRW","b4",false,true);
    setOstElement("numbersRW","n1",0,false);
    setOstElement("numbersRW","n2",-1000,false);
    setOstElement("numbersRW","n3",3.14,false);
    setOstElement("numbersRW","n4",-20.23,true);
    setOstElementAttribute("numbersRW","n4","step",100,true);
    setOstElementAttribute("numbersRW","n4","min",-10000,true);
    setOstElementAttribute("numbersRW","n4","max",10000,true);

    //saveAttributesToFile("dummy.json");

}

Dummy::~Dummy()
{
    Q_CLEANUP_RESOURCE(dummy);
}

void Dummy::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{
        BOOST_LOG_TRIVIAL(debug) << "OnMyExternalEvent - recv : " << getName().toStdString() << "-" << eventType.toStdString() << "-" << eventKey.toStdString();
}
