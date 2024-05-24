#include "propertyjsondumper.h"
#include "elementjsondumper.h"
#include <valuejsondumper.h>
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
    json["showElts"] = pProperty->getShowElts();
    json["hasGrid"] = pProperty->hasGrid();
    if (pProperty->hasGrid())
    {
        json["showGrid"] = pProperty->getShowGrid();
        json["gridLimit"] = pProperty->getGridLimit();
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

    if (pProperty->getGridHeaders().size() > 0)
    {
        //qDebug() << "jsondump gridheader " << pProperty->key() << "-" << pProperty->getGridHeaders();
        json["gridheaders"] = QJsonArray::fromStringList(pProperty->getGridHeaders());
    }

    if (pProperty->hasGrid())
    {
        QJsonArray grid;
        foreach(const QList<ValueBase*> &gridLine, pProperty->getGrid())
        {
            QJsonArray jLine;
            int i = 0;
            foreach(QString elt, pProperty->getGridHeaders())
            {
                ValueJsonDumper d;
                gridLine[i]->accept(&d);
                jLine.append(d.getResult());
                i++;
            }


            grid.append(jLine);
        }
        json["grid"] = grid;

    }

    json["hasGraph"] = pProperty->hasGraph();
    if (pProperty->hasGraph())
    {
        json["graphType"] = GraphTypeToString(pProperty->getGraphDefs().type);
        json["graphParams"] = QJsonObject::fromVariantMap(pProperty->getGraphDefs().params);
    }

    mResult = json;

}

}
