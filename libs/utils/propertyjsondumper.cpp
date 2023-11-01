#include "propertyjsondumper.h"
#include "valuejsondumper.h"
#include <sstream>
namespace  OST
{

QJsonObject PropertyJsonDumper::dumpPropertyCommons(PropertyBase *pProperty)
{
    QJsonObject json;

    json["label"] = pProperty->label();
    json["order"] = pProperty->order();
    json["level1"] = pProperty->level1();
    json["level2"] = pProperty->level2();
    json["status"] = pProperty->state();
    json["permission"] = pProperty->permission();
    json["hasprofile"] = pProperty->hasProfile();


    return json;

}
void PropertyJsonDumper::visit(PropertyMulti *pProperty)
{
    QJsonObject json = dumpPropertyCommons(pProperty);
    if (pProperty->hasArray())
    {
        json["hasArray"] = true;
        json["showArray"] = pProperty->getShowArray();
        json["arrayLimit"] = pProperty->getArrayLimit();
    }
    json["rule"] = pProperty->rule();
    QJsonObject elements;
    foreach(const QString &key, pProperty->getValues()->keys())
    {
        OST::ValueJsonDumper d;

        pProperty->getValue(key)->accept(&d);
        QJsonObject value = d.getResult();
        elements[key] = value;
    }
    json["elements"] = elements;
    mResult = json;
}

}
