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

    setOstProperty("moduleDescription","Dummy module to show what we can do");

    //createOstProperty("version","Version",0);
    setOstProperty("version",0.1);


    setOstProperty("message","Dummy module init finished");

    //saveAttributesToFile("dummy.json");

}

Dummy::~Dummy()
{
    Q_CLEANUP_RESOURCE(dummy);
}

void Dummy::OnExternalEvent(const QString &eventType, const QString &eventData)
{
    Basemodule *mod = qobject_cast<Basemodule *>(sender());

    BOOST_LOG_TRIVIAL(debug) << "OnExternalEvent  with override - recv = " << getName().toStdString() << " sender = " << mod->getName().toStdString();

}
