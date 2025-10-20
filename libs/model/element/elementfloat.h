/**
 * @file elementfloat.h
 * @brief Floating-point element implementation with LOV support
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementFloat (double-precision floating-point element with
 * min/max/step/slider support) and ValueFloat (float value container for grid storage).
 */

#ifndef ELEMENTFLOAT_h
#define ELEMENTFLOAT_h

#include <elementtemplate.h>
#include <lovfloat.h>

namespace  OST
{

/**
 * @class ElementFloat
 * @brief Floating-point element with validation and LOV support
 *
 * ElementFloat is a concrete element type for double-precision floating-point values.
 * It inherits from ElementTemplateNumeric<double> which provides:
 * - Min/max value validation
 * - Step increment for spinboxes
 * - Format string for precision control (e.g., "%.3f" for 3 decimals)
 * - Slider UI hint
 *
 * Additionally, ElementFloat supports LOV (List of Values) for dropdown/radio selection
 * of predefined float values (e.g., common exposure times, focus step sizes).
 *
 * @par Basic Usage (numeric input):
 * @code
 * auto exposure = new ElementFloat("Exposure", "10", "Exposure time in seconds");
 * exposure->setMinMax(0.001, 3600.0);  // Min 1ms, max 1 hour
 * exposure->setStep(0.1);              // Increment by 0.1s
 * exposure->setFormat("%.3f");         // Display 3 decimal places
 * exposure->setValue(10.0, false);     // Set initial value to 10s
 * property->addElt("exposure", exposure);
 * @endcode
 *
 * @par LOV Usage (predefined values):
 * @code
 * auto stepSize = new ElementFloat("Step Size", "20", "Focus step size");
 * stepSize->lovAdd(0.1, "Fine");       // 0.1 micron steps
 * stepSize->lovAdd(1.0, "Medium");     // 1.0 micron steps
 * stepSize->lovAdd(10.0, "Coarse");    // 10.0 micron steps
 * stepSize->setValue(1.0, false);      // Default to medium
 * property->addElt("stepsize", stepSize);
 * // Frontend displays dropdown: [Fine, Medium, Coarse]
 * @endcode
 *
 * @par Slider with Format:
 * @code
 * auto temp = new ElementFloat("Temperature", "30", "Sensor temperature");
 * temp->setMinMax(-20.0, 30.0);
 * temp->setFormat("%.1f");             // One decimal place
 * temp->setSlider(SliderHorizontal);   // Display as slider
 * temp->setValue(15.5, false);
 * @endcode
 *
 * @note Uses double (not float) for extended precision in astronomical calculations
 *
 * @see ElementTemplateNumeric
 * @see ElementInt
 * @see LovFloat
 */
class ElementFloat: public ElementTemplateNumeric<double>
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
         * @brief Construct a new ElementFloat object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementFloat(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementFloat object
         */
        ~ElementFloat();

        /**
         * @brief Get element type as string
         * @return "float"
         *
         * Used for type identification and casting.
         */
        QString getType() override;

        /**
         * @brief Get List of Values (LOV)
         * @return QMap<double, QString> mapping values to display labels
         *
         * Returns the current LOV for dropdown/radio display in frontend.
         * Empty map means no LOV (free-form numeric input).
         *
         * @par Example:
         * @code
         * QMap<double, QString> lov = element->getLov();
         * // lov might contain: {0.1: "Fine", 1.0: "Medium", 10.0: "Coarse"}
         * @endcode
         */
        QMap<double, QString> getLov();

        /**
         * @brief Add value to LOV
         * @param val Double value
         * @param label Display label for this value
         * @return true if added, false if value already exists
         *
         * Adds a new option to the List of Values. If the value already exists,
         * returns false without modifying the LOV.
         *
         * @see lovUpdate()
         */
        bool lovAdd(double val, QString label);

        /**
         * @brief Update LOV value label
         * @param val Double value to update
         * @param label New display label
         * @return true if updated, false if value not found
         *
         * Updates the display label for an existing LOV value.
         * If the value doesn't exist in the LOV, returns false.
         *
         * @see lovAdd()
         */
        bool lovUpdate(double  val, QString label);

        /**
         * @brief Delete value from LOV
         * @param val Double value to delete
         * @return true if deleted, false if value not found
         *
         * Removes an option from the List of Values.
         */
        bool lovDel(double  val);

        /**
         * @brief Clear all LOV entries
         * @return true if cleared successfully
         *
         * Removes all options from the List of Values, reverting to
         * free-form numeric input mode.
         */
        bool lovClear();

    private:
        LovFloat mLov;  /*!< List of Values for dropdown/radio selection */

};

/**
 * @class ValueFloat
 * @brief Floating-point value container for grid storage
 *
 * ValueFloat is the grid value copy for ElementFloat. It stores a double-precision
 * value and provides methods to synchronize with its parent ElementFloat.
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueFloat = new ValueFloat(exposureElement);
 * valueFloat->updateValue();  // Copy element->value() to valueFloat->value
 * gridLine["exposure"] = valueFloat;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * valueFloat->updateElement(true);  // Copy valueFloat->value to element->value()
 * @endcode
 *
 * @note This class is used internally by PropertyMulti for grid operations.
 *       Module code typically doesn't interact with ValueFloat directly.
 *
 * @see ValueTemplate
 * @see ElementFloat
 * @see PropertyMulti::push()
 * @see PropertyMulti::fetchLine()
 */
class ValueFloat: public ValueTemplate<double>
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
         * @brief Construct a new ValueFloat object
         * @param element Pointer to parent ElementFloat
         *
         * Creates a value container linked to its parent element.
         */
        ValueFloat(ElementBase* element): ValueTemplate<double>(element) {}

        /**
         * @brief Destroy the ValueFloat object
         */
        ~ValueFloat() {}

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
