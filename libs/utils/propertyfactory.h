/**
 * @file propertyfactory.h
 * @brief Factory for creating properties from configuration data
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines PropertyFactory, a static factory class that creates
 * PropertyMulti objects from QVariantMap configuration data.
 */

#ifndef OST_PROPERTYFACTORY_H
#define OST_PROPERTYFACTORY_H

#include <baseclient.h>
#include <propertymulti.h>
#include <elementfactory.h>

namespace  OST
{

/**
 * @class PropertyFactory
 * @brief Factory for creating PropertyMulti objects from configuration
 *
 * PropertyFactory provides a static factory method to create fully configured
 * PropertyMulti objects from QVariantMap data. This is typically used when
 * restoring properties from database profiles or deserializing from JSON.
 *
 * The factory:
 * - Creates the PropertyMulti with metadata (key, label, permission, hierarchy)
 * - Creates and adds all elements using ElementFactory
 * - Configures grid settings if present
 * - Configures graph definitions if present
 * - Restores element values and LOVs
 *
 * @par Usage (creating from configuration):
 * @code
 * QVariantMap config;
 * config["key"] = "parameters";
 * config["label"] = "Focus Parameters";
 * config["permission"] = 2;  // ReadWrite
 * config["level1"] = "Control";
 * config["level2"] = "Main";
 * config["order"] = "010";
 * config["hasprofile"] = true;
 * config["hasgrid"] = false;
 *
 * // Element configurations
 * QVariantMap elements;
 * QVariantMap iterationsElt;
 * iterationsElt["type"] = "int";
 * iterationsElt["label"] = "Iterations";
 * iterationsElt["order"] = "10";
 * iterationsElt["value"] = 10;
 * iterationsElt["min"] = 1;
 * iterationsElt["max"] = 20;
 * elements["iterations"] = iterationsElt;
 * config["elements"] = elements;
 *
 * PropertyMulti* property = PropertyFactory::createProperty("parameters", config);
 * // property is fully configured and ready to use
 * @endcode
 *
 * @par Restoring from Database:
 * @code
 * // DBManager returns QVariantMap from saved profile
 * QVariantMap savedConfig = dbManager->loadProperty("focus", "parameters");
 *
 * PropertyMulti* property = PropertyFactory::createProperty("parameters", savedConfig);
 * // Property restored with all saved values
 * @endcode
 *
 * @note Constructor is deleted - this is a pure static factory class.
 * @note Commented INDI-related methods are legacy code for INDI protocol conversion.
 *
 * @see PropertyMulti
 * @see ElementFactory
 * @see DBManager
 */
class PropertyFactory
{

    public:
        /**
         * @brief Deleted constructor - static factory class only
         *
         * PropertyFactory cannot be instantiated. All methods are static.
         */
        PropertyFactory() = delete;

        // Legacy INDI conversion methods (commented out)
        //static RootProperty* createProperty(INumberVectorProperty* pVector);
        //static RootProperty* createProperty(ISwitchVectorProperty * pVector);
        //static RootProperty* createProperty(ITextVectorProperty* pVector);
        //static RootProperty* createProperty(ILightVectorProperty* pVector);

        /**
         * @brief Create PropertyMulti from configuration data
         * @param pKey Property key (unique identifier)
         * @param pData QVariantMap containing property configuration
         * @return Pointer to newly created PropertyMulti (caller owns)
         *
         * Creates a fully configured PropertyMulti object from configuration data.
         *
         * @par Required fields in pData:
         * - "label": Property display label (QString)
         * - "permission": Access permission (int: 0=ReadOnly, 1=WriteOnly, 2=ReadWrite)
         * - "level1": First hierarchy level (QString)
         * - "level2": Second hierarchy level (QString)
         * - "order": Sort order (QString)
         * - "hasprofile": Whether to save in profile (bool)
         * - "hasgrid": Whether property has grid capability (bool)
         *
         * @par Optional fields in pData:
         * - "elements": QVariantMap of element configurations (key → element config)
         * - "hasgraph": Whether property has graph (bool)
         * - "graphdefs": Graph configuration (QVariantMap)
         * - "showgrid": Whether grid is visible (bool)
         * - "showelts": Whether elements are visible (bool)
         * - Other property metadata fields
         *
         * @par Example element configuration:
         * @code
         * QVariantMap eltConfig;
         * eltConfig["type"] = "int";     // Element type
         * eltConfig["label"] = "Iterations";
         * eltConfig["order"] = "10";
         * eltConfig["hint"] = "Number of focus iterations";
         * eltConfig["value"] = 10;       // Initial value
         * eltConfig["min"] = 1;          // Min value (numeric types)
         * eltConfig["max"] = 20;         // Max value (numeric types)
         * eltConfig["lov"] = lovMap;     // List of values (if applicable)
         * @endcode
         *
         * @note Caller is responsible for deleting the returned PropertyMulti.
         * @note Uses ElementFactory internally to create elements from config.
         *
         * @see ElementFactory::createElement()
         * @see PropertyMulti::PropertyMulti()
         */
        static PropertyMulti* createProperty(const QString &pKey, const QVariantMap &pData);
};

}
#endif //OST_PROPERTYFACTORY_H
