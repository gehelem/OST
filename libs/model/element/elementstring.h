/**
 * @file elementstring.h
 * @brief String element implementation with LOV support
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementString (text element with LOV support for dropdowns/radios)
 * and ValueString (string value container for grid storage).
 */

#ifndef ELEMENTSTRING_h
#define ELEMENTSTRING_h
#include <elementtemplate.h>
#include <lovstring.h>
namespace  OST
{

/**
 * @class ElementString
 * @brief String element with LOV support
 *
 * ElementString is a concrete element type for text/string values. It inherits from
 * ElementTemplateNotNumeric<QString> and supports LOV (List of Values) for dropdown/radio
 * selection from predefined string options.
 *
 * Common uses:
 * - Device/module selection (e.g., camera names, filter wheel positions)
 * - Configuration options (e.g., file formats, modes)
 * - Path/filename inputs
 * - Text fields with predefined choices
 *
 * @par Basic Usage (free-form text):
 * @code
 * auto targetName = new ElementString("Target", "10", "Astronomical object name");
 * targetName->setValue("M31 Andromeda", false);
 * property->addElt("target", targetName);
 * @endcode
 *
 * @par LOV Usage (dropdown selection):
 * @code
 * auto filterPos = new ElementString("Filter", "20", "Current filter position");
 * filterPos->lovAdd("Ha", "Hydrogen Alpha 6nm");
 * filterPos->lovAdd("OIII", "Oxygen III 6nm");
 * filterPos->lovAdd("SII", "Sulfur II 6nm");
 * filterPos->lovAdd("L", "Luminance");
 * filterPos->setValue("Ha", false);
 * property->addElt("filter", filterPos);
 * // Frontend displays dropdown: [Hydrogen Alpha 6nm, Oxygen III 6nm, ...]
 * @endcode
 *
 * @par Device Selection:
 * @code
 * auto cameraName = new ElementString("Camera", "30", "Connected camera device");
 * cameraName->lovAdd("ZWO ASI294MM Pro", "ZWO ASI294MM Pro");
 * cameraName->lovAdd("QHY268M", "QHY268M");
 * cameraName->setValue("ZWO ASI294MM Pro", false);
 * @endcode
 *
 * @note ElementString uses QString for values, supporting Unicode and multi-byte characters
 *
 * @see ElementTemplateNotNumeric
 * @see ElementInt
 * @see LovString
 */
class ElementString: public ElementTemplateNotNumeric<QString>
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
         * @brief Construct a new ElementString object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementString(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementString object
         */
        ~ElementString();

        /**
         * @brief Get element type as string
         * @return "string"
         */
        QString getType() override;

        /**
         * @brief Get List of Values (LOV)
         * @return QMap<QString, QString> mapping values to display labels
         *
         * Returns the current LOV for dropdown/radio display.
         * Empty map means no LOV (free-form text input).
         */
        QMap<QString, QString> getLov();

        /**
         * @brief Add value to LOV
         * @param val String value (key)
         * @param label Display label for this value
         * @return true if added, false if value already exists
         */
        bool lovAdd(QString val, QString label);

        /**
         * @brief Update LOV value label
         * @param val String value to update
         * @param label New display label
         * @return true if updated, false if value not found
         */
        bool lovUpdate(QString  val, QString label);

        /**
         * @brief Delete value from LOV
         * @param val String value to delete
         * @return true if deleted, false if value not found
         */
        bool lovDel(QString  val);

        /**
         * @brief Clear all LOV entries
         * @return true if cleared successfully
         */
        bool lovClear();

    private:
        LovString mLov;  /*!< List of Values for dropdown/radio selection */

};

/**
 * @class ValueString
 * @brief String value container for grid storage
 *
 * ValueString is the grid value copy for ElementString. It stores a QString value
 * and provides methods to synchronize with its parent ElementString.
 *
 * @see ValueTemplate
 * @see ElementString
 * @see PropertyMulti::push()
 */
class ValueString: public ValueTemplate<QString>
{
        Q_OBJECT

    public:

        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValueString object
         * @param element Pointer to parent ElementString
         */
        ValueString(ElementBase* element): ValueTemplate<QString>(element) {}

        /**
         * @brief Destroy the ValueString object
         */
        ~ValueString() {}

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
