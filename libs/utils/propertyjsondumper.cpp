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

    if (pProperty->getGridHeaders().size() > 0)
    {
        //qDebug() << "jsondump gridheader " << pProperty->key() << "-" << pProperty->getGridHeaders();
        json["gridheaders"] = QJsonArray::fromStringList(pProperty->getGridHeaders());
    }

    if (pProperty->getGrid().size() > 0)
    {
        QJsonArray grid;
        foreach(const QList<ValueBase*> &gridLine, pProperty->getGrid())
        {
            QJsonArray jLine;
            foreach(ValueBase* value, gridLine)
            {
                ValueJsonDumper d;
                value->accept(&d);
                jLine.append(d.getResult());
            }
            grid.append(jLine);
        }
        json["grid"] = grid;

    }





    mResult = json;
}

}
