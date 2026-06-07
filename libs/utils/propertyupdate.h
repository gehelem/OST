/**
 * @file propertyupdate.h
 * @brief Property update visitor for applying changes from frontend
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines PropertyUpdate, a concrete visitor that updates property
 * values and settings from QVariantMap data received from WebSocket/frontend.
 */

#ifndef OST_PROPERTYUPDATE_H
#define OST_PROPERTYUPDATE_H
#include <string>
#include <propertymulti.h>
namespace  OST
{
class Property;

/**
 * @class PropertyUpdate
 * @brief Visitor that updates properties from QVariantMap data
 *
 * PropertyUpdate implements the Visitor pattern to apply updates to property
 * objects based on data received from frontend clients via WebSocket.
 *
 * The visitor handles:
 * - Element value updates (setValue calls)
 * - Grid operations (newline, updateline, deleteline, cleargrid, swaplines, fetchline)
 * - Element metadata updates (min/max, LOV, etc.)
 * - Property state changes
 *
 * @par Usage (applying frontend update):
 * @code
 * PropertyMulti* property = ...; // Your property
 *
 * // Data received from frontend WebSocket
 * QVariantMap updateData;
 * updateData["iterations"] = 15;  // Update iterations element to 15
 * updateData["exposure"] = 10.0;  // Update exposure element to 10.0
 *
 * PropertyUpdate updater;
 * property->accept(&updater, updateData);  // Apply updates
 * @endcode
 *
 * @par Grid Operations:
 * @code
 * // Frontend sends grid operation
 * QVariantMap gridOp;
 * gridOp["action"] = "newline";
 * gridOp["exposure"] = 5.0;
 * gridOp["filter"] = "Ha";
 *
 * PropertyUpdate updater;
 * property->accept(&updater, gridOp);  // Creates new grid line
 * @endcode
 *
 * @par Supported Grid Actions:
 * - "newline": Create new grid line with provided values
 * - "updateline": Update existing grid line at index
 * - "deleteline": Delete grid line at index
 * - "cleargrid": Clear all grid lines
 * - "swaplines": Swap two grid lines (reorder)
 * - "fetchline": Load grid line into base elements (for editing)
 *
 * @note Only visit(PropertyMulti*, QVariantMap&) is implemented.
 *       This is the primary update path from frontend.
 *
 * @see PropertyVisitor
 * @see PropertyJsonDumper
 * @see PropertyMulti::accept()
 */
class PropertyUpdate : public PropertyVisitor
{

    public:
        /**
         * @brief Default constructor
         */
        PropertyUpdate() = default;

        /**
         * @brief Visit PropertyBase (not implemented)
         * @param pProperty Pointer to PropertyBase
         *
         * Stub implementation as PropertyBase updates are not used.
         */
        void visit(PropertyBase* pProperty) override
        {
            Q_UNUSED(pProperty)
        }

        /**
         * @brief Visit PropertyMulti (not implemented)
         * @param pProperty Pointer to PropertyMulti
         *
         * Stub implementation. Use visit(PropertyMulti*, QVariantMap&) instead.
         */
        void visit(PropertyMulti* pProperty) override
        {
            Q_UNUSED(pProperty)
        }

        /**
         * @brief Visit PropertyBase with data (not implemented)
         * @param pProperty Pointer to PropertyBase
         * @param data QVariantMap with update data
         *
         * Stub implementation. Only PropertyMulti updates are supported.
         */
        void visit(PropertyBase *pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty)
            Q_UNUSED(data)
        }

        /**
         * @brief Visit PropertyMulti and apply updates from data
         * @param pProperty Pointer to PropertyMulti to update
         * @param data QVariantMap containing updates from frontend
         *
         * Main implementation that processes updates from frontend:
         *
         * For element updates, data contains element keys and new values:
         * @code
         * data["elementKey"] = newValue;
         * @endcode
         *
         * For grid operations, data contains "action" field:
         * @code
         * data["action"] = "newline";
         * data["elementKey"] = value;
         * // ... other element values for the new line
         * @endcode
         *
         * Updates are applied by calling appropriate element setValue() methods
         * or property grid methods (push, updateLine, deleteLine, etc.).
         *
         * Emits valueSet signals to propagate changes through the system.
         */
        void visit(PropertyMulti *pProperty, QVariantMap &data ) override;


};
}
#endif //OST_PROPERTYUPDATE_H
