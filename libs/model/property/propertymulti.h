/**
 * @file propertymulti.h
 * @brief Multi-element property implementation with grid support
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines PropertyMulti, the concrete implementation of PropertyBase
 * that can contain multiple elements and optionally manage a grid structure
 * for tabular data (sequences, logs, etc.).
 */

#ifndef PROPERTYMULTI_h_
#define PROPERTYMULTI_h_

#include <propertybase.h>
#include <elementfactory.h>

namespace  OST
{

/**
 * @class PropertyMulti
 * @brief Concrete property class containing multiple typed elements with optional grid
 *
 * PropertyMulti is the main property implementation in OST. It can contain:
 * - Multiple named elements (QMap<QString, ElementBase*>)
 * - Optional grid structure for tabular data (QList of element value copies)
 * - Optional graph definitions for visualization
 * - Switch rules for boolean element coordination (radio/checkbox behavior)
 *
 * @par Basic Usage:
 * @code
 * auto prop = new PropertyMulti("parameters", "Focus Parameters",
 *                               ReadWrite, "Control", "", "010", true, false);
 * prop->addElt("iterations", new ElementInt("Iterations", "0", ""));
 * prop->addElt("steps", new ElementInt("Steps", "1", ""));
 * @endcode
 *
 * @par Grid Usage (for sequences, logs, etc.):
 * @code
 * auto sequence = new PropertyMulti("sequence", "Sequence",
 *                                   ReadWrite, "Control", "", "020", true, true);
 * sequence->addElt("exposure", new ElementFloat("Exposure", "0", ""));
 * sequence->addElt("filter", new ElementString("Filter", "1", ""));
 * sequence->addElt("count", new ElementInt("Count", "2", ""));
 *
 * // Set base element values
 * sequence->getElt("exposure")->setValue(300.0, false);
 * sequence->getElt("filter")->setValue("Ha", false);
 * sequence->getElt("count")->setValue(20, false);
 *
 * // Push to grid (copies element values)
 * sequence->push();  // Creates grid line
 * @endcode
 *
 * @par Switch Rules (for boolean coordination):
 * @code
 * auto actions = new PropertyMulti("actions", "Actions", ...);
 * actions->setRule(SwitchsRule::OneOfMany);  // Radio button behavior
 * actions->addElt("calibrate", new ElementBool(...));
 * actions->addElt("guide", new ElementBool(...));
 * actions->addElt("stop", new ElementBool(...));
 * // Setting one to true automatically sets others to false
 * @endcode
 *
 * @see PropertyBase
 * @see ElementBase
 * @see SwitchsRule
 */
class PropertyMulti: public PropertyBase
{

        Q_OBJECT

    public:
        /**
         * @brief Accept a visitor (Visitor pattern implementation)
         * @param pVisitor Pointer to visitor object
         */
        void accept(PropertyVisitor *pVisitor) override;

        /**
         * @brief Accept a visitor with data parameter
         * @param pVisitor Pointer to visitor object
         * @param data QVariantMap with operation data
         */
        void accept(PropertyVisitor *pVisitor, QVariantMap &data) override;

        /**
         * @brief Construct a new PropertyMulti object
         * @param key Unique internal identifier
         * @param label Display label for frontend
         * @param permission Frontend access permission
         * @param level1 First hierarchy level
         * @param level2 Second hierarchy level
         * @param order Sort order string
         * @param hasProfile Whether to save in profile database
         * @param hasGrid Whether property has grid capability
         */
        PropertyMulti(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                      const QString &level2,
                      const QString &order, const bool &hasProfile, const bool &hasGrid
                     );

        /**
         * @brief Destroy the PropertyMulti object
         *
         * Deletes all contained elements. Grid is automatically cleared.
         */
        ~PropertyMulti();

        /**
         * @brief Get current switch rule
         * @return Current SwitchsRule value
         *
         * @see setRule()
         * @see SwitchsRule
         */
        [[nodiscard]] SwitchsRule rule() const;

        /**
         * @brief Set switch rule for boolean coordination
         * @param rule Switch rule (OneOfMany, AtMostOne, Any)
         *
         * Switch rules define how boolean elements interact:
         * - OneOfMany: Exactly one must be true (radio buttons)
         * - AtMostOne: Zero or one can be true (optional radio)
         * - Any: No constraint (checkboxes)
         *
         * @par Example:
         * @code
         * property->setRule(SwitchsRule::OneOfMany);
         * // Setting one boolean to true auto-sets others to false
         * @endcode
         */
        void setRule(SwitchsRule rule);

