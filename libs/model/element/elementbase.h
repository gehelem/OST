/**
 * @file elementbase.h
 * @brief Base classes for OST typed elements and their value containers
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementBase (base for all typed elements like int, float, string)
 * and ValueBase (base for value containers used in grid lines). Elements hold typed
 * values and metadata, while Values are lightweight copies for grid storage.
 */

#ifndef ELEMENTBASE_h_
#define ELEMENTBASE_h_

#include <elementvisitor.h>
#include <common.h>

namespace  OST
{

class ElementVisitor;
class ValueVisitor;

/**
 * @class ElementBase
 * @brief Abstract base class for all typed elements in OST properties
 *
 * ElementBase provides the common interface for typed elements (int, float, string, bool, etc.).
 * Each element has:
 * - A typed value (implemented in derived classes via templates)
 * - Display metadata (label, order, hint)
 * - Behavior flags (autoUpdate, directEdit)
 * - UI hints (badge, icons)
 * - Optional List of Values (LOV) for dropdowns/radios
 *
 * Elements are contained within PropertyMulti and can be:
 * - Base elements: Single values displayed in property
 * - Grid elements: Values copied to grid lines for tabular data
 *
 * @par Element Hierarchy:
 * @code
 * ElementBase (abstract)
 *   └─ ElementTemplate<T> (abstract template)
 *       ├─ ElementTemplateNumeric<T> (for int, float with min/max/slider)
 *       │   ├─ ElementInt
 *       │   └─ ElementFloat
 *       └─ ElementTemplateNotNumeric<T> (for string, bool, date, time)
 *           ├─ ElementString
 *           ├─ ElementBool
 *           ├─ ElementDate
 *           └─ ElementTime
 * @endcode
 *
 * @par Usage Example:
 * @code
 * auto iterations = new ElementInt("Iterations", "10", "Number of focus iterations");
 * iterations->setValue(5, false);
 * iterations->setMinMax(1, 20);
 * iterations->setAutoUpdate(true);
 * property->addElt("iterations", iterations);
 * @endcode
 *
 * @see PropertyMulti
 * @see ElementTemplate
 */
class ElementBase: public QObject
{

        Q_OBJECT

    public:
        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         *
         * Pure virtual method for Visitor pattern implementation.
         * Used for JSON serialization, text dumping, etc.
         */
        virtual void accept(ElementVisitor* pVisitor) = 0;

        /**
         * @brief Accept a visitor with data parameter
         * @param pVisitor Pointer to visitor object
         * @param data QVariantMap with operation data
         *
         * Overloaded accept for operations requiring external data.
         */
        virtual void accept(ElementVisitor* pVisitor, QVariantMap &data) = 0;

        /**
         * @brief Accept a visitor with action and data parameters
         * @param pVisitor Pointer to visitor object
         * @param action Action string (e.g., "newline", "cleargrid")
         * @param data QVariantMap with operation data
         *
         * Overloaded accept for grid operations and custom actions.
         */
        virtual void accept(ElementVisitor* pVisitor, QString &action, QVariantMap &data) = 0;

        /**
         * @brief Construct a new ElementBase object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementBase(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementBase object
         */
        ~ElementBase();

        /**
         * @brief Get element type as string
         * @return Type identifier ("int", "float", "string", "bool", etc.)
         *
         * Pure virtual method implemented by derived classes.
         * Used for type checking and casting.
         */
        virtual QString getType() = 0;

        /**
         * @brief Get display label
         * @return Label string
         */
        QString label();

        /**
         * @brief Get sort order
         * @return Order string (e.g., "10")
         */
        QString order();

        /**
         * @brief Get hint/tooltip text
         * @return Hint string
         */
        QString hint();

        /**
         * @brief Check if element has auto-update enabled
         * @return true if auto-update enabled
         *
         * When true, frontend changes automatically propagate to backend.
         * When false, user must explicitly save/apply changes.
         */
        bool autoUpdate();

        /**
         * @brief Set auto-update flag
         * @param v true to enable, false to disable
         *
         * @par Usage:
         * - Enable for interactive controls (sliders, spinboxes)
         * - Disable for text fields that should validate on Enter/blur
         */
        void setAutoUpdate(const bool &v);

        /**
         * @brief Check if direct edit is enabled
         * @return true if direct edit enabled
         *
         * When true, frontend allows inline editing (click-to-edit).
         * When false, element is read-only or requires separate edit dialog.
         */
        bool getDirectEdit();

        /**
         * @brief Set direct edit flag
         * @param v true to enable, false to disable
         */
        void setDirectEdit(const bool &v);

        /**
         * @brief Get badge visibility
         * @return true if badge shown
         *
         * Badge is a visual indicator (notification dot) on the element.
         */
        bool getBadge();

