/**
 * @file propertyjsondumper.cpp
 * @brief JSON serialization visitor implementation
 * @author OST Development Team
 * @version 1.0
 */

#include "propertyjsondumper.h"
#include "elementjsondumper.h"
#include <valuejsondumper.h>
#include "elementupdate.h"
#include "qglobal.h"
#include <sstream>
#include <translate.h>

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
    Translate* t = Translate::GetInstance();
    json["label"] = t->translate(pProperty->label());
    json["order"] = pProperty->order();
    json["level1"] = t->translate(pProperty->level1());
    json["level2"] = t->translate(pProperty->level2());
    json["status"] = pProperty->state();
    json["permission"] = pProperty->permission();
    json["hasprofile"] = pProperty->hasProfile();
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
    QJsonObject json = dumpPropertyCommons(pProperty);
    json["showElts"] = pProperty->getShowElts();
    json["hasGrid"] = pProperty->hasGrid();
    if (pProperty->hasGrid())
    {
        json["showGrid"] = pProperty->getShowGrid();
        json["gridLimit"] = pProperty->getGridLimit();
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

    if (pProperty->hasGrid())
    {
        QJsonArray grid;
        for(int i = 0; i < pProperty->getGrid().count(); i++)
        {
            QJsonArray jLine;
            foreach(QString elt, pProperty->getGridHeaders())
            {
                ValueJsonDumper d;
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

    mResult = json;

}

}
