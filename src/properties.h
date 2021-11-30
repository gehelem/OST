#ifndef PROPERTIES_H
#define PROPERTIES_H
#include <QtCore>
#include <indibase.h>

enum propType
{
    /*! Text element */
    PT_TEXT,
    /*! Number element */
    PT_NUM,
    /*! SWitcht element */
    PT_SWITCH,
    /*! Light element - just like indi light properties OK/Idle/busy/error */
    PT_LIGHT,
    /*! Message */
    PT_MESSAGE,
    /*! Image element */
    PT_IMAGE,
    /*! Graphic element */
    PT_GRAPH,
};

/**
 * @typedef OSState
 * @brief Switch state.
 */
typedef enum
{
OSS_OFF = 0, /*!< Switch is OFF */
OSS_ON       /*!< Switch is ON */
} OSState;

/**
 * @typedef OPState
 * @brief Property state.
 */
typedef enum
{
OPS_IDLE = 0, /*!< State is idle */
OPS_OK,       /*!< State is ok */
OPS_BUSY,     /*!< State is busy */
OPS_ALERT     /*!< State is alert */
} OPState;

/**
 * @typedef OSRule
 * @brief Switch vector rule hint.
 */
typedef enum
{
OSR_1OFMANY, /*!< Only 1 switch of many can be ON (e.g. radio buttons) */
OSR_ATMOST1, /*!< At most one switch can be ON, but all switches can be off. It is similar to ISR_1OFMANY with the exception that all switches can be off. */
OSR_NOFMANY  /*!< Any number of switches can be ON (e.g. check boxes) */
} OSRule;

/**
 * @typedef OPerm
 * @brief Permission hint, with respect to client.
 */
typedef enum
{
OP_RO, /*!< Read Only */
OP_WO, /*!< Write Only */
OP_RW  /*!< Read & Write */
} OPerm;
/**
 * @typedef OImgType
 * @brief Image item type
 */
typedef enum
{
IM_FULL, /*!< Full image */
IM_MINI, /*!< Overvieew */
IM_OVERLAY  /*!< Overlay */
} OImgType;

/**
 * @struct OText
 * @brief One text descriptor.
 */
typedef struct
{
    /** internal name */
    QString     name;
    /** Short description */
    QString     label;
    /** Allocated text string */
    QString     text;
    /** Helper info */
    QString     aux0;
    /** Helper info */
    QString     aux1;
    /** GUI order */
    int         order;

} OText;

/**
 * @struct ONumber
 * @brief One number descriptor.
 */
typedef struct
{
    /** internal name */
    QString     name;
    /** Short description */
    QString     label;
    /** GUI display format, see above */
    QString     format;
    /** Range min, ignored if min == max */
    double      min =0;
    /** Range max, ignored if min == max */
    double      max =0;
    /** Step size, ignored if step == 0 */
    double      step =0;
    /** Current value */
    double      value =0;
    /** Helper info */
    QString     aux0;
    /** Helper info */
    QString     aux1;
    /** GUI order */
    int         order;

} ONumber;

/**
 * @struct OSwitch
 * @brief One switch descriptor.
 */
typedef struct
{
    /** internal name */
    QString     name;
    /** Switch label */
    QString     label;
    /** Switch state */
    OSState     s;
    /** Helper info */
    QString     aux0;
    /** Helper info */
    QString     aux1;
    /** GUI order */
    int         order;

} OSwitch;

/**
 * @struct OLight
 * @brief One light descriptor.
 */
typedef struct
{
    /** internal name */
    QString     name;
    /** Light labels */
    QString     label;
    /** Light state */
    OPState     l;
    /** Helper info */
    QString     aux0;
    /** Helper info */
    QString     aux1;
    /** GUI order */
    int         order;

} OLight;

/**
 * @struct OImage
 * @brief One image descriptor.
 */
typedef struct
{
    /** internal name */
    QString     name;
    /** Short description */
    QString     label;
    /** Image URL */
    QString     url;
    /** Image file */
    QString     f;
    /** Helper info */
    QString     aux0;
    /** Helper info */
    QString     aux1;
    /** Image type */
    OImgType    imgtype;
    /** GUI order */
    int         order;

} OImage;

/**
 * @struct OMessage
 * @brief One message descriptor.
 */
typedef struct
{
    /** internal name */
    QString     name;
    /** Short description */
    QString     label;
    /** Message content - work in progress */
    QString     text;
    /** Helper info */
    QString     aux0;
    /** Helper info */
    QString     aux1;
    /** GUI order */
    int         order;

} OMessage;


/**
 * @struct OGraphValues
 * @brief Graph item data store
 */
typedef struct
{
    /** Value 0  */
    double v0 =0;
    /** Value 1  */
    double v1 =0;
    /** Value 2  */
    double v2 =0;
    /** Value 3  */
    double v3 =0;
    /** Value 4  */
    double v4 =0;
} OGraphValue;

/**
 * @struct OGraph
 * @brief One message descriptor.
 */
typedef struct
{
    /** internal name */
    QString     name;
    /** Short description */
    QString     label ="";
    /** Graph  Type */
    QString     gtype ="2D";
    /** Helper info */
    QString     aux0  ="";
    /** Helper info */
    QString     aux1  ="";
    /** Data 0 label */
    QString     V0label ="";
    /** Data 1 label */
    QString     V1label ="";
    /** Data 2 label */
    QString     V2label ="";
    /** Data 3 label */
    QString     V3label ="";
    /** Data 4 label */
    QString     V4label ="";
    /** Data Store */
    QVector<OGraphValue> values;
    /** GUI order */
    int         order;

} OGraph;




class Devcat : public QObject {
Q_OBJECT
public:
    Devcat( QString name, QString label,QObject* parent = nullptr)
        : QObject(parent), _name(name), _label(label){setObjectName(name);}
    virtual ~Devcat() = default;

    [[nodiscard]] inline const QString &getName() const { return _name; }
    [[nodiscard]] inline const QString &getLabel() const { return _label; }
private:
    QString _name;
    QString _label;
};


class Property : public QObject {
Q_OBJECT
public:
    Property( QString name, QString label, int permission, int state,QObject* parent = nullptr);
    virtual ~Property() = default;

    [[nodiscard]] inline const QString &getName() const { return _name; }
    [[nodiscard]] inline const QString &getLabel() const { return _label; }
    [[nodiscard]] inline int getPermission() const { return _permission; }
    [[nodiscard]] inline int getState() const { return _state; }
private:
    QString _name;
    QString _label;
    int _permission;
    int _state;
signals:
    void propertyCreated(Property *prop)    ;
    void propertyModified(Property *prop)    ;
    void propertyDeleted(Property *prop)    ;

};

#endif // PROPERTIES_H
