/**
 * @file elementtemplate.h
 * @brief Template classes for typed elements with numeric/non-numeric specializations
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines the template hierarchy that implements typed elements in OST.
 * The template system splits into two branches:
 * - ElementTemplateNumeric<T>: For numeric types (int, float) with min/max/step/slider support
 * - ElementTemplateNotNumeric<T>: For non-numeric types (string, bool, date, time, etc.)
 *
 * Also defines ValueTemplate<T> for grid value storage.
 */

#ifndef ELEMENTTEMPLATE_h
#define ELEMENTTEMPLATE_h

#include <elementbase.h>
#include <lovsingle.h>

namespace  OST
{

/**
 * @class ElementTemplate
 * @brief Base template class for all typed elements
 * @tparam T The value type (int, float, QString, bool, QDate, QTime, etc.)
 *
 * ElementTemplate provides the base for typed element storage. It stores a value of
 * type T and provides basic accessors. Derived classes add type-specific behavior
 * (numeric validation, LOV handling, etc.).
 *
 * @par Template Hierarchy:
 * @code
 * ElementTemplate<T> (abstract base)
 *   ├─ ElementTemplateNumeric<T> (for int, float)
 *   │   ├─ ElementInt
 *   │   └─ ElementFloat
 *   └─ ElementTemplateNotNumeric<T> (for QString, bool, QDate, QTime, etc.)
 *       ├─ ElementString
 *       ├─ ElementBool
 *       ├─ ElementDate
 *       └─ ElementTime
 * @endcode
 *
 * @note This class is not instantiated directly. Use concrete types like ElementInt,
 *       ElementFloat, ElementString, etc.
 *
 * @see ElementTemplateNumeric
 * @see ElementTemplateNotNumeric
 * @see ElementBase
 */
template <typename T>
class ElementTemplate: public ElementBase
{

    public:

        /**
         * @brief Construct a new ElementTemplate object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         *
         * Constructs the base element with metadata. The value is default-initialized.
         */
        ElementTemplate(const QString &label, const QString &order, const QString &hint):
            ElementBase(label, order, hint) {}

        /**
         * @brief Destroy the ElementTemplate object
         */
        ~ElementTemplate() {}

        /**
         * @brief Get current value
         * @return Current value of type T
         *
         * Returns a copy of the stored value. For complex types like QString,
         * consider using const reference returns in production code.
         *
         * @par Example:
         * @code
         * auto intElt = dynamic_cast<ElementInt*>(element);
         * int val = intElt->value();  // Returns int
         * @endcode
         */
        T value()
        {
            return mValue;
        }

        /**
         * @brief Get element type as string
         * @return Type identifier (should be overridden by derived classes)
         *
         * @warning Base implementation returns "error". Derived classes must override
         *          to return proper type strings ("int", "float", "string", etc.)
         *
         * @see getRealType()
         */
        QString getType() override
        {
            return "error";
        }

        /**
         * @brief Get C++ type information
         * @return Mangled C++ type name from typeid()
         *
         * Returns the raw C++ type name as provided by typeid(T).name().
         * This is primarily for debugging and type introspection.
         * For frontend communication, use getType() instead.
         *
         * @note Type names are compiler-dependent and mangled
         */
        QString getRealType()
        {
            return typeid(T).name();
        }

    protected:
        T mValue;  /*!< Stored value of type T */
};
/**
 * @class ElementTemplateNumeric
 * @brief Template specialization for numeric elements (int, float) with validation
 * @tparam T Numeric type (int, float, double)
 *
 * ElementTemplateNumeric adds numeric-specific features to ElementTemplate:
 * - Min/max value validation
 * - Step increment for spinboxes/sliders
 * - Format string for display precision
 * - Slider UI hint (None/Horizontal/Vertical)
 *
 * Used as base for ElementInt and ElementFloat.
 *
 * @par Usage Example:
 * @code
 * auto exposure = new ElementFloat("Exposure", "10", "Exposure time in seconds");
 * exposure->setMinMax(0.001, 3600.0);  // Min 1ms, max 1 hour
 * exposure->setStep(0.1);              // Increment by 0.1s
 * exposure->setFormat("%.3f");         // Display 3 decimal places
 * exposure->setSlider(SliderHorizontal);  // Show as slider in UI
 * exposure->setValue(10.0, false);     // Set to 10s without emitting signal
 * @endcode
 *
 * @par Validation:
 * setValue() enforces min/max bounds if setMinMax() has been called.
 * Out-of-range values are rejected and emit a warning message.
 *
 * @see ElementInt
 * @see ElementFloat
 * @see ElementTemplate
 */
template <typename T>
class ElementTemplateNumeric : public ElementTemplate<T>
{

