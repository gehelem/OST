/**
 * @file elementbool.h
 * @brief Boolean element implementation
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementBool (boolean/checkbox element) and
 * ValueBool (boolean value container for grid storage).
 */

#ifndef ELEMENTBOOL_h
#define ELEMENTBOOL_h

#include <elementtemplate.h>

namespace  OST
{

/**
 * @class ElementBool
 * @brief Boolean element for checkboxes and switches
 *
 * ElementBool is a concrete element type for boolean (true/false) values. It inherits
 * from ElementTemplateNotNumeric<bool> and is typically displayed as:
 * - Checkbox (individual toggles)
 * - Radio button (when grouped with SwitchsRule::OneOfMany)
 * - Toggle switch in frontend UI
 *
 * Boolean elements can be coordinated using SwitchsRule in PropertyMulti:
 * - SwitchsRule::OneOfMany: Radio button behavior (exactly one true)
 * - SwitchsRule::AtMostOne: Optional radio (zero or one true)
 * - SwitchsRule::Any: Independent checkboxes (no constraint)
 *
 * @par Basic Usage (independent checkbox):
 * @code
 * auto autoDark = new ElementBool("Auto Dark", "10", "Enable automatic dark frames");
 * autoDark->setValue(false, false);  // Initially disabled
 * property->addElt("autodark", autoDark);
 * @endcode
 *
 * @par Radio Button Group (OneOfMany rule):
 * @code
 * auto actions = new PropertyMulti("actions", "Actions", ReadWrite, "Control", "", "010", true, false);
 * actions->setRule(SwitchsRule::OneOfMany);  // Exactly one must be true
 *
 * auto calibrate = new ElementBool("Calibrate", "10", "Run calibration");
 * auto guide = new ElementBool("Guide", "20", "Start guiding");
 * auto stop = new ElementBool("Stop", "30", "Stop operations");
 *
 * actions->addElt("calibrate", calibrate);
 * actions->addElt("guide", guide);
 * actions->addElt("stop", stop);
 *
 * calibrate->setValue(true, false);  // Select calibrate
 * // guide and stop are automatically set to false by PropertyMulti
 * @endcode
 *
 * @par Enable/Disable Feature:
 * @code
 * auto coolerOn = new ElementBool("Cooler", "40", "Camera cooler on/off");
 * coolerOn->setValue(true, false);   // Turn cooler on
 * @endcode
 *
 * @note Boolean elements don't use LOV - value is always true or false
 *
 * @see ElementTemplateNotNumeric
 * @see PropertyMulti::setRule()
 * @see SwitchsRule
 */
class ElementBool: public ElementTemplateNotNumeric<bool>
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
         * @brief Construct a new ElementBool object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementBool(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementBool object
         */
        ~ElementBool();

        /**
         * @brief Get element type as string
         * @return "bool"
         */
        QString getType() override;

};

/**
 * @class ValueBool
 * @brief Boolean value container for grid storage
 *
 * ValueBool is the grid value copy for ElementBool. It stores a boolean value
 * and provides methods to synchronize with its parent ElementBool.
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueBool = new ValueBool(enabledElement);
 * valueBool->updateValue();  // Copy element->value() to valueBool->value
 * gridLine["enabled"] = valueBool;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * valueBool->updateElement(true);  // Copy valueBool->value to element->value()
 * @endcode
 *
 * @see ValueTemplate
 * @see ElementBool
 * @see PropertyMulti::push()
 */
class ValueBool: public ValueTemplate<bool>
{
        Q_OBJECT

    public:
        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValueBool object
         * @param element Pointer to parent ElementBool
         */
        ValueBool(ElementBase* element): ValueTemplate<bool>(element) {}

        /**
         * @brief Destroy the ValueBool object
         */
        ~ValueBool() {}

        /**
         * @brief Update value from parent element
         *
         * Copies the parent element's current value into this value object.
         */
        void updateValue() override;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this value back into the parent element.
         */
        void updateElement(const bool &emitEvent) override;

};

}
#endif
