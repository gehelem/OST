/**
 * @file propertyfactory.cpp
 * @brief PropertyFactory implementation for creating properties from configuration
 * @author OST Development Team
 * @version 1.0
 */

#include "propertyfactory.h"
namespace  OST
{

/**
 * @brief Create PropertyMulti from configuration QVariantMap
 * @param pKey Property unique identifier (key)
 * @param pData QVariantMap containing complete property configuration
 * @return Pointer to newly created PropertyMulti (caller owns), or nullptr on error
 *
 * Factory method that constructs a fully configured PropertyMulti from serialized data.
 * This is the primary method for restoring properties from database profiles or
 * deserializing from JSON configuration files.
 *
 * @par Construction Process:
 * 1. Create PropertyMulti with basic metadata (label, permission, hierarchy, profiles, grid)
 * 2. Configure switch rule (OneOfMany/AtMostOne/Any) if present
 * 3. Apply grid settings (hasGrid, showGrid, showElts, gridLimit)
 * 4. Validate and configure graph definitions (type + params)
 * 5. Set UI hints (autoupdown, autoselect, badge)
 * 6. Create all elements using ElementFactory
 * 7. Restore grid data if present (using gridheaders for column order)
 * 8. Configure icons (preIcon1/2, postIcon1/2)
 * 9. Set freevalue if present
 *
 * @par Required Fields in pData:
 * - "label": QString - Display label
 * - "permission": int - Permission level (0/1/2)
 * - "devcat": QString - Device category (level1 hierarchy)
 * - "group": QString - Property group (level2 hierarchy)
 * - "order": QString - Sort order string
 * - "hasprofile": bool - Whether to persist in profiles
 * - "hasGrid": bool - Whether property supports grid
 *
 * @par Optional Fields in pData:
 * - "rule": int - SwitchsRule (0=OneOfMany, 1=AtMostOne, 2=Any, default=Any)
 * - "elements": QVariantMap - Element definitions (key → element config)
 * - "showGrid": bool - Grid visibility (default=false)
 * - "showElts": bool - Elements visibility (default=true)
 * - "gridLimit": int - Max grid lines (default unlimited)
 * - "graphType": QString - Graph type ("xy", "dy", "phd", "sxy", "sdy")
 * - "graphParams": QVariantMap - Graph parameters
 * - "grid": QJsonArray - Grid data (requires "gridheaders")
 * - "gridheaders": QJsonArray - Column order for grid
 * - "autoupdown": bool - Auto up/down buttons
 * - "autoselect": bool - Auto-select on add
 * - "badge": bool - Show badge indicator
 * - "preicon1", "preicon2": QString - Leading icons
 * - "posticon1", "posticon2": QString - Trailing icons
 * - "freevalue": QString - Custom free value field
 *
 * @par Element Creation:
 * Elements are created using ElementFactory::createElement() from element configs:
 * @code
 * QVariantMap eltConfig;
 * eltConfig["type"] = "int";
 * eltConfig["label"] = "Iterations";
 * eltConfig["value"] = 10;
 * eltConfig["min"] = 1;
 * eltConfig["max"] = 20;
 * @endcode
 *
 * @par Grid Restoration:
 * If "grid" field present, restores grid lines using gridheaders for column mapping:
 * @code
 * // For each line in grid array:
 * //   1. Set element values from line[j] using headers[j] as key
 * //   2. Call property->push() to add line to grid
 * @endcode
 *
 * @par Validation:
 * - Warns if elements field missing (returns empty property)
 * - Warns if graph defined without grid capability
 * - Warns if graph defined without params
 * - Warns if grid data present without gridheaders
 *
 * @note Unreachable code at line 133 (after return statement)
 * @warning Graph requires both hasGrid=true and graphType+graphParams fields
 *
 * @see ElementFactory::createElement()
 * @see PropertyMulti::PropertyMulti()
 * @see PropertyMulti::push()
 */
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
    if (pData.contains("gridLimit"))
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
            qDebug() << "Can't initialize grid values without gridheaders definitions " << pKey;
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
    if (pData.contains("freevalue"))
    {
        pProperty->setPostIcon2(pData["freevalue"].toString());
    }

    return pProperty;


    qDebug() << "Can't create property " << pData;
    return nullptr;

}

}