    public:

        /**
         * @brief Construct a new ElementTemplateNumeric object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementTemplateNumeric(const QString &label, const QString &order, const QString &hint):
            ElementTemplate<T>(label, order, hint)
        {
        }

        /**
         * @brief Destroy the ElementTemplateNumeric object
         */
        ~ElementTemplateNumeric<T>() {}

        /**
         * @brief Set value with optional signal emission
         * @param value New value to set
         * @param emitEvent Whether to emit valueSet() signal
         * @return true if value accepted, false if out of range
         *
         * Validates the value against min/max if set, then updates the stored value.
         * If emitEvent is true and validation succeeds, emits valueSet() signal
         * to notify observers.
         *
         * @par Usage:
         * @code
         * element->setValue(100, true);   // Set and notify observers
         * element->setValue(200, false);  // Set without notification (e.g., during init)
         * @endcode
         */
        bool setValue(const T &value, const bool &emitEvent)
        {
            bool r = setValue(value);
            if (r && emitEvent) emit ElementBase::valueSet(this);
            return r;
        }

        /**
         * @brief Set value without signal emission
         * @param value New value to set
         * @return true if value accepted, false if out of range
         *
         * Internal setValue that performs validation but does not emit signals.
         * If min/max validation is enabled and value is out of range,
         * emits a warning message and returns false without changing the value.
         *
         * @see setMinMax()
         */
        bool setValue(const T &value)
        {
            if (mUseMinMax)
            {
                if (value < mMin)
                {
                    emit ElementTemplate<T>::sendMessage(Warn,
                                                         "setValue - value too low " + QString::number(value) + " min= " + QString::number(mMin) );
                    return false;
                }
                if (value > mMax)
                {
                    emit ElementTemplate<T>::sendMessage(Warn,
                                                         "setValue - value too high " + QString::number(value) + " max= " + QString::number(mMax) );
                    return false;
                }
            }
            ElementTemplate<T>::mValue = value;
            return true;
        }

        /**
         * @brief Get minimum allowed value
         * @return Minimum value
         *
         * Returns the minimum bound set by setMinMax(). If min/max validation
         * is not enabled, returns the default (0).
         */
        T min()
        {
            return mMin;
        }

        /**
         * @brief Set minimum allowed value
         * @param min Minimum value
         *
         * Sets the minimum bound. Does not automatically enable validation -
         * use setMinMax() to enable min/max enforcement.
         * Emits eltChanged() signal to notify frontend of metadata change.
         */
        void setMin(const T &min)
        {
            mMin = min;
            emit ElementBase::eltChanged(this);
        }

        /**
         * @brief Get maximum allowed value
         * @return Maximum value
         */
        int max()
        {
            return mMax;
        }

        /**
         * @brief Set maximum allowed value
         * @param max Maximum value
         *
         * Sets the maximum bound. Does not automatically enable validation -
         * use setMinMax() to enable min/max enforcement.
         * Emits eltChanged() signal to notify frontend of metadata change.
         */
        void setMax(const T &max)
        {
            mMax = max;
            emit ElementBase::eltChanged(this);
        }

        /**
         * @brief Set min and max bounds and enable validation
         * @param min Minimum allowed value
         * @param max Maximum allowed value
         *
         * Sets both min and max bounds and enables validation.
         * After calling this, setValue() will reject out-of-range values.
         * Emits eltChanged() signal to notify frontend.
         *
         * @par Example:
         * @code
         * iterations->setMinMax(1, 100);  // Allow 1-100
         * iterations->setValue(150, false);  // Rejected, returns false
         * @endcode
         *
         * @see unSetMinMax()
         */
        void setMinMax(const T &min, const T &max)
        {
            mMin = min;
            mMax = max;
            mUseMinMax = true;
            emit ElementBase::eltChanged(this);
        }

