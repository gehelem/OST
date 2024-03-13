#include "elementfactory.h"
namespace  OST
{

ElementBase *ElementFactory::createElement(const QVariantMap &pData)
{
    //qDebug() << "ElementFactory::createElement " << "-" << pData;
    if (pData.contains("type"))
    {
        if (pData["type"].toString() == "bool")
        {
            auto *pElement = new ElementBool(pData["label"].toString(),
                                             pData["order"].toString(),
                                             pData["hint"].toString()
                                            );
            pElement->setValue(pData["value"].toBool(), false);
            if (pData.contains("autoupdate")) pElement->setAutoUpdate(pData["autoupdate"].toBool());
            if (pData.contains("directedit")) pElement->setDirectEdit(pData["directedit"].toBool());
            if (pData.contains("arrayLimit")) pElement->setArrayLimit(pData["arrayLimit"].toInt());
            if (pData.contains("badge")) pElement->setBadge(pData["badge"].toBool());
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pElement->gridAdd(val.toBool());
                }
            }

            return pElement;
        }
        if (pData["type"].toString() == "light")
        {
            auto *pElement = new ElementLight(pData["label"].toString(),
                                              pData["order"].toString(),
                                              pData["hint"].toString()
                                             );
            if (pData.contains("autoupdate")) pElement->setAutoUpdate(pData["autoupdate"].toBool());
            if (pData.contains("directedit")) pElement->setDirectEdit(pData["directedit"].toBool());
            if (pData.contains("arrayLimit")) pElement->setArrayLimit(pData["arrayLimit"].toInt());
            if (pData.contains("preicon")) pElement->setPreIcon(pData["preicon"].toString());
            if (pData.contains("posticon")) pElement->setPostIcon(pData["posticon"].toString());
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pElement->gridAdd(IntToState(val.toInt()));
                }
            }

            return pElement;
        }
        if (pData["type"].toString() == "string")
        {
            auto *pElement = new ElementString(pData["label"].toString(),
                                               pData["order"].toString(),
                                               pData["hint"].toString()
                                              );
            pElement->setValue(pData["value"].toString(), false);
            if (pData.contains("autoupdate")) pElement->setAutoUpdate(pData["autoupdate"].toBool());
            if (pData.contains("directedit")) pElement->setDirectEdit(pData["directedit"].toBool());
            if (pData.contains("arrayLimit")) pElement->setArrayLimit(pData["arrayLimit"].toInt());
            if (pData.contains("preicon")) pElement->setPreIcon(pData["preicon"].toString());
            if (pData.contains("posticon")) pElement->setPostIcon(pData["posticon"].toString());
            if (pData.contains("listOfValues"))
            {
                if (pData["listOfValues"].canConvert<QVariantList>())
                {
                    QVariantMap elts = pData["listOfValues"].toMap();
                    QList ll  = pData["listOfValues"].toList();
                    foreach (auto line, ll)
                    {
                        pElement->lovAdd(line.toList()[0].toString(), line.toList()[1].toString());
                    }
                }
                else
                {
                    pElement->setGlobalLov(pData["listOfValues"].toString());
                }
            }
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pElement->gridAdd(val.toString());
                }
            }

            return pElement;
        }
        if (pData["type"].toString() == "int")
        {
            auto *pElement = new ElementInt(pData["label"].toString(),
                                            pData["order"].toString(),
                                            pData["hint"].toString()
                                           );
            if (pData.contains("value")) pElement->setValue(pData["value"].toLongLong(), false);
            if (pData.contains("min")) pElement->setMin(pData["min"].toLongLong());
            if (pData.contains("max")) pElement->setMax(pData["max"].toLongLong());
            if (pData.contains("step")) pElement->setStep(pData["step"].toLongLong());
            if (pData.contains("format")) pElement->setFormat(pData["format"].toString());
            if (pData.contains("autoupdate")) pElement->setAutoUpdate(pData["autoupdate"].toBool());
            if (pData.contains("directedit")) pElement->setDirectEdit(pData["directedit"].toBool());
            if (pData.contains("arrayLimit")) pElement->setArrayLimit(pData["arrayLimit"].toInt());
            if (pData.contains("preicon")) pElement->setPreIcon(pData["preicon"].toString());
            if (pData.contains("posticon")) pElement->setPostIcon(pData["posticon"].toString());
            if (pData.contains("listOfValues"))
            {
                if (pData["listOfValues"].canConvert<QVariantList>())
                {
                    QVariantMap elts = pData["listOfValues"].toMap();
                    QList ll  = pData["listOfValues"].toList();
                    foreach (auto line, ll)
                    {
                        long i = line.toList()[0].toInt();
                        pElement->lovAdd(i, line.toList()[1].toString());
                    }
                }
                else
                {
                    pElement->setGlobalLov(pData["listOfValues"].toString());
                }
            }
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pElement->gridAdd(val.toLongLong());
                }
            }

            return pElement;
        }
        if (pData["type"].toString() == "float")
        {
            auto *pElement = new ElementFloat(pData["label"].toString(),
                                              pData["order"].toString(),
                                              pData["hint"].toString()
                                             );
            if (pData.contains("value")) pElement->setValue(pData["value"].toDouble(), false);
            if (pData.contains("min")) pElement->setMin(pData["min"].toDouble());
            if (pData.contains("max")) pElement->setMax(pData["max"].toDouble());
            if (pData.contains("step")) pElement->setStep(pData["step"].toDouble());
            if (pData.contains("format")) pElement->setFormat(pData["format"].toString());
            if (pData.contains("autoupdate")) pElement->setAutoUpdate(pData["autoupdate"].toBool());
            if (pData.contains("directedit")) pElement->setDirectEdit(pData["directedit"].toBool());
            if (pData.contains("arrayLimit")) pElement->setArrayLimit(pData["arrayLimit"].toInt());
            if (pData.contains("preicon")) pElement->setPreIcon(pData["preicon"].toString());
            if (pData.contains("posticon")) pElement->setPostIcon(pData["posticon"].toString());
            if (pData.contains("listOfValues"))
            {
                if (pData["listOfValues"].canConvert<QVariantList>())
                {
                    QVariantMap elts = pData["listOfValues"].toMap();
                    QList ll  = pData["listOfValues"].toList();
                    foreach (auto line, ll)
                    {
                        pElement->lovAdd(line.toList()[0].toDouble(), line.toList()[1].toString());
                    }
                }
                else
                {
                    pElement->setGlobalLov(pData["listOfValues"].toString());
                }
            }
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pElement->gridAdd(val.toDouble());
                }
            }


            return pElement;
        }
        if (pData["type"].toString() == "img")
        {
            auto *pElement = new ElementImg(pData["label"].toString(),
                                            pData["order"].toString(),
                                            pData["hint"].toString()
                                           );
            ImgData dta;

            if (pData.contains("arrayLimit")) pElement->setArrayLimit(pData["arrayLimit"].toInt());

            if (pData.contains("urljpeg")) dta.mUrlJpeg = pData["urljpeg"].toString();
            if (pData.contains("urlfits")) dta.mUrlFits = pData["urlfits"].toString();
            if (pData.contains("urloverlay")) dta.mUrlOverlay = pData["urloverlay"].toString();
            if (pData.contains("height")) dta.height = pData["height"].toInt();
            if (pData.contains("width")) dta.width = pData["width"].toInt();
            if (pData.contains("min")) dta.min[0] = pData["min"].toList()[0].toInt();
            if (pData.contains("max")) dta.max[0] = pData["max"].toList()[0].toInt();
            if (pData.contains("mean")) dta.mean[0] = pData["mean"].toList()[0].toDouble();
            if (pData.contains("stddev")) dta.stddev[0] = pData["stddev"].toList()[0].toDouble();
            if (pData.contains("snr")) dta.SNR = pData["snr"].toDouble();
            if (pData.contains("hfravg")) dta.HFRavg = pData["hfravg"].toDouble();
            if (pData.contains("stars")) dta.starsCount = pData["stars"].toDouble();
            if (pData.contains("issolved")) dta.isSolved = pData["issolved"].toBool();
            if (pData.contains("solverra")) dta.solverRA = pData["solverra"].toDouble();
            if (pData.contains("solverde")) dta.solverDE = pData["solverde"].toDouble();

            pElement->setValue(dta, false);
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pElement->gridAdd(QVariantMapToImgData(val.toMap()));
                }
            }

            return pElement;
        }

        if (pData["type"].toString() == "video")
        {
            auto *pElement = new ElementVideo(pData["label"].toString(),
                                              pData["order"].toString(),
                                              pData["hint"].toString()
                                             );
            if (pData.contains("value")) pElement->setValue(VariantMapToVideoData(pData["value"].toMap()), false);
            if (pData.contains("arrayLimit")) pElement->setArrayLimit(pData["arrayLimit"].toInt());

            return pElement;
        }

        if (pData["type"].toString() == "message")
        {
            auto *pElement = new ElementMessage(pData["label"].toString(),
                                                pData["order"].toString(),
                                                pData["hint"].toString()
                                               );
            if (pData.contains("message")) pElement->setMessage(pData["message"].toString());
            return pElement;
        }
        if (pData["type"].toString() == "graph")
        {
            auto *pElement = new ElementGraph(pData["label"].toString(),
                                              pData["order"].toString(),
                                              pData["hint"].toString()
                                             );
            GraphDefs d;
            if (!pData.contains("graphtype"))
            {
                qDebug() << "Graph defined without type " << pElement->label();
            }
            if (!pData.contains("params"))
            {
                qDebug() << "Graph defined without params " << pElement->label();
            }
            d.type = StringToGraphType(pData["graphtype"].toString());
            d.params = pData["params"].toMap();
            pElement->setGraphDefs(d);

            return pElement;
        }
        if (pData["type"].toString() == "prg")
        {
            auto *pElement = new ElementPrg(pData["label"].toString(),
                                            pData["order"].toString(),
                                            pData["hint"].toString()
                                           );
            if (!pData.contains("prgtype"))
            {
                qDebug() << "Progress defined without type " << pElement->label() << " set default to bar";
            }
            else
            {
                if (pData["prgtype"].toString() == "bar") pElement->setPrgType(bar);
                if (pData["prgtype"].toString() == "spinner") pElement->setPrgType(spinner);
            }
            if (pData.contains("value")) pElement->setValue(pData["value"].toDouble(), false);
            if (pData.contains("dynlabel")) pElement->setDynLabel(pData["dynlabel"].toString(), false);
            if (pData.contains("arrayLimit")) pElement->setArrayLimit(pData["arrayLimit"].toInt());
            if (pData.contains("gridvalues"))
            {
                QList ll  = pData["gridvalues"].toList();
                foreach (QVariant val, ll)
                {
                    pElement->gridAdd(val.toDouble());
                }
            }


            return pElement;
        }


        qDebug() << "Unknown Element type " << pData["label"].toString() << ":" << pData["type"].toString() << "-" <<
                 pData["label"].toString();
    }


    qDebug() << "Can't guess element type " << pData["label"].toString();
    return nullptr;

}

