#include "dummy.h"

Dummy *initialize(QString name,QString label)
{
    Dummy *basemodule = new Dummy(name,label);
    return basemodule;
}

Dummy::Dummy(QString name,QString label)
    : Basemodule(name,label)
{

    _moduledescription="Dummy module to show what we can do";

    loadPropertiesFromFile(":dummy.json");

    setOstProperty("moduleDescription","Dummy module to show what we can do");

    //createOstProperty("version","Version",0);
    setOstProperty("version",0.1);


    setOstProperty("message","Dummy module init finished");

    //saveAttributesToFile("dummy.json");

}

Dummy::~Dummy()
{

}

//void Dummy::OnSetPropertyText(TextProperty* prop)
//{
//}