        /**
         * @brief Disable min/max validation
         *
         * Clears min/max bounds and disables validation.
         * After calling this, setValue() will accept any value.
         * Emits eltChanged() signal to notify frontend.
         */
        void unSetMinMax(void)
        {
            mMin = 0;
            mMax = 0;
            mUseMinMax = false;
            emit ElementBase::eltChanged(this);
        }

        /**
         * @brief Get step increment
         * @return Step value for spinbox/slider increments
         *
         * The step defines the increment/decrement amount for UI controls.
         * For example, step=0.1 makes a spinbox increment by 0.1 per click.
         */
        T step()
        {
            return mStep;
        }

        /**
         * @brief Set step increment
         * @param step Step value for UI controls
         *
         * Sets the increment amount for spinboxes and sliders.
         * Emits eltChanged() signal to notify frontend.
         *
         * @par Example:
         * @code
         * element->setStep(0.01);  // Fine control for float
         * element->setStep(10);    // Coarse control for int
         * @endcode
         */
        void setStep(const T &step)
        {
            mStep = step;
            emit ElementBase::eltChanged(this);
        }

        /**
         * @brief Get display format string
         * @return Format string (e.g., "%.2f", "%d")
         *
         * The format string controls how the value is displayed in the frontend.
         * Uses printf-style format specifiers.
         */
        QString format()
        {
            return mFormat;
        }

        /**
         * @brief Set display format string
         * @param format Printf-style format string
         *
         * Sets the format for value display in frontend.
         * Emits eltChanged() signal to notify frontend.
         *
         * @par Examples:
         * @code
         * element->setFormat("%.3f");   // 3 decimal places: 1.234
         * element->setFormat("%.1f");   // 1 decimal place: 1.2
         * element->setFormat("%05d");   // Zero-padded int: 00042
         * @endcode
         */
        void setFormat(const QString &format)
        {
            mFormat = format;
            emit ElementBase::eltChanged(this);
        }

        /**
         * @brief Get slider UI hint
         * @return Current SliderRule (SliderNone, SliderHorizontal, SliderVertical)
         *
         * Indicates whether frontend should display a slider control.
         *
         * @see SliderRule
         */
        SliderRule slider()
        {
            return mSlider;
        }

        /**
         * @brief Set slider UI hint
         * @param s SliderRule value
         *
         * Controls whether and how frontend displays a slider control.
         * Emits eltChanged() signal to notify frontend.
         *
         * @par Usage:
         * @code
         * element->setSlider(SliderHorizontal);  // Show horizontal slider
         * element->setSlider(SliderVertical);    // Show vertical slider
         * element->setSlider(SliderNone);        // No slider, use spinbox
         * @endcode
         *
         * @see SliderRule
         */
        void setSlider(const SliderRule &s)
        {
            mSlider = s;
            emit ElementBase::eltChanged(this);
        }

