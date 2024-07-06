#include "propertyfactory.h"
namespace  OST
{

PropertyMulti *PropertyFactory::createProperty(const QString &pKey, const QVariantMap &pData)
{
    //qDebug() << "PropertyFactory::createProperty - " << pData;
    auto *pProperty = new PropertyMulti(pKey,
                                        pData["label"].toString(),
                                        IntToPermission(pData["permission"].toInt()),
                                        pData["devcat"].toString(),
                                        pData["group"].toString(),
                                        pData["order"].toString(),
                                        pData["hasprofile"].toBool(),
                                        pData["hasGrid"].toBool()
                                       );
    if (pData.contains("rule"))
    {
        pProperty->setRule(IntToRule(pData["rule"].toInt()));
    }
    else
    {
        pProperty->setRule(Any);
    }
    if (!pData.contains("elements"))
    {
        qDebug() << "Multiproperty defined without elements " << pData;
        return pProperty;
    }
    if (pData.contains("hasGrid"))
    {
        pProperty->setHasGrid(pData["hasGrid"].toBool());
    }
    if (pData.contains("showGrid"))
    {
        pProperty->setShowGrid(pData["showGrid"].toBool());
    }
    if (pData.contains("showElts"))
    {
        pProperty->setShowElts(pData["showElts"].toBool());
    }
    if (pData.contains("arrayLimit"))
    {
        pProperty->setGridLimit(pData["gridLimit"].toInt());
    }
    if (pData.contains("graphType") && !pProperty->hasGrid())
    {
        qDebug() << "Graph defined without grid " << pProperty->key();
    }
    if (pData.contains("autoupdown"))
    {
        pProperty->setAutoUpDown(pData["autoupdown"].toBool());
    }
    if (pData.contains("autoselect"))
    {
        pProperty->setAutoSelect(pData["autoselect"].toBool());
    }
    if (pData.contains("graphType") && pProperty->hasGrid())
    {
        if (!pData.contains("graphParams"))
        {
            qDebug() << "Graph defined without params " << pProperty->key();
        }
        else
        {
            GraphDefs d;
            d.type = StringToGraphType(pData["graphType"].toString());
            d.params = pData["graphParams"].toMap();
            pProperty->setGraphDefs(d);
        }
    }
    if (pData.contains("badge"))
    {
        pProperty->setBadge(pData["badge"].toBool());
    }
    QVariantMap elts = pData["elements"].toMap();
    foreach(const QString &key, elts.keys())
    {
        QVariantMap elt = elts[key].toMap();
        ElementBase *v = ElementFactory::createElement(elt);
        if (v != nullptr)
        {
            pProperty->addElt(key, v);
        }
    }
    if (pData.contains("grid"))
    {
        QJsonArray arr = pData["grid"].toJsonArray();
        if (!pData.contains("gridheaders"))
        {
            qDebug() << "Can't initialize grid without gridheaders definitions " << pKey;
        }
        else
        {
            QJsonArray headers = pData["gridheaders"].toJsonArray();
            for(int i = 0; i < arr.size(); i++)
            {
                QJsonArray line = arr[i].toArray();
                for(int j = 0; j < headers.size(); j++)
                {
                    pProperty->setElt(headers[j].toString(), line[j].toVariant());
                }
                pProperty->push();

            }

        }

    }
    if (pData.contains("preicon1"))
    {
        pProperty->setPreIcon1(pData["preicon1"].toString());
    }
    if (pData.contains("preicon2"))
    {
        pProperty->setPreIcon2(pData["preicon2"].toString());
    }
    if (pData.contains("posticon1"))
    {
        pProperty->setPostIcon1(pData["posticon1"].toString());
    }
    if (pData.contains("posticon2"))
    {
        pProperty->setPostIcon2(pData["posticon2"].toString());
    }

    return pProperty;


    qDebug() << "Can't create property " << pData;
    return nullptr;

}

}
