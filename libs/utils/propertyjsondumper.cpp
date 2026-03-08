/**
 * @file propertyjsondumper.cpp
 * @brief JSON serialization visitor implementation
 * @author OST Development Team
 * @version 1.0
 */

#include "propertyjsondumper.h"
#include "elementjsondumper.h"
#include <valuejsondumper.h>
#include "qglobal.h"

namespace  OST
{

/**
 * @brief Extract common property metadata to JSON
 * @param pProperty Pointer to property base
 * @return QJsonObject with common metadata fields
 *
 * Serializes metadata common to all property types:
 * - label (translated), order, hierarchy (level1/level2)
 * - status, permission, hasprofile
 * - badge, icons (pre/post), enabled state, freevalue
 *
 * Uses Translate singleton for i18n support.
 */
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
    json["autoupdate"] = pProperty->autoUpdate();
    json["badge"] = pProperty->getBadge();
    json["preicon1"] = pProperty->getPreIcon1();
    json["preicon2"] = pProperty->getPreIcon2();
    json["posticon1"] = pProperty->getPostIcon1();
    json["posticon2"] = pProperty->getPostIcon2();
    json["enabled"] = pProperty->isEnabled();
    json["freevalue"] = pProperty->getFreeValue();

    return json;

}

/**
 * @brief Serialize PropertyMulti to JSON format
 * @param pProperty Pointer to PropertyMulti to serialize
 *
 * Main visitor implementation that converts a PropertyMulti to complete JSON.
 *
 * Serialization process:
 * 1. Extract common metadata via dumpPropertyCommons()
 * 2. Add multi-property specific fields (showElts, hasGrid, rule)
 * 3. Serialize all elements using ElementJsonDumper visitor
 * 4. Serialize grid data if present (headers + grid lines)
 * 5. Add graph definitions if property has graph capability
 *
 * Result stored in mResult member, accessible via getResult().
 *
 * @par Grid Serialization:
 * Grid is converted to 2D JSON array where each line is an array of values
 * following the order defined in gridheaders. Uses ValueJsonDumper to
 * convert grid cell values.
 *
 * @par Graph Serialization:
 * If hasGraph is true, includes graphType (as string) and graphParams
 * (as QJsonObject from params QVariantMap).
 *
 * @see ElementJsonDumper
 * @see ValueJsonDumper
 */

