/**
 * @file elementprg.h
 * @brief Progress/activity indicator element implementation
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementPrg (progress bar/spinner element) and
 * ValuePrg (progress data container for grid storage).
 */

#ifndef ELEMENTPRG_h
#define ELEMENTPRG_h

#include <elementtemplate.h>

namespace  OST
{

/**
 * @class ElementPrg
 * @brief Progress and activity indicator element
 *
 * ElementPrg is a specialized element for displaying operation progress and activity.
 * It stores PrgData structures containing progress value (0-100) and dynamic label text.
 *
 * Supports two display modes via PrgType:
 * - spinner: Indeterminate spinner (activity indicator, no percentage)
 * - progressbar: Determinate progress bar (0-100% with value display)
 *
 * The dynamic label allows updating status text during operations without
 * changing the element label itself.
 *
 * Common uses:
 * - Long-running operation progress (exposure, focus, sequence)
 * - File download/upload progress
 * - Processing activity indicators
 * - Multi-step operation status
 *
 * @par Progress Bar Usage:
 * @code
 * auto exposureProgress = new ElementPrg("Exposure", "10", "Exposure progress");
 * exposureProgress->setPrgType(progressbar);
 * exposureProgress->setPrgValue(0.0, false);      // 0% start
 * exposureProgress->setDynLabel("Starting exposure", false);
 * property->addElt("expprogress", exposureProgress);
 *
 * // During exposure (e.g., 30% complete):
 * exposureProgress->setPrgValue(30.0, true);      // Update to 30%
 * exposureProgress->setDynLabel("Exposing (90s remaining)", true);
 *
 * // On completion:
 * exposureProgress->setPrgValue(100.0, true);     // 100%
 * exposureProgress->setDynLabel("Exposure complete", true);
 * @endcode
 *
 * @par Spinner Usage (indeterminate):
 * @code
 * auto platesolve = new ElementPrg("Platesolve", "20", "Platesolving");
 * platesolve->setPrgType(spinner);                // Indeterminate spinner
 * platesolve->setDynLabel("Analyzing stars...", false);
 * property->addElt("platesolve", platesolve);
 *
 * // Update status:
 * platesolve->setDynLabel("Matching catalog...", true);
 * platesolve->setDynLabel("Solution found!", true);
 * @endcode
 *
 * @par Focus Run Progress:
 * @code
 * auto focusProgress = new ElementPrg("Focus", "30", "Focus run progress");
 * focusProgress->setPrgType(progressbar);
 * focusProgress->setPrgValue(0.0, false);
 *
 * // Step 5 of 20:
 * double percent = (5.0 / 20.0) * 100.0;  // 25%
 * focusProgress->setPrgValue(percent, true);
 * focusProgress->setDynLabel("Step 5/20", true);
 * @endcode
 *
 * @note PrgData and PrgType are defined in common.h
 * @note Progress values are 0.0-100.0 (percentage)
 * @note Frontend renders as HTML5 progress bar or CSS spinner
 *
 * @see PrgData
 * @see PrgType
 * @see ElementTemplateNotNumeric
 */
class ElementPrg: public ElementTemplateNotNumeric<PrgData>
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
         * @brief Construct a new ElementPrg object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementPrg(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementPrg object
         */
        ~ElementPrg();

        /**
         * @brief Get element type as string
         * @return "prg"
         */
        QString getType() override;

        /**
         * @brief Set progress value (percentage)
         * @param v Progress value (0.0 to 100.0)
         * @param emitEvent Whether to emit valueSet signal
         *
         * Updates the progress percentage displayed in the progress bar.
         * Only relevant when prgType is progressbar (ignored for spinner).
         *
         * @par Example:
         * @code
         * element->setPrgValue(50.0, true);   // Set to 50%
         * element->setPrgValue(100.0, true);  // Complete
         * @endcode
         */
        void setPrgValue(const double &v, const bool &emitEvent);

        /**
         * @brief Set dynamic label text
         * @param s New label text
         * @param emitEvent Whether to emit valueSet signal
         *
         * Updates the dynamic status text shown alongside the progress indicator.
         * This allows status updates without changing the element's main label.
         *
         * @par Example:
         * @code
         * element->setDynLabel("Processing frame 5/20", true);
         * element->setDynLabel("Analyzing results", true);
         * element->setDynLabel("Complete!", true);
         * @endcode
         */
        void setDynLabel(const QString &s, const bool &emitEvent);

        /**
         * @brief Get current dynamic label
         * @return Dynamic label string
         */
        QString dynLabel();

        /**
         * @brief Get progress indicator type
         * @return Current PrgType (spinner or progressbar)
         */
        PrgType prgType();

        /**
         * @brief Set progress indicator type
         * @param t PrgType (spinner for indeterminate, progressbar for percentage)
         *
         * Controls whether the element displays as:
         * - spinner: Indeterminate activity indicator (no percentage)
         * - progressbar: Determinate progress bar with 0-100% value
         *
         * @par Example:
         * @code
         * element->setPrgType(progressbar);  // Show percentage bar
         * element->setPrgType(spinner);      // Show spinning activity indicator
         * @endcode
         */
        void setPrgType(PrgType t);

    private:
        PrgType mType = spinner;      /*!< Progress indicator type (spinner or progressbar) */
        QString mDynLabel = "";       /*!< Dynamic status label text */

};

/**
 * @class ValuePrg
 * @brief Progress data container for grid storage
 *
 * ValuePrg is the grid value copy for ElementPrg. It stores PrgData structures
 * and provides methods to synchronize with its parent ElementPrg.
 *
 * Useful for storing progress history in grids (e.g., operation log with
 * completion percentages per step).
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valuePrg = new ValuePrg(progressElement);
 * valuePrg->updateValue();  // Copy element->value() to valuePrg->value
 * gridLine["progress"] = valuePrg;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * valuePrg->updateElement(true);  // Copy valuePrg->value to element->value()
 * @endcode
 *
 * @see ValueTemplate
 * @see ElementPrg
 * @see PrgData
 * @see PropertyMulti::push()
 */
class ValuePrg: public ValueTemplate<PrgData>
{
        Q_OBJECT

    public:

        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValuePrg object
         * @param element Pointer to parent ElementPrg
         */
        ValuePrg(ElementBase* element): ValueTemplate<PrgData>(element) {}

        /**
         * @brief Destroy the ValuePrg object
         */
        ~ValuePrg() {}

        /**
         * @brief Update value from parent element
         *
         * Copies the parent element's current progress data into this value object.
         */
        void updateValue() override;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this progress data back into the parent element.
         */
        void updateElement(const bool &emitEvent) override;

};

}
#endif
