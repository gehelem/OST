/**
 * @file propertybase.h
 * @brief Base class for OST properties with metadata and state management
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines the base class for all properties in the OST system.
 * Properties are containers for elements and provide metadata like permissions,
 * hierarchy levels, and runtime state.
 */

#ifndef PROPERTYBASE_h_
#define PROPERTYBASE_h_

#include <propertyvisitor.h>
#include <elementbase.h>

namespace  OST
{

class PropertyVisitor;

/**
 * @enum Permission
 * @brief Frontend access permission for properties
 *
 * Defines whether the frontend can read, write, or both for a property.
 * This permission is enforced at the frontend level only.
 */
typedef enum
{
    ReadOnly = 0,  /*!< Property is read-only for frontend - values can only be displayed */
    WriteOnly,     /*!< Property is write-only for frontend - rare use case for sensitive data */
    ReadWrite      /*!< Property is read-write for frontend - normal interactive mode */
} Permission;

/**
 * @brief Converts integer to property permission enum
 * @param val Integer value (0, 1, or 2)
 * @return Corresponding Permission enum value
 *
 * @note Invalid values default to ReadOnly and emit a warning
 *
 * @par Conversion table:
 * - 0 → ReadOnly
 * - 1 → WriteOnly
 * - 2 → ReadWrite
 */
inline Permission IntToPermission(int val )
{
    if (val == 0) return ReadOnly;
    if (val == 1) return WriteOnly;
    if (val == 2) return ReadWrite;
    qDebug() << "Cant convert " << val << " to OST::Permission (0-2) - defaults to readonly";
    return ReadOnly;
}


/**
 * @class PropertyBase
 * @brief Abstract base class for all OST properties
 *
 * PropertyBase provides the common interface and implementation for all property types.
 * It manages metadata (key, label, permissions, hierarchy), runtime state (Idle/Ok/Busy/Error),
 * and UI hints (icons, badges).
 *
 * Properties are organized hierarchically using level1 (device/category) and level2 (group)
 * fields, similar to INDI protocol structure.
 *
 * @par Hierarchy Example:
 * @code
 * PropertyBase("parameters",       // key
 *              "Focus Parameters",  // label
 *              ReadWrite,          // permission
 *              "Control",          // level1 (tab in UI)
 *              "Main",             // level2 (section in tab)
 *              "Control010",       // order (alphabetic sort)
 *              true)               // hasProfile (save to DB)
 * @endcode
 *
 * @par Visitor Pattern:
 * PropertyBase uses the Visitor pattern for operations like JSON serialization,
 * text dumping, and updates from frontend. Derived classes must implement accept().
 *
 * @see PropertyMulti
 * @see ElementBase
 */
class PropertyBase: public QObject
{

        Q_OBJECT

    public:
        /**
         * @brief Accept a visitor for the Visitor pattern (const version)
         * @param pVisitor Pointer to the visitor object
         *
         * Pure virtual method that must be implemented by derived classes.
         * Used for operations like JSON serialization (PropertyJsonDumper),
         * text dumping (PropertyTextDumper), etc.
         */
        virtual void accept(PropertyVisitor* pVisitor) = 0;

        /**
         * @brief Accept a visitor with data parameter
         * @param pVisitor Pointer to the visitor object
         * @param data QVariantMap containing data for the operation
         *
         * Overloaded accept method that allows passing data to the visitor.
         * Used for operations that need external data, like updates from frontend.
         */
        virtual void accept(PropertyVisitor* pVisitor, QVariantMap &data) = 0;

        /**
         * @brief Construct a new PropertyBase object
         * @param key Internal unique identifier (e.g., "parameters")
         * @param label Display label for frontend (e.g., "Focus Parameters")
         * @param permission Access permission (ReadOnly, WriteOnly, ReadWrite)
         * @param level1 First hierarchy level - device/category (e.g., "Control")
         * @param level2 Second hierarchy level - group (e.g., "Main")
         * @param order Sort order string (e.g., "Control010")
         * @param hasProfile Whether to save values in profile database
         *
         * @note All parameters except hasProfile are immutable after construction
         */
        PropertyBase(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                     const QString &level2,
                     const QString &order, const bool &hasProfile
                    );

