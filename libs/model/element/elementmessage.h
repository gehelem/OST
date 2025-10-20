/**
 * @file elementmessage.h
 * @brief Message log element implementation
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementMessage (message/log display element) and
 * ValueMessage (message data container for grid storage).
 */

#ifndef ELEMENTMESSAGE_h
#define ELEMENTMESSAGE_h

#include <elementtemplate.h>

namespace  OST
{

/**
 * @class ElementMessage
 * @brief Message and log display element
 *
 * ElementMessage is a specialized element for displaying timestamped log messages
 * with severity levels. It stores MsgData structures containing:
 * - Message text
 * - MsgLevel (Info, Warn, Err)
 * - Timestamp (QDateTime)
 *
 * Frontend typically displays messages with color coding based on level:
 * - Info: Normal/white text (informational messages)
 * - Warn: Yellow/orange text (warnings, non-critical issues)
 * - Err: Red text (errors, critical problems)
 *
 * Common uses:
 * - Operation status messages
 * - Error and warning logs
 * - Module activity logs
 * - Debug output
 * - System event notifications
 *
 * ElementMessage is often used in properties with grids to maintain
 * a scrollable message history.
 *
 * @par Basic Usage (single message display):
 * @code
 * auto statusMsg = new ElementMessage("Status", "10", "Current status message");
 *
 * MsgData msgData;
 * msgData.message = "Focus run completed successfully";
 * msgData.level = Info;
 * msgData.ts = QDateTime::currentDateTime();
 * statusMsg->setValue(msgData, false);
 *
 * property->addElt("status", statusMsg);
 * @endcode
 *
 * @par Error Message:
 * @code
 * MsgData errorData;
 * errorData.message = "Camera connection failed";
 * errorData.level = Err;
 * errorData.ts = QDateTime::currentDateTime();
 * statusMsg->setValue(errorData, true);  // Display in red
 * @endcode
 *
 * @par Warning Message:
 * @code
 * MsgData warnData;
 * warnData.message = "Temperature above threshold";
 * warnData.level = Warn;
 * warnData.ts = QDateTime::currentDateTime();
 * statusMsg->setValue(warnData, true);  // Display in yellow
 * @endcode
 *
 * @par Grid Usage (message log/history):
 * @code
 * // Typically used with PropertyMulti grid for scrollable log
 * auto logProperty = new PropertyMulti("log", "Activity Log",
 *                                      ReadOnly, "Log", "", "010", false, true);
 * logProperty->addElt("message", new ElementMessage("Message", "10", "Log entry"));
 *
 * // Add messages to grid:
 * MsgData msg1;
 * msg1.message = "Session started";
 * msg1.level = Info;
 * msg1.ts = QDateTime::currentDateTime();
 * logProperty->getElt("message")->setValue(msg1, false);
 * logProperty->push();  // Add to grid
 *
 * MsgData msg2;
 * msg2.message = "Camera connected";
 * msg2.level = Info;
 * msg2.ts = QDateTime::currentDateTime();
 * logProperty->getElt("message")->setValue(msg2, false);
 * logProperty->push();  // Add second line
 * @endcode
 *
 * @note MsgData and MsgLevel are defined in common.h
 * @note Timestamps are stored in QDateTime for timezone-aware display
 * @note Grid typically has limit (e.g., 5000 messages) for memory management
 *
 * @see MsgData
 * @see MsgLevel
 * @see ElementTemplateNotNumeric
 * @see PropertyMulti::push()
 */
class ElementMessage: public ElementTemplateNotNumeric<MsgData>
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
         * @brief Construct a new ElementMessage object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementMessage(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementMessage object
         */
        ~ElementMessage();

        /**
         * @brief Get element type as string
         * @return "message"
         */
        QString getType() override;

        /**
         * @brief Get message text
         * @return Message string from current MsgData value
         *
         * Convenience method to extract message text from value.
         */
        QString message();

        /**
         * @brief Get message severity level
         * @return MsgLevel (Info, Warn, Err) from current MsgData value
         *
         * Convenience method to extract severity level from value.
         */
        MsgLevel level();

        /**
         * @brief Get message timestamp
         * @return QDateTime timestamp from current MsgData value
         *
         * Convenience method to extract timestamp from value.
         */
        QDateTime ts();

};

/**
 * @class ValueMessage
 * @brief Message data container for grid storage
 *
 * ValueMessage is the grid value copy for ElementMessage. It stores MsgData structures
 * and provides methods to synchronize with its parent ElementMessage.
 *
 * Primarily used for message logs/history in grids where each grid line
 * represents a timestamped log entry.
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueMsg = new ValueMessage(messageElement);
 * valueMsg->updateValue();  // Copy element->value() to valueMsg->value
 * gridLine["message"] = valueMsg;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * valueMsg->updateElement(true);  // Copy valueMsg->value to element->value()
 * @endcode
 *
 * @see ValueTemplate
 * @see ElementMessage
 * @see MsgData
 * @see PropertyMulti::push()
 */
class ValueMessage: public ValueTemplate<MsgData>
{
        Q_OBJECT

    public:

        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValueMessage object
         * @param element Pointer to parent ElementMessage
         */
        ValueMessage(ElementBase* element): ValueTemplate<MsgData>(element) {}

        /**
         * @brief Destroy the ValueMessage object
         */
        ~ValueMessage() {}

        /**
         * @brief Update value from parent element
         *
         * Copies the parent element's current message data into this value object.
         */
        void updateValue() override;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this message data back into the parent element.
         */
        void updateElement(const bool &emitEvent) override;

};
}
#endif
