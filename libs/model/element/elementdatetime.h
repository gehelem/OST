/**
 * @file elementdatetime.h
 * @brief Date time element implementation for datetime selection
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementDateTime
 */

#ifndef ELEMENTDATETIME_h
#define ELEMENTDATETIME_h

#include <elementtemplate.h>

namespace  OST
{


class ElementDateTime: public ElementTemplateNotNumeric<QDateTime>
{

        Q_OBJECT

    public:
        /**
         * @brief Accept a visitor with action and data parameters
         * @param pVisitor Pointer to visitor object
         * @param data QVariantMap with operation data
         * @param signalType Signal dispatch
         *
         * Used for grid operations and custom actions.
         */
        void accept(ElementVisitor *pVisitor, QVariantMap &data, bool &emitEvent) override;

        /**
         * @brief Construct a new ElementDate object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementDateTime(const QString &key, const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementDate object
         */
        ~ElementDateTime();

        /**
         * @brief Get element type as string
         * @return "date"
         */
        QString getType() override;

    private:

};

class ValueDateTime: public ValueTemplate<QDateTime>
{
        Q_OBJECT

    public:

        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValueDateTime object
         * @param element Pointer to parent ElementDate
         */
        ValueDateTime(ElementBase* element): ValueTemplate<QDateTime>(element) {}

        /**
         * @brief Destroy the ValueDateTime object
         */
        ~ValueDateTime() {}

        /**
         * @brief Update value from parent element
         *
         * Copies the parent element's current datetime value into this value object.
         */
        void updateValue() override;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this datetime value back into the parent element.
         */
        void updateElement(const bool  &emitEvent) override;

};

}
#endif