        /**
         * @brief Destroy the PropertyBase object
         */
        ~PropertyBase();

        /**
         * @brief Internal property's name
         * @return returns property's name
         *
         * This value cannot be modified after instanciation
         * \hidecallgraph
         */
        QString key();
        /**
         * @brief Property's label
         * @return value of label
         *
         * This value cannot be modified after instanciation
         * \hidecallgraph
         */
        QString label();
        /**
         * @brief Property's permission
         * @return value of permission
         *
         * This value cannot be modified after instanciation
         * \hidecallgraph
         */
        Permission permission();
        /**
         * @brief Property's hierachy first level
         * @return value of level 1
         *
         * This value cannot be modified after instanciation
         *
         * It equivalent to indi "device" level
         * \hidecallgraph
         */
        QString level1();
        /**
         * @brief Property's hierachy second level
         * @return value of level 2
         *
         * This value cannot be modified after instanciation
         *
         * It equivalent to indi "group" level
         * \hidecallgraph
         */
        QString level2();
        /**
         * @brief Property's display order
         * @return order value
         *
         * This value cannot be modified after instanciation.
         *
         * This value sets display order value within same level1 / level2 values
         * \hidecallgraph
         */
        QString order();
        /**
         * @brief should values be saved in profiles ?
         * @return true = yes, false = no
         *
         * This value cannot be modified after instanciation.
         * \hidecallgraph
         */
        bool hasProfile();

        /**
         * @brief Get current runtime state
         * @return Current State enum value (Idle, Ok, Busy, Error)
         *
         * State represents the runtime status of operations on this property.
         * Frontend typically displays different colors based on state.
         *
         * @see setState()
         */
        State state();

        /**
         * @brief Set runtime state
         * @param state New State value (Idle, Ok, Busy, Error)
         *
         * Emits stateChanged() signal to notify observers.
         *
         * @par Usage Example:
         * @code
         * property->setState(OST::Busy);  // Start operation
         * // ... do work ...
         * property->setState(OST::Ok);    // Success
         * @endcode
         */
        void setState(State state);

        /**
         * @brief Check if property is enabled
         * @return true if enabled, false if disabled
         *
         * Disabled properties cannot be modified from frontend and are typically
         * displayed as grayed out or hidden.
         */
        bool isEnabled();

        /**
         * @brief Enable the property
         *
         * Allows frontend to interact with the property.
         * Emits eltChanged() signal.
         */
        void enable(void);

        /**
         * @brief Disable the property
         *
         * Prevents frontend from modifying the property.
         * Useful during operations that require property to be read-only temporarily.
         * Emits eltChanged() signal.
         */
        void disable(void);

        /**
         * @brief Get badge status
         * @return true if badge is visible, false otherwise
         *
         * Badge is a visual indicator (e.g., notification dot) on the property
         * in frontend UI. Used to draw attention (e.g., calibration required).
         */
        bool getBadge();

        /**
         * @brief Set badge visibility
         * @param b true to show badge, false to hide
         *
         * @par Usage Example:
         * @code
         * property->setBadge(true);  // Show notification badge
         * @endcode
         */
        void setBadge(bool b);

        /**
         * @brief Send informational message
         * @param m Message text
         *
         * Emits sendMessage() signal with Info level.
         * Messages are typically logged and displayed in frontend message area.
         */
        void sendInfo(QString m);

        /**
         * @brief Send warning message
         * @param m Message text
         *
         * Emits sendMessage() signal with Warn level.
         * Warnings indicate potential issues but don't stop execution.
         */
        void sendWarning(QString m);

        /**
         * @brief Send error message
         * @param m Message text
         *
         * Emits sendMessage() signal with Err level.
         * Errors indicate failures that may require user intervention.
         */
        void sendError(QString m);

        /**
         * @brief Get first prefix icon identifier
         * @return Icon identifier string (e.g., "fa-camera")
         *
         * PreIcon1 is displayed before the property label in frontend.
         */
        QString getPreIcon1(void);

