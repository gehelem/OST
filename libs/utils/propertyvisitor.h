/**
 * @file propertyvisitor.h
 * @brief Abstract visitor interface for property traversal and operations
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines the PropertyVisitor abstract base class implementing the
 * Visitor design pattern for properties. Visitors allow performing operations
 * on property hierarchies without modifying property classes themselves.
 *
 * Concrete implementations:
 * - PropertyJsonDumper: Serializes properties to JSON for WebSocket transmission
 * - PropertyTextDumper: Converts properties to human-readable text
 * - PropertyUpdate: Updates property values from QVariantMap data
 */

#ifndef OST_PROPERTYVISITOR_H
#define OST_PROPERTYVISITOR_H
#include <QtCore>
namespace  OST
{

class PropertyBase;
class PropertyMulti;

/**
 * @class PropertyVisitor
 * @brief Abstract base class for property visitor pattern implementation
 *
 * PropertyVisitor defines the interface for operations on property objects using
 * the Visitor design pattern. This allows adding new operations without modifying
 * property classes.
 *
 * Each property type (PropertyBase, PropertyMulti) accepts visitors via their
 * accept() methods, which call back to the appropriate visit() method.
 *
 * @par Visitor Pattern Flow:
 * @code
 * PropertyMulti* property = ...;
 * PropertyJsonDumper dumper;
 * property->accept(&dumper);  // Calls dumper.visit(property)
 * QJsonObject json = dumper.getResult();
 * @endcode
 *
 * @par Implementing a Custom Visitor:
 * @code
 * class MyCustomVisitor : public PropertyVisitor
 * {
 * public:
 *     void visit(PropertyBase* pProperty) override {
 *         // Handle PropertyBase
 *     }
 *     void visit(PropertyMulti* pProperty) override {
 *         // Handle PropertyMulti - access elements, grid, etc.
 *     }
 *     void visit(PropertyBase* pProperty, QVariantMap &data) override {
 *         // Handle PropertyBase with data
 *     }
 *     void visit(PropertyMulti* pProperty, QVariantMap &data) override {
 *         // Handle PropertyMulti with data
 *     }
 * };
 * @endcode
 *
 * @see PropertyJsonDumper
 * @see PropertyTextDumper
 * @see PropertyUpdate
 * @see PropertyBase::accept()
 * @see PropertyMulti::accept()
 */
class PropertyVisitor
{

    protected:
        /**
         * @brief Protected default constructor
         *
         * Constructor is protected to enforce abstract base class usage.
         * Only derived classes can be instantiated.
         */
        PropertyVisitor() = default;

    public:
        /**
         * @brief Visit a PropertyBase object
         * @param pProperty Pointer to PropertyBase to visit
         *
         * Pure virtual method called when visiting a PropertyBase.
         * Derived classes implement specific behavior (e.g., serialize, update).
         */
        virtual void visit(PropertyBase* pProperty) = 0;

        /**
         * @brief Visit a PropertyBase object with data parameter
         * @param pProperty Pointer to PropertyBase to visit
         * @param data QVariantMap containing operation data
         *
         * Overloaded visit method for operations requiring external data,
         * such as updating property values from frontend.
         */
        virtual void visit(PropertyBase* pProperty, QVariantMap &data ) = 0;

        /**
         * @brief Visit a PropertyMulti object
         * @param pProperty Pointer to PropertyMulti to visit
         *
         * Pure virtual method called when visiting a PropertyMulti.
         * Provides access to elements, grid, and multi-property features.
         */
        virtual void visit(PropertyMulti* pProperty) = 0;

        /**
         * @brief Visit a PropertyMulti object with data parameter
         * @param pProperty Pointer to PropertyMulti to visit
         * @param data QVariantMap containing operation data
         *
         * Overloaded visit method for operations requiring external data.
         * Used for grid operations, element updates, etc.
         */
        virtual void visit(PropertyMulti* pProperty, QVariantMap &data ) = 0;

};

}
#endif //OST_PROPERTYVISITOR_H