        /**
         * @brief Get pointer to elements map
         * @return Pointer to QMap<QString, ElementBase*>
         *
         * @warning Direct map access - use with caution. Prefer getElt() for safety.
         */
        QMap<QString, ElementBase*>* getElts();

        /**
         * @brief Get element by key
         * @param pElement Element key name
         * @return Pointer to ElementBase, or nullptr if not found
         *
         * @par Example:
         * @code
         * auto iterations = property->getElt("iterations");
         * if (iterations && iterations->getType() == "int") {
         *     auto intElt = dynamic_cast<ElementInt*>(iterations);
         *     int val = intElt->value();
         * }
         * @endcode
         */
        ElementBase* getElt(QString pElement);

        /**
         * @brief Set element value by key
         * @param key Element key name
         * @param val New value as QVariant
         * @return true if successful, false if element not found or type mismatch
         *
         * Handles switch rules for boolean elements automatically.
         * For OneOfMany rule, setting one bool to true sets others to false.
         */
        bool setElt(QString key, QVariant val);

        /**
         * @brief Add element to property
         * @param key Element key name (must be unique)
         * @param pElt Pointer to ElementBase-derived object
         *
         * Property takes ownership of the element pointer.
         * Connects element signals to property slots for event propagation.
         *
         * @warning Do not delete element manually after adding to property
         */
        void addElt(QString key, ElementBase* pElt);

        /**
         * @brief Delete element from property
         * @param key Element key name
         *
         * Removes element from map and deletes the object.
         */
        void deleteElt(QString key);

        // ===== Grid Management Methods =====

        /**
         * @brief Push current element values to grid as new line
         *
         * Creates a copy of all element values and appends as new grid line.
         * Elements themselves are not modified - only their values are copied.
         *
         * @par Example:
         * @code
         * property->getElt("exposure")->setValue(300.0, false);
         * property->getElt("filter")->setValue("Ha", false);
         * property->push();  // Grid now has one line with exposure=300, filter="Ha"
         * @endcode
         *
         * @see newLine()
         * @see clearGrid()
         */
        void push();

        /**
         * @brief Create new grid line from values map
         * @param pValues QVariantMap with element keys and values
         *
         * Alternative to push() that creates line directly from map
         * without modifying base elements.
         */
        void newLine(const QVariantMap &pValues);

        /**
         * @brief Update existing grid line with values map
         * @param i Line index (0-based)
         * @param pValues QVariantMap with element keys and values to update
         * @return true if successful, false if index out of range
         */
        bool updateLine(const int i, const QVariantMap &pValues);

        /**
         * @brief Delete grid line at index
         * @param i Line index (0-based)
         * @return true if successful, false if index out of range
         */
        bool deleteLine(const int i);

        /**
         * @brief Update grid line from current element values
         * @param i Line index (0-based)
         * @return true if successful, false if index out of range
         *
         * Overwrites grid line with current element values.
         */
        bool updateLine(const int i);

        /**
         * @brief Clear all grid lines
         *
         * Removes all lines from grid. Element values are not affected.
         */
        void clearGrid();

        /**
         * @brief Get list of grid column headers (element keys)
         * @return QList of element key names in order
         */
        QList<QString> getGridHeaders();

        /**
         * @brief Get complete grid data structure
         * @return QList of lines, each line is QMap<QString, ValueBase*>
         *
         * @warning Each ValueBase* points to value copy, not the base element
         */
        QList<QMap<QString, ValueBase*>> getGrid();

        /**
         * @brief Check if base elements are shown in frontend
         * @return true if elements visible, false if only grid shown
         */
        bool getShowElts();

        /**
         * @brief Set base elements visibility
         * @param b true to show elements, false to hide
         *
         * When false, frontend displays only grid, not individual elements.
         * Useful for properties that are purely tabular (sequences, logs).
         */
        void setShowElts(bool b);

        /**
         * @brief Check if property has grid capability
         * @return true if grid enabled, false otherwise
         */
        bool hasGrid();

        /**
         * @brief Enable or disable grid capability
         * @param b true to enable grid, false to disable
         */
        void setHasGrid(bool b);

        /**
         * @brief Check if grid is shown in frontend
         * @return true if grid visible, false otherwise
         */
        bool getShowGrid();

        /**
         * @brief Set grid visibility in frontend
         * @param b true to show grid, false to hide
         */
        void setShowGrid(bool b);

        /**
         * @brief Get maximum number of grid lines
         * @return Current grid limit
         *
         * Prevents unbounded memory growth for logs.
         * Default is 5000 lines.
         */
        int getGridLimit();

