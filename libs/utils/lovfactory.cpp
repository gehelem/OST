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
            if (pData.contains("values"))
            {
                QList ll  = pData["values"].toList();
                foreach (QVariant line, ll)
                {
                    pLov->lovAdd(line.toList()[0].toString(), line.toList()[1].toString());
                }

            }


            return pLov;
        }
        if (pData["type"].toString() == "int")
        {
            auto *pLov = new LovInt(pData["label"].toString());
            if (pData.contains("values"))
            {
                QList ll  = pData["values"].toList();
                foreach (QVariant line, ll)
                {
                    long i = line.toList()[0].toInt();
                    pLov->lovAdd(i, line.toList()[1].toString());
                }

            }


            return pLov;
        }
        if (pData["type"].toString() == "float")
        {
            auto *pLov = new LovFloat(pData["label"].toString());
            if (pData.contains("values"))
            {
                QList ll  = pData["values"].toList();
                foreach (QVariant line, ll)
                {
                    pLov->lovAdd(line.toList()[0].toFloat(), line.toList()[1].toString());
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
