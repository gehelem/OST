#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "jsondumper.h"
#include "model/numberproperty.h"
#include "model/textproperty.h"

namespace  OST
{



QJsonObject JSonDumper::dumpPropertyCommons(RootProperty *pProperty)
{

    QJsonObject json;

    json["level1"] = pProperty->level1();
    json["level2"] = pProperty->level2();
    json["label"] = pProperty->label();
    json["permission"] = pProperty->permission();
    json["state"] = pProperty->state();

    return json;
}
void JSonDumper::visit(BasicProperty *pProperty)
{
    _result = dumpPropertyCommons(pProperty);
}
void JSonDumper::visit(TextProperty *pProperty)
{
    _result = dumpPropertyCommons(pProperty);
}
void JSonDumper::visit(NumberProperty *pProperty)
{
    _result = dumpPropertyCommons(pProperty);
}
void JSonDumper::visit(MultiProperty *pProperty)
{
    QJsonObject json = dumpPropertyCommons(pProperty);
    QJsonObject jsonElements;
    foreach(const QString &key, pProperty->getElts().keys())
    {
        QJsonObject jsonElement;
        JSonDumper d;
        pProperty->getElts()[key]->accept(&d);
        jsonElements[key] = d.getResult();
    }
    json["elements"] = jsonElements;
    QJsonDocument doc(json);
    _result = json;
}
}

