#include "lovfactory.h"
namespace  OST
{

LovBase *LovFactory::createLov(const QVariantMap &pData)
{
    //qDebug() << "LovFactory::createLov " << "-" << pData;
    if (pData.contains("type"))
    {
        if (pData["type"].toString() == "string")
        {
            auto *pLov = new LovString(pData["label"].toString());
            if (pData.contains("values") && pData["values"].canConvert<QVariantMap>())
            {
                // Object format written by LovJsonDumper: { "value": "label" }
                QVariantMap vmap = pData["values"].toMap();
                for (auto it = vmap.constBegin(); it != vmap.constEnd(); ++it)
                {
                    pLov->lovAdd(it.key(), it.value().toString());
                }
            }
            return pLov;
        }
        if (pData["type"].toString() == "int")
        {
            auto *pLov = new LovInt(pData["label"].toString());
            if (pData.contains("values") && pData["values"].canConvert<QVariantMap>())
            {
                // Object format written by LovJsonDumper: { "value_as_string": "label" }
                QVariantMap vmap = pData["values"].toMap();
                for (auto it = vmap.constBegin(); it != vmap.constEnd(); ++it)
                {
                    pLov->lovAdd(it.key().toLongLong(), it.value().toString());
                }
            }
            return pLov;
        }
        if (pData["type"].toString() == "float")
        {
            auto *pLov = new LovFloat(pData["label"].toString());
            if (pData.contains("values") && pData["values"].canConvert<QVariantMap>())
            {
                // Object format written by LovJsonDumper: { "value_as_string": "label" }
                QVariantMap vmap = pData["values"].toMap();
                for (auto it = vmap.constBegin(); it != vmap.constEnd(); ++it)
                {
                    pLov->lovAdd(it.key().toDouble(), it.value().toString());
                }
            }
            return pLov;
        }


        qDebug() << "Unknown lov type " << pData["label"].toString() << ":" << pData["type"].toString() << "-" <<
                 pData["label"].toString();
        return nullptr;
    }


    qDebug() << "Can't guess lov type " << pData["label"].toString();
    return nullptr;

}


}
