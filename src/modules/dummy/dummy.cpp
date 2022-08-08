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

    //saveAttributesToFile("dummy.json");

}

Dummy::~Dummy()
{
    Q_CLEANUP_RESOURCE(dummy);
}

void Dummy::OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData)
{
    Q_UNUSED(eventType);Q_UNUSED(eventModule);Q_UNUSED(eventKey);Q_UNUSED(eventData);
    //BOOST_LOG_TRIVIAL(debug) << "OnMyExternalEvent - recv : " << getName().toStdString() << " eventType : " << eventType.toStdString();

}