void PropertyJsonDumper::visit(PropertyMulti *pProperty)
{
    switch (mEvent)
    {
        case OST::EvType::zz:
            break;
        case OST::EvType::aa:
        case OST::EvType::am:
            mResult = dumpDefault(pProperty);
            break;
        case OST::EvType::av:
        case OST::EvType::pr:
            mResult = dumpProfile(pProperty);
            break;
        case OST::EvType::ee:
            mResult = dumpSetValue(pProperty);
            break;
        case OST::EvType::ea:
            mResult = dumpSetAll(pProperty);
            break;
        case OST::EvType::ev:
            mResult = dumpSetValueWithMinMax(pProperty);
            break;
        case OST::EvType::ps:
            mResult = dumpPropertyState(pProperty);
            break;
        case OST::EvType::gc:
            mResult = dumpGridCreate(pProperty);
            break;
        case OST::EvType::gu:
            mResult = dumpGridUpdate(pProperty);
            break;
        case OST::EvType::gd:
            mResult = dumpGridDelete(pProperty);
            break;
        default:
            //mResult = dumpDefault(pProperty);
            mResult = QJsonObject();
            mResult["PropertyJsonDumperError"] = "Unhandled event : " +  EvToString(mEvent) ;
    }


}
QJsonObject PropertyJsonDumper::dumpPropertyState(PropertyMulti* pProperty)
{
    QJsonObject json;
    json["status"] = pProperty->state();
    json["enabled"] = pProperty->isEnabled();
    return json;
}
QJsonObject PropertyJsonDumper::dumpDefault(PropertyMulti* pProperty)
{
    QJsonObject json = dumpPropertyCommons(pProperty);
    QJsonObject elements;
    foreach(const QString &key, pProperty->getElts()->keys())
    {
        OST::ElementJsonDumper d(mEvent, mData, pProperty->getElt(key));
        QVariantMap m;
        bool b = false;
        pProperty->getElt(key)->accept(&d, m, b);
        QJsonObject value = d.getResult();
        elements[key] = value;
    }
    json["e"] = elements;
    json["showElts"] = pProperty->getShowElts();
    json["hasGrid"] = pProperty->hasGrid();
    if (pProperty->hasGrid())
    {
        json["showGrid"] = pProperty->getShowGrid();
        json["gridLimit"] = pProperty->getGridLimit();
    }
    json["rule"] = pProperty->rule();

    if (pProperty->hasGrid())
    {
        json["gridheaders"] = QJsonArray::fromStringList(pProperty->getGridHeaders());
        QJsonArray grid;
        for(int i = 0; i < pProperty->getGrid().count(); i++)
        {
            QJsonArray jLine;
            foreach(QString elt, pProperty->getGridHeaders())
            {
                ValueJsonDumper d(mEvent, mData, pProperty->getElt(elt));
                pProperty->getGrid()[i][elt]->accept(&d);
                jLine.append(d.getResult());
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

    return json;
}
QJsonObject PropertyJsonDumper::dumpSetValue(PropertyMulti* pProperty)
{
    // Minimal content, no keyword for value
    // {"evt": "sv","m": {"Dummy1": {"p": {"testnumbers": {"e": {"i1": 12 }}}}}}
    QJsonObject json;
    QJsonObject elements;
    OST::ElementJsonDumper d(mEvent, mData, mElt);
    QVariantMap m;
    bool b = false;
    mElt->accept(&d, m, b);
    QJsonObject value = d.getResult();
    elements[mElt->key()] = value["value"];
    json["e"] = elements;
    return json;
}
QJsonObject PropertyJsonDumper::dumpSetValueWithMinMax(PropertyMulti* pProperty)
{
    // Minimal content for multiple elements with more details, mostly for numeric details
    // {"evt": "se","m": {"Dummy1": {"p": {"testnumbers": {"e": {"i1": {"value":12,"min":10,"max":50},"i2": ... (... )}}}}}}

    QJsonObject json;
    QJsonObject elements;
    foreach(const QString &key, pProperty->getElts()->keys())
    {
        OST::ElementJsonDumper d(mEvent, mData, pProperty->getElt(key));
        QVariantMap m;
        bool b = false;
        pProperty->getElt(key)->accept(&d, m, b);
        QJsonObject value = d.getResult();
        elements[key] = value;
    }
    json["e"] = elements;
    return json;

}
QJsonObject PropertyJsonDumper::dumpSetAll(PropertyMulti* pProperty)
{
    // Minimal content for multiple elements
    // {"evt": "sa","m": {"Dummy1": {"p": {"testnumbers": {"e": {"i1": 12,"i2": 15}}}}}}
    QJsonObject json;
    QJsonObject elements;
    foreach(const QString &key, pProperty->getElts()->keys())
    {
        OST::ElementJsonDumper d(mEvent, mData, pProperty->getElt(key));
        QVariantMap m;
        bool b = false;
        pProperty->getElt(key)->accept(&d, m, b);
        QJsonObject value = d.getResult();
        elements[key] = value["value"];
    }
    json["e"] = elements;
    return json;

}
QJsonObject PropertyJsonDumper::dumpGridCreate(PropertyMulti* pProperty)
{
    QJsonObject json;
    if (pProperty->hasGrid() && pProperty->getGrid().count() > 0 )
    {
        int i = pProperty->getGrid().count() - 1;
        QJsonObject values;
        foreach(QString elt, pProperty->getGridHeaders())
        {
            ValueJsonDumper d(mEvent, mData, pProperty->getElt(elt));
            pProperty->getGrid()[i][elt]->accept(&d);
            values[elt] = d.getResult();
        }
        json["values"] = values;
        json["i"] = i;
    }
    return json;
}
QJsonObject PropertyJsonDumper::dumpProfile(PropertyMulti* pProperty)
{
    QJsonObject json, elements;

    foreach(const QString &key, pProperty->getElts()->keys())
    {
        OST::ElementJsonDumper d(EvType::ev, QVariant(), pProperty->getElt(key));
        QVariantMap m;
        bool b = false;
        pProperty->getElt(key)->accept(&d, m, b);
        QJsonObject value = d.getResult();
        elements[key] = value;
    }
    json["e"] = elements;

    if (pProperty->hasGrid())
    {
        json["gridheaders"] = QJsonArray::fromStringList(pProperty->getGridHeaders());
        QJsonArray grid;
        for(int i = 0; i < pProperty->getGrid().count(); i++)
        {
            QJsonArray jLine;
            foreach(QString elt, pProperty->getGridHeaders())
            {
                ValueJsonDumper d(EvType::ev, QVariant(), pProperty->getElt(elt));
                pProperty->getGrid()[i][elt]->accept(&d);
                jLine.append(d.getResult());
            }
            grid.append(jLine);
        }
        json["grid"] = grid;
    }

    return json;
}
QJsonObject PropertyJsonDumper::dumpGridUpdate(PropertyMulti* pProperty)
{
    QJsonObject json;
    if (pProperty->hasGrid() && pProperty->getGrid().count() > 0  )
    {
        int i = mData.toInt();
        QJsonObject values;
        foreach(QString elt, pProperty->getGridHeaders())
        {
            ValueJsonDumper d(mEvent, mData, pProperty->getElt(elt));
            pProperty->getGrid()[i][elt]->accept(&d);
            values[elt] = d.getResult();
        }
        json["values"] = values;
        json["i"] = i;
    }
    return json;
}
QJsonObject PropertyJsonDumper::dumpGridDelete(PropertyMulti* pProperty)
{
    QJsonObject json;
    json["i"] = mData.toInt();
    return json;
}

}