        /**
         * @brief Set maximum number of grid lines
         * @param limit Maximum lines (e.g., 5000)
         *
         * When limit is reached, oldest lines are discarded (FIFO).
         */
        void setGridLimit(int limit);

        // ===== Graph Configuration Methods =====

        /**
         * @brief Check if property has graph visualization
         * @return true if graph enabled, false otherwise
         */
        bool hasGraph();

        /**
         * @brief Enable or disable graph visualization
         * @param b true to enable graph, false to disable
         */
        void setHasGraph(bool b);

        /**
         * @brief Get graph definitions
         * @return GraphDefs structure with type and parameters
         *
         * @see setGraphDefs()
         * @see GraphDefs
         */
        GraphDefs getGraphDefs(void);

        /**
         * @brief Set graph definitions
         * @param defs GraphDefs structure with type and parameters
         *
         * @par Example:
         * @code
         * GraphDefs defs;
         * defs.type = GraphType::XY;
         * defs.params["X"] = "focpos";
         * defs.params["Y"] = "loopHFRavg";
         * property->setGraphDefs(defs);
         * @endcode
         */
        void setGraphDefs(GraphDefs defs);

        // ===== Grid Line Operations =====

        /**
         * @brief Swap two grid lines
         * @param l1 First line index
         * @param l2 Second line index
         * @return true if successful, false if indices invalid
         *
         * Used for reordering sequences.
         */
        bool swapLines(int l1, int l2);

        /**
         * @brief Fetch grid line values into base elements
         * @param l1 Line index to fetch
         * @return true if successful, false if index invalid
         *
         * Copies grid line values back to base elements.
         * Used for "edit line" functionality in frontend.
         */
        bool fetchLine(int l1);

        /**
         * @brief Check if auto up/down is enabled
         * @return true if auto up/down enabled
         *
         * When enabled, frontend can reorder lines with up/down buttons.
         */
        bool autoUpDown(void);

        /**
         * @brief Enable or disable auto up/down
         * @param b true to enable, false to disable
         */
        void setAutoUpDown(bool b);

        /**
         * @brief Check if auto select is enabled
         * @return true if auto select enabled
         *
         * When enabled, clicking grid line fetches it into base elements.
         */
        bool autoSelect(void);

        /**
         * @brief Enable or disable auto select
         * @param b true to enable, false to disable
         */
        void setAutoSelect(bool b);


    public slots:
        /**
         * @brief Slot called when element value changes
         * @param elt Pointer to element that changed
         *
         * Propagates element valueSet signal to property level.
         * Connected automatically when element is added via addElt().
         */
        void OnValueSet(ElementBase*);

        /**
         * @brief Slot called when element metadata changes
         * @param elt Pointer to element that changed
         *
         * Handles changes in element metadata (min/max, LOV, etc.).
         */
        void OnEltChanged(ElementBase*);

        /**
         * @brief Slot called when element list changes
         * @param elt Pointer to element whose list changed
         *
         * Handles dynamic list updates (e.g., LOV modifications).
         */
        void OnListChanged(ElementBase*);

        /**
         * @brief Slot called when LOV changes
         * @param elt Pointer to element whose LOV changed
         *
         * Propagates LOV change events from elements.
         */
        void OnLovChanged(ElementBase*);

        /**
         * @brief Slot called when element sends message
         * @param l Message level (Info, Warn, Err)
         * @param m Message text
         *
         * Propagates messages from elements to property level.
         */
        void OnMessage(MsgLevel l, QString m);

    private:
        // Grid configuration
        bool mHasGrid = false;          /*!< Whether grid functionality is enabled */
        bool mShowGrid = false;         /*!< Whether grid is visible in frontend */
        bool mShowElts = true;          /*!< Whether base elements are visible in frontend */
        int mGridLimit = 5000;          /*!< Maximum number of grid lines (FIFO when exceeded) */

        // Graph configuration
        bool mHasGraph = false;         /*!< Whether graph visualization is enabled */
        GraphDefs mGraphDefs;           /*!< Graph type and parameters */

        // Boolean coordination
        SwitchsRule mRule = SwitchsRule::Any;  /*!< Rule for boolean element coordination */

        // Data storage
        QMap<QString, ElementBase*> mElts;     /*!< Map of element key → element pointer */
        QList<QMap<QString, ValueBase*>> mGrid;  /*!< Grid lines (each line is map of key → value copy) */

        // Grid interaction flags
        bool mAutoUpDown = true;        /*!< Enable automatic up/down line reordering */
        bool mAutoSelect = true;        /*!< Enable automatic line fetch on click */


};

}
#endif

