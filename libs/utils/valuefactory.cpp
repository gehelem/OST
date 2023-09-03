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
            auto *pValue = new ValueBool(pData["label"].toString(),
                                         pData["order"].toString(),
                                         pData["hint"].toString()
                                        );
            pValue->setValue(pData["value"].toBool(), false);
            if (pData.contains("autoupdate")) pValue->setAutoUpdate(pData["autoupdate"].toBool());
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pValue->grid.add(val.toBool());
                }
            }

            return pValue;
        }
        if (pData["type"].toString() == "light")
        {
            auto *pValue = new ValueLight(pData["label"].toString(),
                                          pData["order"].toString(),
                                          pData["hint"].toString()
                                         );
            if (pData.contains("autoupdate")) pValue->setAutoUpdate(pData["autoupdate"].toBool());
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pValue->grid.add(IntToState(val.toInt()));
                }
            }

            return pValue;
        }
        if (pData["type"].toString() == "string")
        {
            auto *pValue = new ValueString(pData["label"].toString(),
                                           pData["order"].toString(),
                                           pData["hint"].toString()
                                          );
            pValue->setValue(pData["value"].toString(), false);
            if (pData.contains("autoupdate")) pValue->setAutoUpdate(pData["autoupdate"].toBool());
            if (pData.contains("listOfValues"))
            {
                QVariantMap elts = pData["listOfValues"].toMap();
                QList ll  = pData["listOfValues"].toList();
                foreach (auto line, ll)
                {
                    pValue->lov.add(line.toList()[0].toString(), line.toList()[1].toString());
                }
            }
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pValue->grid.add(val.toString());
                }
            }

            return pValue;
        }
        if (pData["type"].toString() == "int")
        {
            auto *pValue = new ValueInt(pData["label"].toString(),
                                        pData["order"].toString(),
                                        pData["hint"].toString()
                                       );
            if (pData.contains("value")) pValue->setValue(pData["value"].toLongLong(), false);
            if (pData.contains("min")) pValue->setMin(pData["min"].toLongLong());
            if (pData.contains("max")) pValue->setMax(pData["max"].toLongLong());
            if (pData.contains("step")) pValue->setStep(pData["step"].toLongLong());
            if (pData.contains("format")) pValue->setFormat(pData["format"].toString());
            if (pData.contains("autoupdate")) pValue->setAutoUpdate(pData["autoupdate"].toBool());
            if (pData.contains("listOfValues"))
            {
                QVariantMap elts = pData["listOfValues"].toMap();
                QList ll  = pData["listOfValues"].toList();
                foreach (auto line, ll)
                {
                    pValue->lov.add(line.toList()[0].toInt(), line.toList()[1].toString());
                }
            }
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pValue->grid.add(val.toLongLong());
                }
            }

            return pValue;
        }
        if (pData["type"].toString() == "float")
        {
            auto *pValue = new ValueFloat(pData["label"].toString(),
                                          pData["order"].toString(),
                                          pData["hint"].toString()
                                         );
            if (pData.contains("value")) pValue->setValue(pData["value"].toDouble(), false);
            if (pData.contains("min")) pValue->setMin(pData["min"].toDouble());
            if (pData.contains("max")) pValue->setMax(pData["max"].toDouble());
            if (pData.contains("step")) pValue->setStep(pData["step"].toDouble());
            if (pData.contains("format")) pValue->setFormat(pData["format"].toString());
            if (pData.contains("autoupdate")) pValue->setAutoUpdate(pData["autoupdate"].toBool());
            if (pData.contains("listOfValues"))
            {
                QVariantMap elts = pData["listOfValues"].toMap();
                QList ll  = pData["listOfValues"].toList();
                foreach (auto line, ll)
                {
                    pValue->lov.add(line.toList()[0].toFloat(), line.toList()[1].toString());
                }
            }
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pValue->grid.add(val.toDouble());
                }
            }


            return pValue;
        }
        if (pData["type"].toString() == "img")
        {
            auto *pValue = new ValueImg(pData["label"].toString(),
                                        pData["order"].toString(),
                                        pData["hint"].toString()
                                       );
            if (pData.contains("urljpeg")) pValue->setUrlJpeg(pData["urljpeg"].toString());
            if (pData.contains("urlfits")) pValue->setUrlJpeg(pData["urlfits"].toString());
            if (pData.contains("urlthumbnail")) pValue->setUrlJpeg(pData["urlthumbnail"].toString());
            if (pData.contains("urloverlay")) pValue->setUrlJpeg(pData["urloverlay"].toString());
            if (pData.contains("height")) pValue->setHeight(pData["height"].toLongLong());
            if (pData.contains("width")) pValue->setWidth(pData["width"].toLongLong());
            if (pData.contains("min")) pValue->setMin(pData["min"].toLongLong());
            if (pData.contains("max")) pValue->setMax(pData["max"].toLongLong());
            if (pData.contains("mean")) pValue->setMean(pData["mean"].toDouble());
            if (pData.contains("stddev")) pValue->setStddev(pData["stddev"].toDouble());
            if (pData.contains("snr")) pValue->setSNR(pData["snr"].toDouble());
            return pValue;
        }

        if (pData["type"].toString() == "message")
        {
            auto *pValue = new ValueMessage(pData["label"].toString(),
                                            pData["order"].toString(),
                                            pData["hint"].toString()
                                           );
            if (pData.contains("message")) pValue->setMessage(pData["message"].toString());
            return pValue;
        }

        qDebug() << "Unknown value type " << pData["label"].toString() << ":" << pData["type"].toString() << "-" <<
                 pData["label"].toString();
    }

    //if (pData.contains("value"))
    //{
    //    if (strcmp(pData["value"].typeName(), "bool") == 0 )
    //    {
    //        auto *pValue = new ValueBool(pData["label"].toString(),
    //                                     pData["order"].toString(),
    //                                     pData["hint"].toString()
    //                                    );
    //        pValue->setValue(pData["value"].toBool());
    //        return pValue;
    //    }
    //    if (strcmp(pData["value"].typeName(), "QString") == 0 )
    //    {
    //        auto *pValue = new ValueString(pData["label"].toString(),
    //                                       pData["order"].toString(),
    //                                       pData["hint"].toString()
    //                                      );
    //        pValue->setValue(pData["value"].toString(), false);
    //        return pValue;
    //    }
    //    if ((strcmp(pData["value"].typeName(), "int") == 0 )
    //            || (strcmp(pData["value"].typeName(), "qlonglong") == 0 )
    //       )
    //    {
    //        auto *pValue = new ValueInt(pData["label"].toString(),
    //                                    pData["order"].toString(),
    //                                    pData["hint"].toString()
    //                                   );
    //        if (pData.contains("value")) pValue->setValue(pData["value"].toLongLong());
    //        if (pData.contains("min")) pValue->setMin(pData["min"].toLongLong());
    //        if (pData.contains("max")) pValue->setMax(pData["max"].toLongLong());
    //        if (pData.contains("step")) pValue->setStep(pData["step"].toLongLong());
    //        if (pData.contains("format")) pValue->setFormat(pData["format"].toString());
    //        return pValue;
    //    }
    //    if ((strcmp(pData["value"].typeName(), "double") == 0 )
    //            || (strcmp(pData["value"].typeName(), "float") == 0 )
    //       )
    //    {
    //        auto *pValue = new ValueFloat(pData["label"].toString(),
    //                                      pData["order"].toString(),
    //                                      pData["hint"].toString()
    //                                     );
    //        if (pData.contains("value")) pValue->setValue(pData["value"].toDouble());
    //        if (pData.contains("min")) pValue->setMin(pData["min"].toDouble());
    //        if (pData.contains("max")) pValue->setMax(pData["max"].toDouble());
    //        if (pData.contains("step")) pValue->setStep(pData["step"].toDouble());
    //        if (pData.contains("format")) pValue->setFormat(pData["format"].toString());
    //        return pValue;
    //    }
    //
    //    qDebug() << "Unknown value type " << pData["label"].toString() << ":" << pData["value"].typeName();
    //}
    qDebug() << "Can't guess element type " << pData["label"].toString();
    return nullptr;

}


}