GridBase *GridFactory::createGrid(ElementBase * &pElement)
{
    if (pElement->getType() == "string")
    {
        auto *pGrid = new GridString(pElement);
        return pGrid;
    }
    if (pElement->getType() == "int")
    {
        auto *pGrid = new GridInt(pElement);
        return pGrid;
    }
    if (pElement->getType() == "bool")
    {
        auto *pGrid = new GridBool(pElement);
        return pGrid;
    }
    if (pElement->getType() == "float")
    {
        auto *pGrid = new GridFloat(pElement);
        return pGrid;
    }
    if (pElement->getType() == "prg")
    {
        auto *pGrid = new GridPrg(pElement);
        return pGrid;
    }
    if (pElement->getType() == "message")
    {
        auto *pGrid = new GridMessage(pElement);
        return pGrid;
    }
    if (pElement->getType() == "video")
    {
        auto *pGrid = new GridVideo(pElement);
        return pGrid;
    }
    if (pElement->getType() == "img")
    {
        auto *pGrid = new GridImg(pElement);
        return pGrid;
    }
    if (pElement->getType() == "light")
    {
        auto *pGrid = new GridLight(pElement);
        return pGrid;
    }
    if (pElement->getType() == "graph")
    {
        auto *pGrid = new GridGraph(pElement);
        return pGrid;
    }

    qDebug() << "Unhandled grid type " << pElement->getType();

    return nullptr;

}


}