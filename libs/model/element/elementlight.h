/**
 * @file elementlight.h
 * @brief Light/status indicator element implementation
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementLight (status indicator light element) and
 * ValueLight (light status container for grid storage).
 */

#ifndef ELEMENTLIGHT_h
#define ELEMENTLIGHT_h

#include <elementtemplate.h>

namespace  OST
{

/**
 * @class ElementLight
 * @brief Status indicator light element (colored state display)
 *
 * ElementLight is a specialized element type for visual status indicators.
 * It uses the State enum (Idle, Ok, Busy, Error) to display color-coded
 * status lights in the frontend UI.
 *
 * Typical status colors:
 * - State::Idle: Gray (inactive/waiting)
 * - State::Ok: Green (success/ready)
 * - State::Busy: Yellow/Orange (working/in progress)
 * - State::Error: Red (failure/problem)
 *
 * Common uses:
 * - Device connection status
 * - Operation status indicators
 * - Health monitoring lights
 * - Safety status (weather, equipment)
 * - System component states
 *
 * @par Basic Usage (connection status):
 * @code
 * auto connectionLight = new ElementLight("Camera", "10", "Camera connection status");
 * connectionLight->setValue(State::Idle, false);  // Start as disconnected
 * property->addElt("cameraStatus", connectionLight);
 *
 * // When connected:
 * connectionLight->setValue(State::Ok, true);     // Green light
 *
 * // When error:
 * connectionLight->setValue(State::Error, true);  // Red light
 * @endcode
 *
 * @par Operation Status:
 * @code
 * auto focusLight = new ElementLight("Focus", "20", "Autofocus status");
 * focusLight->setValue(State::Idle, false);       // Ready
 *
 * // During focus run:
 * focusLight->setValue(State::Busy, true);        // Yellow/working
 *
 * // On completion:
 * focusLight->setValue(State::Ok, true);          // Green/success
 *
 * // On failure:
 * focusLight->setValue(State::Error, true);       // Red/failed
 * @endcode
 *
 * @par Weather Safety:
 * @code
 * auto weatherLight = new ElementLight("Weather", "30", "Weather safety status");
 * weatherLight->setValue(State::Ok, false);       // Safe (green)
 *
 * // When clouds detected:
 * weatherLight->setValue(State::Busy, true);      // Warning (yellow)
 *
 * // When unsafe:
 * weatherLight->setValue(State::Error, true);     // Unsafe (red)
 * @endcode
 *
 * @note State enum is defined in common.h
 * @note Frontend displays as colored indicator (circle, LED, icon)
 * @note Similar to INDI protocol IPState (Idle/Ok/Busy/Alert)
 *
 * @see State
 * @see PropertyBase::state() (property-level state)
 * @see ElementTemplateNotNumeric
 */
class ElementLight: public ElementTemplateNotNumeric<State>
{

        Q_OBJECT

    public:
        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ElementVisitor *pVisitor) override;

        /**
         * @brief Accept a visitor with data parameter
         * @param pVisitor Pointer to visitor object
         * @param data QVariantMap with operation data
         */
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;

        /**
         * @brief Accept a visitor with action and data parameters
         * @param pVisitor Pointer to visitor object
         * @param action Action string (e.g., "newline", "cleargrid")
         * @param data QVariantMap with operation data
         */
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        /**
         * @brief Construct a new ElementLight object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementLight(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementLight object
         */
        ~ElementLight();

        /**
         * @brief Get element type as string
         * @return "light"
         */
        QString getType() override;

    private:

};

/**
 * @class ValueLight
 * @brief Light status container for grid storage
 *
 * ValueLight is the grid value copy for ElementLight. It stores State values
 * and provides methods to synchronize with its parent ElementLight.
 *
 * Useful for storing status history in grids (e.g., operation log with
 * status indicators per line).
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueLight = new ValueLight(statusElement);
 * valueLight->updateValue();  // Copy element->value() to valueLight->value
 * gridLine["status"] = valueLight;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * valueLight->updateElement(true);  // Copy valueLight->value to element->value()
 * @endcode
 *
 * @see ValueTemplate
 * @see ElementLight
 * @see State
 * @see PropertyMulti::push()
 */
class ValueLight: public ValueTemplate<State>
{
        Q_OBJECT

    public:

        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValueLight object
         * @param element Pointer to parent ElementLight
         */
        ValueLight(ElementBase* element): ValueTemplate<State>(element) {}

        /**
         * @brief Destroy the ValueLight object
         */
        ~ValueLight() {}

        /**
         * @brief Update value from parent element
         *
         * Copies the parent element's current state value into this value object.
         */
        void updateValue() override;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this state value back into the parent element.
         */
        void updateElement(const bool &emitEvent) override;

};

}
#endif
