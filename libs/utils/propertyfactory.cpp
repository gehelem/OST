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
                                        pData["hasArray"].toBool()
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
    if (pData.contains("showArray"))
    {
        pProperty->setShowArray(pData["showArray"].toBool());
    }
    if (pData.contains("arrayLimit"))
    {
        pProperty->setArrayLimit(pData["arrayLimit"].toInt());
    }
    if (pData.contains("badge"))
    {
        pProperty->setBadge(pData["badge"].toBool());
    }
    QVariantMap elts = pData["elements"].toMap();
    foreach(const QString &key, elts.keys())
    {
        QVariantMap elt = elts[key].toMap();
        if (pProperty->hasArray()) elt["arrayLimit"] = pProperty->getArrayLimit();
        ElementBase *v = ElementFactory::createElement(elt);
        if (v != nullptr)
        {
            pProperty->addElt(key, v);
        }
    }
    if (pData.contains("grid"))
    {
        QJsonArray arr = pData["grid"].toJsonArray();
        for(int i = 0; i < arr.size(); i++)
        {
            QJsonArray line = arr[i].toArray();
            qDebug() << "Grid content " << pProperty->key() << "(" << i << ") : " << line;
            for(int j = 0; j < line.size(); j++)
            {
                pProperty->setElt(pProperty->getGridHeaders()[j], line[j].toVariant());
            }
            pProperty->push();

        }


    }

    return pProperty;


    qDebug() << "Can't create property " << pData;
    return nullptr;

}

}