        /**
         * @brief Set first prefix icon
         * @param s Icon identifier (e.g., FontAwesome icon name)
         */
        void setPreIcon1(QString s);

        /**
         * @brief Get second prefix icon identifier
         * @return Icon identifier string
         */
        QString getPreIcon2(void);

        /**
         * @brief Set second prefix icon
         * @param s Icon identifier
         */
        void setPreIcon2(QString s);

        /**
         * @brief Get first postfix icon identifier
         * @return Icon identifier string
         *
         * PostIcon1 is displayed after the property label in frontend.
         */
        QString getPostIcon1(void);

        /**
         * @brief Set first postfix icon
         * @param s Icon identifier
         */
        void setPostIcon1(QString s);

        /**
         * @brief Get second postfix icon identifier
         * @return Icon identifier string
         */
        QString getPostIcon2(void);

        /**
         * @brief Set second postfix icon
         * @param s Icon identifier
         */
        void setPostIcon2(QString s);

        /**
         * @brief Get free-form value field
         * @return Free value string
         *
         * FreeValue is a general-purpose string field for custom data.
         * Its interpretation is left to frontend implementation.
         */
        QString getFreeValue(void);

        /**
         * @brief Set free-form value field
         * @param s Free value string
         */
        void setFreeValue(QString s);

    signals:
        /**
         * @brief Signal emitted when state changes
         * @param state New State value
         *
         * Connect to this signal to react to state changes (Idle/Ok/Busy/Error).
         */
        void stateChanged(OST::State);

        /**
         * @brief Signal emitted when property metadata changes
         * @param prop Pointer to the property that changed
         *
         * Emitted when non-value attributes change (e.g., enabled/disabled,
         * badge, icons). Value changes emit valueSet() instead.
         */
        void eltChanged(OST::PropertyBase*);

        /**
         * @brief Signal emitted when a value in the property changes
         * @param prop Pointer to the property containing the changed value
         *
         * Propagated from child elements when their values change.
         * Used to trigger frontend updates and profile saves.
         */
        void valueSet(OST::PropertyBase*);

        /**
         * @brief Signal emitted for custom property events
         * @param eventType Type of event (e.g., "Flcreate", "Fldelete")
         * @param eventKey Key identifier for the event
         * @param prop Pointer to the property
         *
         * Generic event mechanism for property-level operations like
         * grid line creation/deletion, up/down movements, etc.
         */
        void propertyEvent(QString, QString, OST::PropertyBase*);

        /**
         * @brief Signal emitted when sending messages
         * @param level Message level (Info, Warn, Err)
         * @param message Message text
         *
         * Messages are propagated to module level and eventually to frontend.
         * Used by sendInfo(), sendWarning(), sendError() methods.
         */
        void sendMessage(MsgLevel, QString);

    private:
        // Immutable metadata (set in constructor)
        QString mKey = "";                              /*!< Unique internal identifier */
        QString mLabel = "change me";                   /*!< Display label for frontend */
        Permission mPermission = Permission::ReadOnly;  /*!< Frontend access permission */
        QString mLevel1 = "";                           /*!< First hierarchy level (device/category) */
        QString mLevel2 = "";                           /*!< Second hierarchy level (group) */
        QString mOrder = 0;                             /*!< Sort order string */
        bool mHasProfile = false;                       /*!< Whether to save in profile database */

        // Runtime state (mutable)
        State mState = State::Idle;     /*!< Current runtime state (Idle/Ok/Busy/Error) */
        bool mIsEnabled = true;          /*!< Whether property is enabled for frontend */

        // UI hints (mutable)
        bool mBadge = false;             /*!< Badge visibility (notification indicator) */
        QString mPreIcon1 = "";          /*!< First prefix icon identifier */
        QString mPreIcon2 = "";          /*!< Second prefix icon identifier */
        QString mPostIcon1 = "";         /*!< First postfix icon identifier */
        QString mPostIcon2 = "";         /*!< Second postfix icon identifier */
        QString mFreeValue = "";         /*!< Free-form custom data field */



};

}
#endif

