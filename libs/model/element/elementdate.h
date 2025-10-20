/**
 * @file elementdate.h
 * @brief Date element implementation for calendar date selection
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementDate (QDate-based calendar date picker) and
 * ValueDate (date value container for grid storage).
 */

#ifndef ELEMENTDATE_h
#define ELEMENTDATE_h

#include <elementtemplate.h>

namespace  OST
{

/**
 * @class ElementDate
 * @brief Date element for calendar date selection
 *
 * ElementDate is a concrete element type for date values using Qt's QDate class.
 * It inherits from ElementTemplateNotNumeric<QDate> and is typically displayed
 * as a calendar date picker in the frontend UI.
 *
 * Common uses:
 * - Observation date selection
 * - Dark/flat frame expiration dates
 * - Session start/end dates
 * - Equipment maintenance schedules
 * - Data log timestamps (date portion)
 *
 * @par Basic Usage:
 * @code
 * auto observationDate = new ElementDate("Date", "10", "Observation date");
 * observationDate->setValue(QDate::currentDate(), false);  // Set to today
 * property->addElt("obsdate", observationDate);
 * @endcode
 *
 * @par Session Planning:
 * @code
 * auto sessionStart = new ElementDate("Start Date", "20", "Session start date");
 * sessionStart->setValue(QDate(2025, 10, 20), false);  // Set specific date
 * property->addElt("startdate", sessionStart);
 * @endcode
 *
 * @par Dark Frame Expiry:
 * @code
 * auto darkExpiry = new ElementDate("Expiry", "30", "Dark library expiration date");
 * QDate expiry = QDate::currentDate().addMonths(3);  // 3 months from now
 * darkExpiry->setValue(expiry, false);
 * @endcode
 *
 * @note QDate stores dates in ISO 8601 format internally (YYYY-MM-DD)
 * @note Frontend typically displays dates in user's locale format
 *
 * @see ElementTime
 * @see ElementTemplateNotNumeric
 * @see QDate (Qt documentation)
 */
class ElementDate: public ElementTemplateNotNumeric<QDate>
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
         * @brief Construct a new ElementDate object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementDate(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementDate object
         */
        ~ElementDate();

        /**
         * @brief Get element type as string
         * @return "date"
         */
        QString getType() override;

    private:

};

/**
 * @class ValueDate
 * @brief Date value container for grid storage
 *
 * ValueDate is the grid value copy for ElementDate. It stores a QDate value
 * and provides methods to synchronize with its parent ElementDate.
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueDate = new ValueDate(dateElement);
 * valueDate->updateValue();  // Copy element->value() to valueDate->value
 * gridLine["obsdate"] = valueDate;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * valueDate->updateElement(true);  // Copy valueDate->value to element->value()
 * @endcode
 *
 * @see ValueTemplate
 * @see ElementDate
 * @see PropertyMulti::push()
 */
class ValueDate: public ValueTemplate<QDate>
{
        Q_OBJECT

    public:

        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValueDate object
         * @param element Pointer to parent ElementDate
         */
        ValueDate(ElementBase* element): ValueTemplate<QDate>(element) {}

        /**
         * @brief Destroy the ValueDate object
         */
        ~ValueDate() {}

        /**
         * @brief Update value from parent element
         *
         * Copies the parent element's current date value into this value object.
         */
        void updateValue() override;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this date value back into the parent element.
         */
        void updateElement(const bool &emitEvent) override;

};

}
#endif
