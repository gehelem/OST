#include "propertyfactory.h"
namespace  OST
{

PropertyMulti *PropertyFactory::createProperty(const QString &pKey, const QVariantMap &pData)
{
    //qDebug() << "PropertyFactory::createProperty - " << pData;
    auto *pProperty = new PropertyMulti(pKey,
                                        pData["label"].toString(),
                                        OST::IntToPermission(pData["permission"].toInt()),
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
    QVariantMap elts = pData["elements"].toMap();
    foreach(const QString &key, elts.keys())
    {
        QVariantMap elt = elts[key].toMap();
        ValueBase *v = ValueFactory::createValue(elt);
        if (v != nullptr)
        {
            pProperty->addValue(key, v);
        }
    }

    return pProperty;


    qDebug() << "Can't create property " << pData;
    return nullptr;

}

}
