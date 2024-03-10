#include "propertyjsondumper.h"
#include "elementjsondumper.h"
#include "elementupdate.h"
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
    json["badge"] = pProperty->getBadge();

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
    foreach(const QString &key, pProperty->getElts()->keys())
    {
        OST::ElementJsonDumper d;

        pProperty->getElt(key)->accept(&d);
        QJsonObject value = d.getResult();
        elements[key] = value;
    }
    json["elements"] = elements;
    QJsonObject grids;
    foreach(const QString &key, pProperty->getGrids().keys())
    {
        OST::GridJsonDumper d;

        QString action = "";
        int line = 0;
        pProperty->getGrids()[key]->accept(&d, action, line);
        QJsonObject grid = d.getResult();
        grids[key] = grid;
    }

    json["grids"] = grids;
    mResult = json;
}

}
