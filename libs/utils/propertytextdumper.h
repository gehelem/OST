/**
 * @file propertytextdumper.h
 * @brief Human-readable text serialization visitor for properties
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines PropertyTextDumper, a concrete visitor that converts
 * property objects to human-readable text format for debugging and logging.
 */

#ifndef OST_PROPERTYTEXTDUMPER_H
#define OST_PROPERTYTEXTDUMPER_H

#include <string>
#include <propertymulti.h>

namespace  OST
{

class Property;

/**
 * @class PropertyTextDumper
 * @brief Visitor that converts properties to human-readable text
 *
 * PropertyTextDumper implements the Visitor pattern to convert property objects
 * and their elements into formatted text strings suitable for debugging, logging,
 * or console output.
 *
 * The generated text includes:
 * - Property metadata (key, label, state, permission)
 * - All elements with their types and values
 * - Indented formatting for readability
 * - Grid summary if property has grid capability
 *
 * @par Usage:
 * @code
 * PropertyMulti* property = ...; // Your property with elements
 *
 * PropertyTextDumper dumper;
 * property->accept(&dumper);     // Convert property to text
 *
 * QString text = dumper.getResult();
 * qDebug() << text;  // Print to debug console
 * @endcode
 *
 * @par Example Output:
 * @code
 * Property: parameters (Focus Parameters)
 *   State: Ok (1)
 *   Permission: ReadWrite (2)
 *   Elements:
 *     iterations (int): 10
 *     exposure (float): 5.0
 *     enabled (bool): true
 *   Grid: 3 lines
 * @endcode
 *
 * @note Only visit(PropertyMulti*) is implemented. PropertyBase visitor methods
 *       are stubs as PropertyMulti is the primary concrete property type.
 *
 * @see PropertyVisitor
 * @see PropertyJsonDumper
 * @see PropertyMulti
 */
class PropertyTextDumper : public PropertyVisitor
{

    public:
        /**
         * @brief Default constructor
         */
        PropertyTextDumper() = default;

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
         * Stub implementation - text dumping doesn't use data parameter.
         */
        void visit(PropertyBase* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }

        /**
         * @brief Visit PropertyMulti and convert to text
         * @param pProperty Pointer to PropertyMulti to convert
         *
         * Main implementation that converts a PropertyMulti object to formatted text.
         * Walks through all elements, extracts values, and builds a readable
         * text representation.
         *
         * Result is stored in mResult and accessible via getResult().
         */
        void visit(PropertyMulti *pProperty) override;

        /**
         * @brief Visit PropertyMulti with data (not implemented)
         * @param pProperty Pointer to PropertyMulti
         * @param data QVariantMap with operation data
         *
         * Stub implementation - text dumping doesn't use data parameter.
         */
        void visit(PropertyMulti* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }

        /**
         * @brief Get text conversion result
         * @return QString containing formatted property text
         *
         * Returns the text representation built by the last visit() call.
         * This string is suitable for logging or debugging output.
         *
         * @par Example:
         * @code
         * PropertyTextDumper dumper;
         * property->accept(&dumper);
         * QString text = dumper.getResult();
         * qDebug() << text;
         * // Or write to log file
         * @endcode
         */
        [[nodiscard]] const QString &getResult() const
        {
            return mResult;
        }

    private:

        /**
         * @brief Convert common property metadata to text
         * @param pProperty Pointer to PropertyBase
         * @return std::string with common metadata text
         *
         * Helper method that formats metadata common to all property types:
         * - key, label, state
         * - permission, hierarchy levels
         * - Other metadata fields
         */
        std::string dumpPropertyCommons(PropertyBase *pProperty);

        QString mResult;  /*!< Text conversion result */

};
}
#endif //OST_PROPERTYTEXTDUMPER_H