        /**
         * @brief Set badge visibility
         * @param b true to show badge, false to hide
         */
        void setBadge(const bool &b);

        /**
         * @brief Get prefix icon identifier
         * @return Icon identifier string (e.g., "fa-thermometer")
         */
        QString getPreIcon();

        /**
         * @brief Set prefix icon
         * @param s Icon identifier (FontAwesome, Material Icons, etc.)
         */
        void setPreIcon(QString s);

        /**
         * @brief Get postfix icon identifier
         * @return Icon identifier string
         */
        QString getPostIcon();

        /**
         * @brief Set postfix icon
         * @param s Icon identifier
         */
        void setPostIcon(QString s);

        /**
         * @brief Get global LOV name
         * @return Global LOV key, or empty string if none
         *
         * Global LOVs are shared across properties (e.g., list of loaded modules).
         * Local LOVs are specific to each element.
         */
        QString getGlobalLov();

        /**
         * @brief Set global LOV reference
         * @param lovName Key of global LOV in Datastore::mGlobLov
         *
         * Links element to a global LOV for dynamic dropdowns.
         */
        void setGlobalLov(QString lovName);

    private:
        QString mLabel = "change me";     /*!< Display label for frontend */
        QString mOrder = "change me";     /*!< Sort order within property */
        QString mHint = "";               /*!< Tooltip/help text */
        bool mAutoUpdate = false;         /*!< Auto-propagate changes to backend */
        bool mDirectEdit = false;         /*!< Allow inline editing in frontend */
        bool mBadge = false;              /*!< Show notification badge */
        QString mPreIcon = "";            /*!< Prefix icon identifier */
        QString mPostIcon = "";           /*!< Postfix icon identifier */
        QString mGlobalLov = "";          /*!< Reference to global LOV key */

    public slots:
        /**
         * @brief Slot called when linked LOV changes
         *
         * Handles updates from global LOVs.
         */
        void OnLovChanged();

    signals:
        /**
         * @brief Signal emitted when element value changes
         * @param elt Pointer to this element
         *
         * Emitted by derived template classes when setValue() is called.
         * Propagated to parent property's OnValueSet slot.
         */
        void valueSet(OST::ElementBase*);

        /**
         * @brief Signal emitted when element metadata changes
         * @param elt Pointer to this element
         *
         * Emitted when non-value attributes change (min/max, LOV, icons, etc.).
         */
        void eltChanged(OST::ElementBase*);

        /**
         * @brief Signal emitted when element list/LOV changes
         * @param elt Pointer to this element
         */
        void listChanged(OST::ElementBase*);

        /**
         * @brief Signal emitted when sending messages
         * @param level Message level (Info, Warn, Err)
         * @param message Message text
         *
         * Propagated to property and module levels.
         */
        void sendMessage(MsgLevel, QString);

        /**
         * @brief Signal emitted when LOV changes
         * @param elt Pointer to this element
         */
        void lovChanged(OST::ElementBase*);

        /**
         * @brief Signal emitted for grid-related events
         *
         * Generic signal for grid operations.
         */
        void gridEvent();

};

/**
 * @class ValueBase
 * @brief Abstract base class for value containers used in grid lines
 *
 * ValueBase provides lightweight value copies for grid storage. Each grid line
 * contains a map of element keys to ValueBase-derived objects (ValueInt, ValueFloat, etc.).
 *
 * Values serve two purposes:
 * - Store element values in grid lines (read from element via updateValue())
 * - Restore element values from grid (write to element via updateElement())
 *
 * @par Usage (internal):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueInt = new ValueInt(element);
 * valueInt->updateValue();  // Copy element->value() to value->value
 * gridLine["iterations"] = valueInt;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * auto valueInt = dynamic_cast<ValueInt*>(gridLine["iterations"]);
 * valueInt->updateElement(true);  // Copy value->value to element->value()
 * @endcode
 *
 * @see ElementBase
 * @see PropertyMulti::push()
 * @see PropertyMulti::fetchLine()
 */
class ValueBase: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         *
         * Pure virtual method for value serialization.
         */
        virtual void accept(ValueVisitor* pVisitor) = 0;

        /**
         * @brief Construct a new ValueBase object
         * @param element Pointer to parent element
         */
        ValueBase(ElementBase* element);

        /**
         * @brief Destroy the ValueBase object
         */
        ~ValueBase();

        /**
         * @brief Update value from parent element
         *
         * Copies element's current value into this value object.
         * Called when pushing element to grid.
         */
        virtual void updateValue() = 0;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this value back into parent element.
         * Called when fetching grid line into elements.
         */
        virtual void updateElement(const bool &emitEvent) = 0;

    protected:
        ElementBase* pElement;  /*!< Pointer to parent element */
};

}

#endif

