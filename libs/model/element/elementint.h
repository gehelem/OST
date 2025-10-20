/**
 * @file elementint.h
 * @brief Integer element implementation with LOV (List of Values) support
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementInt (integer element with min/max/step/slider support)
 * and ValueInt (integer value container for grid storage).
 */

#ifndef ELEMENTINT_h
#define ELEMENTINT_h

#include <elementtemplate.h>
#include <lovint.h>

namespace  OST
{

/**
 * @class ElementInt
 * @brief Integer element with validation and LOV support
 *
 * ElementInt is a concrete element type for integer values. It inherits from
 * ElementTemplateNumeric<int> which provides:
 * - Min/max value validation
 * - Step increment for spinboxes
 * - Format string for display
 * - Slider UI hint
 *
 * Additionally, ElementInt supports LOV (List of Values) for dropdown/radio selection.
 * The LOV maps integer values to display labels, allowing users to select from
 * predefined options instead of free-form input.
 *
 * @par Basic Usage (numeric input):
 * @code
 * auto iterations = new ElementInt("Iterations", "10", "Number of focus iterations");
 * iterations->setMinMax(1, 20);       // Allow 1-20
 * iterations->setStep(1);             // Increment by 1
 * iterations->setValue(5, false);     // Set initial value
 * property->addElt("iterations", iterations);
 * @endcode
 *
 * @par LOV Usage (dropdown selection):
 * @code
 * auto binning = new ElementInt("Binning", "20", "Camera binning mode");
 * binning->lovAdd(1, "1x1");          // Full resolution
 * binning->lovAdd(2, "2x2");          // 2x2 binning
 * binning->lovAdd(4, "4x4");          // 4x4 binning
 * binning->setValue(1, false);        // Default to 1x1
 * property->addElt("binning", binning);
 * // Frontend displays dropdown: [1x1, 2x2, 4x4]
 * @endcode
 *
 * @par Slider Usage:
 * @code
 * auto gain = new ElementInt("Gain", "30", "Sensor gain");
 * gain->setMinMax(0, 100);
 * gain->setSlider(SliderHorizontal);  // Display as slider
 * gain->setValue(50, false);
 * @endcode
 *
 * @see ElementTemplateNumeric
 * @see ElementFloat
 * @see LovInt
 */
class ElementInt: public ElementTemplateNumeric<int>
{

        Q_OBJECT

    public:
        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         *
         * Implements Visitor pattern for operations like JSON serialization.
         */
        void accept(ElementVisitor *pVisitor) override;

        /**
         * @brief Accept a visitor with data parameter
         * @param pVisitor Pointer to visitor object
         * @param data QVariantMap with operation data
         *
         * Used for operations requiring external data (e.g., updates from frontend).
         */
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;

        /**
         * @brief Accept a visitor with action and data parameters
         * @param pVisitor Pointer to visitor object
         * @param action Action string (e.g., "newline", "cleargrid")
         * @param data QVariantMap with operation data
         *
         * Used for grid operations and custom actions.
         */
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        /**
         * @brief Construct a new ElementInt object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementInt(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementInt object
         */
        ~ElementInt();

        /**
         * @brief Get element type as string
         * @return "int"
         *
         * Used for type identification and casting.
         */
        QString getType() override;

        /**
         * @brief Get List of Values (LOV)
         * @return QMap<int, QString> mapping values to display labels
         *
         * Returns the current LOV for dropdown/radio display in frontend.
         * Empty map means no LOV (free-form numeric input).
         *
         * @par Example:
         * @code
         * QMap<int, QString> lov = element->getLov();
         * // lov might contain: {1: "1x1", 2: "2x2", 4: "4x4"}
         * @endcode
         */
        QMap<int, QString> getLov();

        /**
         * @brief Add value to LOV
         * @param val Integer value
         * @param label Display label for this value
         * @return true if added, false if value already exists
         *
         * Adds a new option to the List of Values. If the value already exists,
         * returns false without modifying the LOV.
         *
         * @see lovUpdate()
         */
        bool lovAdd(int val, QString label);

        /**
         * @brief Update LOV value label
         * @param val Integer value to update
         * @param label New display label
         * @return true if updated, false if value not found
         *
         * Updates the display label for an existing LOV value.
         * If the value doesn't exist in the LOV, returns false.
         *
         * @see lovAdd()
         */
        bool lovUpdate(int  val, QString label);

        /**
         * @brief Delete value from LOV
         * @param val Integer value to delete
         * @return true if deleted, false if value not found
         *
         * Removes an option from the List of Values.
         */
        bool lovDel(int  val);

        /**
         * @brief Clear all LOV entries
         * @return true if cleared successfully
         *
         * Removes all options from the List of Values, reverting to
         * free-form numeric input mode.
         */
        bool lovClear();

    private:
        LovInt mLov;  /*!< List of Values for dropdown/radio selection */
};

/**
 * @class ValueInt
 * @brief Integer value container for grid storage
 *
 * ValueInt is the grid value copy for ElementInt. It stores an integer value
 * and provides methods to synchronize with its parent ElementInt.
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueInt = new ValueInt(iterationsElement);
 * valueInt->updateValue();  // Copy element->value() to valueInt->value
 * gridLine["iterations"] = valueInt;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * valueInt->updateElement(true);  // Copy valueInt->value to element->value()
 * @endcode
 *
 * @note This class is used internally by PropertyMulti for grid operations.
 *       Module code typically doesn't interact with ValueInt directly.
 *
 * @see ValueTemplate
 * @see ElementInt
 * @see PropertyMulti::push()
 * @see PropertyMulti::fetchLine()
 */
class ValueInt: public ValueTemplate<int>
{
        Q_OBJECT

    public:

        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         *
         * Implements Visitor pattern for grid value serialization.
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValueInt object
         * @param element Pointer to parent ElementInt
         *
         * Creates a value container linked to its parent element.
         */
        ValueInt(ElementBase* element): ValueTemplate<int>(element) {}

        /**
         * @brief Destroy the ValueInt object
         */
        ~ValueInt() {}

        /**
         * @brief Update value from parent element
         *
         * Copies the parent element's current value into this value object.
         * Called by PropertyMulti::push() when creating grid lines.
         */
        void updateValue() override;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this value back into the parent element.
         * Called by PropertyMulti::fetchLine() when restoring elements from grid.
         */
        void updateElement(const bool &emitEvent) override;

};

}

#endif