    private:
        T mMin = 0;                       /*!< Minimum allowed value */
        T mMax = 0;                       /*!< Maximum allowed value */
        T mStep = 0;                      /*!< Step increment for UI controls */
        bool mUseMinMax = false;          /*!< Whether min/max validation is enabled */
        QString mFormat = "";             /*!< Printf-style format string for display */
        SliderRule mSlider = SliderNone;  /*!< Slider UI hint (None/Horizontal/Vertical) */
};
/**
 * @class ElementTemplateNotNumeric
 * @brief Template specialization for non-numeric elements (string, bool, date, time, etc.)
 * @tparam T Non-numeric type (QString, bool, QDate, QTime, QImage, etc.)
 *
 * ElementTemplateNotNumeric provides a simpler template for non-numeric types that
 * don't require validation or numeric-specific UI hints like sliders and format strings.
 *
 * Used as base for:
 * - ElementString (QString)
 * - ElementBool (bool)
 * - ElementDate (QDate)
 * - ElementTime (QTime)
 * - ElementImg (QImage)
 * - ElementVideo (QString - path)
 * - ElementLight (LightStatus enum)
 * - ElementPrg (int - progress 0-100)
 * - ElementMessage (QString)
 *
 * @par Usage Example:
 * @code
 * auto filterName = new ElementString("Filter", "10", "Current filter name");
 * filterName->setValue("Ha", false);   // Set without emitting signal
 * filterName->setValue("OIII", true);  // Set and emit valueSet()
 *
 * auto enabled = new ElementBool("Enabled", "20", "Enable feature");
 * enabled->setValue(true, true);
 * @endcode
 *
 * @note Unlike ElementTemplateNumeric, setValue() always succeeds (returns true)
 *       since there is no validation for non-numeric types.
 *
 * @see ElementString
 * @see ElementBool
 * @see ElementDate
 * @see ElementTime
 * @see ElementTemplate
 */
template <typename T>
class ElementTemplateNotNumeric : public ElementTemplate<T>
{

    public:

        /**
         * @brief Construct a new ElementTemplateNotNumeric object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementTemplateNotNumeric(const QString &label, const QString &order, const QString &hint):
            ElementTemplate<T>(label, order, hint)
        {
        }

        /**
         * @brief Destroy the ElementTemplateNotNumeric object
         */
        ~ElementTemplateNotNumeric<T>() {}

        /**
         * @brief Set value with optional signal emission
         * @param value New value to set
         * @param emitEvent Whether to emit valueSet() signal
         * @return Always returns true (no validation for non-numeric types)
         *
         * Updates the stored value and optionally emits valueSet() signal
         * to notify observers. Unlike numeric elements, this always succeeds.
         *
         * @par Usage:
         * @code
         * element->setValue("new value", true);   // Set and notify
         * element->setValue("init value", false); // Set without notification
         * @endcode
         */
        bool setValue(const T &value, const bool &emitEvent)
        {
            bool r = setValue(value);
            if (r && emitEvent) emit ElementBase::valueSet(this);
            return r;
        }

        /**
         * @brief Set value without signal emission
         * @param value New value to set
         * @return Always returns true (no validation)
         *
         * Internal setValue that updates the value without validation or signals.
         * Always succeeds since non-numeric types have no validation rules.
         */
        bool setValue(const T &value)
        {
            ElementTemplate<T>::mValue = value;
            return true;
        }

};

/**
 * @class ValueTemplate
 * @brief Template class for grid value storage
 * @tparam T Value type (int, float, QString, bool, QDate, QTime, etc.)
 *
 * ValueTemplate is a lightweight copy of an element's value used in grid storage.
 * Each grid line contains a map of element keys to ValueTemplate-derived objects
 * (ValueInt, ValueFloat, ValueString, etc.).
 *
 * The value is stored directly as a public member for fast access during
 * serialization and grid operations.
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueFloat = new ValueFloat(exposureElement);
 * valueFloat->updateValue();  // Copy exposureElement->value() → valueFloat->value
 * gridLine["exposure"] = valueFloat;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * auto valueFloat = dynamic_cast<ValueFloat*>(gridLine["exposure"]);
 * valueFloat->updateElement(true);  // Copy valueFloat->value → exposureElement->value()
 * @endcode
 *
 * @note This class is not used directly by module code. Grid operations are
 *       handled automatically by PropertyMulti (push, newLine, updateLine, etc.)
 *
 * @see ValueBase
 * @see PropertyMulti::push()
 * @see PropertyMulti::newLine()
 * @see PropertyMulti::fetchLine()
 */
template <typename T>
class ValueTemplate: public ValueBase
{
    public:
        /**
         * @brief Construct a new ValueTemplate object
         * @param element Pointer to parent element
         *
         * Creates a value container linked to its parent element.
         * The value itself is uninitialized until updateValue() is called.
         */
        ValueTemplate(ElementBase* element) : ValueBase(element) {}

        /**
         * @brief Destroy the ValueTemplate object
         */
        ~ValueTemplate() {}

        T value;  /*!< Stored value (public for fast grid access) */
};

}
#endif
