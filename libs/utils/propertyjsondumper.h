/**
 * @file propertyjsondumper.h
 * @brief JSON serialization visitor for properties
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines PropertyJsonDumper, a concrete visitor that serializes
 * property objects to JSON format for WebSocket transmission to frontend clients.
 */

#ifndef OST_PROPERTYJSONDUMPER_H
#define OST_PROPERTYJSONDUMPER_H

#include <string>
#include <propertymulti.h>


namespace  OST
{

class Property;

/**
 * @class PropertyJsonDumper
 * @brief Visitor that serializes properties to JSON format
 *
 * PropertyJsonDumper implements the Visitor pattern to convert property objects
 * and their elements into QJsonObject structures suitable for transmission over
 * WebSocket to frontend clients.
 *
 * The generated JSON includes:
 * - Property metadata (key, label, permission, state, hierarchy levels)
 * - All elements with their types, values, and metadata
 * - LOV (List of Values) for dropdown/radio elements
 * - Grid data if property has grid capability
 * - Graph definitions for visualization
 *
 * @par Usage:
 * @code
 * PropertyMulti* property = ...; // Your property with elements
 *
 * PropertyJsonDumper dumper;
 * property->accept(&dumper);     // Serialize property to JSON
 *
 * QJsonObject json = dumper.getResult();
 * // Send json over WebSocket to frontend
 * @endcode
 *
 * @par Example Output (simplified):
 * @code
 * {
 *   "key": "parameters",
 *   "label": "Focus Parameters",
 *   "permission": 2,
 *   "state": 1,
 *   "elements": {
 *     "iterations": {
 *       "type": "int",
 *       "value": 10,
 *       "min": 1,
 *       "max": 20,
 *       "label": "Iterations"
 *     },
 *     "exposure": {
 *       "type": "float",
 *       "value": 5.0,
 *       "label": "Exposure"
 *     }
 *   }
 * }
 * @endcode
 *
 * @note Only visit(PropertyMulti*) is implemented. PropertyBase visitor methods
 *       are stubs as PropertyMulti is the primary concrete property type.
 *
 * @see PropertyVisitor
 * @see PropertyTextDumper
 * @see PropertyMulti
 */
class PropertyJsonDumper : public PropertyVisitor
{

    public:
        /**
         * @brief Default constructor
         */
        PropertyJsonDumper() = default;

        /**
         * @brief Visit PropertyBase (not implemented)
         * @param pProperty Pointer to PropertyBase
         *
         * Stub implementation as PropertyBase is abstract.
         * Only PropertyMulti visitor is used in practice.
         */
        void visit(PropertyBase *pProperty) override
        {
            Q_UNUSED(pProperty)
        }

        /**
         * @brief Visit PropertyBase with data (not implemented)
         * @param pProperty Pointer to PropertyBase
         * @param data QVariantMap with operation data
         *
         * Stub implementation - JSON dumping doesn't use data parameter.
         */
        void visit(PropertyBase* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }

        /**
         * @brief Visit PropertyMulti and serialize to JSON
         * @param pProperty Pointer to PropertyMulti to serialize
         *
         * Main implementation that serializes a PropertyMulti object to JSON.
         * Walks through all elements, extracts metadata, values, LOVs, and
         * grid data, building a complete JSON representation.
         *
         * Result is stored in mResult and accessible via getResult().
         */
        void visit(PropertyMulti *pProperty) override;

        /**
         * @brief Visit PropertyMulti with data (not implemented)
         * @param pProperty Pointer to PropertyMulti
         * @param data QVariantMap with operation data
         *
         * Stub implementation - JSON dumping doesn't use data parameter.
         */
        void visit(PropertyMulti* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }

        /**
         * @brief Get serialization result
         * @return QJsonObject containing serialized property data
         *
         * Returns the JSON representation built by the last visit() call.
         * This object is ready to be sent over WebSocket to frontend.
         *
         * @par Example:
         * @code
         * PropertyJsonDumper dumper;
         * property->accept(&dumper);
         * QJsonObject json = dumper.getResult();
         * QString jsonString = QJsonDocument(json).toJson();
         * // Send jsonString over WebSocket
         * @endcode
         */
        [[nodiscard]] const QJsonObject &getResult() const
        {
            return mResult;
        }

    private:
        QJsonObject mResult;  /*!< JSON serialization result */

        /**
         * @brief Serialize common property metadata to JSON
         * @param pProperty Pointer to PropertyBase
         * @return QJsonObject with common metadata fields
         *
         * Helper method that extracts metadata common to all property types:
         * - key, label, permission
         * - level1, level2, order (hierarchy)
         * - state, isEnabled, badge
         * - icons (preIcon1, preIcon2, postIcon1, postIcon2)
         * - freeValue
         */
        QJsonObject dumpPropertyCommons(PropertyBase *pProperty);

};
}
#endif //OST_PROPERTYJSONDUMPER_H
