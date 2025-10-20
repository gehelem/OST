/**
 * @file elementtime.h
 * @brief Time element implementation for time-of-day selection
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementTime (QTime-based time picker with optional milliseconds)
 * and ValueTime (time value container for grid storage).
 */

#ifndef ELEMENTTIME_h
#define ELEMENTTIME_h

#include <elementtemplate.h>

namespace  OST
{

/**
 * @class ElementTime
 * @brief Time element for time-of-day selection
 *
 * ElementTime is a concrete element type for time values using Qt's QTime class.
 * It inherits from ElementTemplateNotNumeric<QTime> and is typically displayed
 * as a time picker in the frontend UI.
 *
 * Supports optional millisecond precision via the mUseMs flag:
 * - false (default): HH:MM:SS format (e.g., "23:45:30")
 * - true: HH:MM:SS.zzz format with milliseconds (e.g., "23:45:30.123")
 *
 * Common uses:
 * - Observation start/end times
 * - Session scheduling
 * - Equipment operation times
 * - Data log timestamps (time portion)
 * - Exposure timing (when millisecond precision needed)
 *
 * @par Basic Usage (seconds precision):
 * @code
 * auto startTime = new ElementTime("Start Time", "10", "Session start time");
 * startTime->setValue(QTime(22, 30, 0), false);  // 10:30 PM
 * property->addElt("starttime", startTime);
 * @endcode
 *
 * @par Current Time:
 * @code
 * auto currentTime = new ElementTime("Current", "20", "Current time");
 * currentTime->setValue(QTime::currentTime(), false);
 * @endcode
 *
 * @par High Precision (with milliseconds):
 * @code
 * auto timestamp = new ElementTime("Timestamp", "30", "Precise event timestamp");
 * timestamp->setUseMs(true);  // Enable millisecond display
 * timestamp->setValue(QTime::currentTime(), false);
 * // Frontend displays: 23:45:30.123
 * @endcode
 *
 * @par Duration/Elapsed Time:
 * @code
 * auto elapsed = new ElementTime("Elapsed", "40", "Elapsed session time");
 * QTime duration(2, 15, 30);  // 2 hours, 15 minutes, 30 seconds
 * elapsed->setValue(duration, false);
 * @endcode
 *
 * @note QTime stores time in 24-hour format internally
 * @note Frontend may display in 12/24 hour format based on locale
 * @note For date+time together, use both ElementDate and ElementTime
 *
 * @see ElementDate
 * @see ElementTemplateNotNumeric
 * @see QTime (Qt documentation)
 */
class ElementTime: public ElementTemplateNotNumeric<QTime>
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
         * @brief Construct a new ElementTime object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementTime(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementTime object
         */
        ~ElementTime();

        /**
         * @brief Get element type as string
         * @return "time"
         */
        QString getType() override;

        /**
         * @brief Enable or disable millisecond display
         * @param b true to show milliseconds, false for seconds only
         *
         * Controls the precision of time display in frontend:
         * - false (default): HH:MM:SS (e.g., "23:45:30")
         * - true: HH:MM:SS.zzz (e.g., "23:45:30.123")
         *
         * @par Example:
         * @code
         * element->setUseMs(true);   // Enable millisecond precision
         * element->setValue(QTime::currentTime(), true);
         * @endcode
         */
        void setUseMs(bool b);

        /**
         * @brief Check if millisecond display is enabled
         * @return true if milliseconds shown, false if seconds only
         */
        bool getUseMs(void);

    private:
        bool mUseMs = false;  /*!< Whether to display milliseconds (false=HH:MM:SS, true=HH:MM:SS.zzz) */

};

/**
 * @class ValueTime
 * @brief Time value container for grid storage
 *
 * ValueTime is the grid value copy for ElementTime. It stores a QTime value
 * and provides methods to synchronize with its parent ElementTime.
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueTime = new ValueTime(timeElement);
 * valueTime->updateValue();  // Copy element->value() to valueTime->value
 * gridLine["starttime"] = valueTime;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * valueTime->updateElement(true);  // Copy valueTime->value to element->value()
 * @endcode
 *
 * @see ValueTemplate
 * @see ElementTime
 * @see PropertyMulti::push()
 */
class ValueTime: public ValueTemplate<QTime>
{
        Q_OBJECT

    public:

        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValueTime object
         * @param element Pointer to parent ElementTime
         */
        ValueTime(ElementBase* element): ValueTemplate<QTime>(element) {}

        /**
         * @brief Destroy the ValueTime object
         */
        ~ValueTime() {}

        /**
         * @brief Update value from parent element
         *
         * Copies the parent element's current time value into this value object.
         */
        void updateValue() override;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this time value back into the parent element.
         */
        void updateElement(const bool &emitEvent) override;

};

}
#endif
