#include "valuefactory.h"
namespace  OST
{

ValueBase *ValueFactory::createValue(const QVariantMap &pData)
{
    //qDebug() << "ValueFactory::createValue " << "-" << pData;
    if (pData.contains("type"))
    {
        if (pData["type"].toString() == "bool")
        {
            auto *pValue = new ValueBool(pData["elementLabel"].toString(),
                                         pData["order"].toString(),
                                         pData["hint"].toString()
                                        );
            pValue->setValue(pData["value"].toBool());
            return pValue;
        }
        if (pData["type"].toString() == "string")
        {
            auto *pValue = new ValueString(pData["elementLabel"].toString(),
                                           pData["order"].toString(),
                                           pData["hint"].toString()
                                          );
            pValue->setValue(pData["value"].toString());
            return pValue;
        }
        if (pData["type"].toString() == "int")
        {
            auto *pValue = new ValueInt(pData["elementLabel"].toString(),
                                        pData["order"].toString(),
                                        pData["hint"].toString()
                                       );
            if (pData.contains("value")) pValue->setValue(pData["value"].toLongLong());
            if (pData.contains("min")) pValue->setMin(pData["min"].toLongLong());
            if (pData.contains("max")) pValue->setMax(pData["max"].toLongLong());
            if (pData.contains("step")) pValue->setStep(pData["step"].toLongLong());
            if (pData.contains("format")) pValue->setFormat(pData["format"].toString());
            return pValue;
        }
        if (pData["type"].toString() == "float")
        {
            auto *pValue = new ValueFloat(pData["elementLabel"].toString(),
                                          pData["order"].toString(),
                                          pData["hint"].toString()
                                         );
            if (pData.contains("value")) pValue->setValue(pData["value"].toDouble());
            if (pData.contains("min")) pValue->setMin(pData["min"].toDouble());
            if (pData.contains("max")) pValue->setMax(pData["max"].toDouble());
            if (pData.contains("step")) pValue->setStep(pData["step"].toDouble());
            if (pData.contains("format")) pValue->setFormat(pData["format"].toString());
            return pValue;
        }

        qDebug() << "Unknown value type " << pData["elementLabel"].toString() << ":" << pData["type"].toString() << "-" <<
                 pData["elementLabel"].toString();
    }

    if (pData.contains("value"))
    {
        if (strcmp(pData["value"].typeName(), "bool") == 0 )
        {
            auto *pValue = new ValueBool(pData["elementLabel"].toString(),
                                         pData["order"].toString(),
                                         pData["hint"].toString()
                                        );
            pValue->setValue(pData["value"].toBool());
            return pValue;
        }
        if (strcmp(pData["value"].typeName(), "QString") == 0 )
        {
            auto *pValue = new ValueString(pData["elementLabel"].toString(),
                                           pData["order"].toString(),
                                           pData["hint"].toString()
                                          );
            pValue->setValue(pData["value"].toString());
            return pValue;
        }
        if ((strcmp(pData["value"].typeName(), "int") == 0 )
                || (strcmp(pData["value"].typeName(), "qlonglong") == 0 )
           )
        {
            auto *pValue = new ValueInt(pData["elementLabel"].toString(),
                                        pData["order"].toString(),
                                        pData["hint"].toString()
                                       );
            if (pData.contains("value")) pValue->setValue(pData["value"].toLongLong());
            if (pData.contains("min")) pValue->setMin(pData["min"].toLongLong());
            if (pData.contains("max")) pValue->setMax(pData["max"].toLongLong());
            if (pData.contains("step")) pValue->setStep(pData["step"].toLongLong());
            if (pData.contains("format")) pValue->setFormat(pData["format"].toString());
            return pValue;
        }
        if ((strcmp(pData["value"].typeName(), "double") == 0 )
                || (strcmp(pData["value"].typeName(), "float") == 0 )
           )
        {
            auto *pValue = new ValueFloat(pData["elementLabel"].toString(),
                                          pData["order"].toString(),
                                          pData["hint"].toString()
                                         );
            if (pData.contains("value")) pValue->setValue(pData["value"].toDouble());
            if (pData.contains("min")) pValue->setMin(pData["min"].toDouble());
            if (pData.contains("max")) pValue->setMax(pData["max"].toDouble());
            if (pData.contains("step")) pValue->setStep(pData["step"].toDouble());
            if (pData.contains("format")) pValue->setFormat(pData["format"].toString());
            return pValue;
        }

        qDebug() << "Unknown value type " << pData["elementLabel"].toString() << ":" << pData["value"].typeName();
    }
    qDebug() << "Can't guess element type " << pData["elementLabel"].toString();
    return nullptr;

}


}
