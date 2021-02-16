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
/**
 * @struct Prop
 * @brief One property descriptor.
 */
typedef struct
{
    /** Module owner internal name */
    QString     modulename;
    /** Property internal name */
    QString     propname;
    /** Short description */
    QString     label;
    /** Property type */
    propType    typ;
    /** GUI Category  name */
    QString     categname;
    /** GUI Group name */
    QString     groupname;
    /** GUI Permission */
    OPerm       perm;
    /** Switch behaviour */
    OSRule      rule;
    /** Current max time to change, secs */
    double      timeout;
    /** Current property state */
    OPState     state;
    /** Helper info */
    QString     aux0;
    /** Helper info */
    QString     aux1;
    /** GUI order */
    int         order;

    /** Text elements */
    QMap<QString,OText>     t;
    /** Number elements */
    QMap<QString,ONumber>   n;
    /** Switchs elements */
    QMap<QString,OSwitch>   s;
    /** Light elements */
    QMap<QString,OLight>    l;
    /** Message elements */
    QMap<QString,OMessage>  m;
    /** Image elements */
    QMap<QString,OImage>    i;
    /** Graph elements */
    QMap<QString,OGraph>    g;
} Prop;

Q_DECLARE_METATYPE(Prop);

typedef struct
{
    QString                 groupname;
    QString                 grouplabel;
    QString                 modulename;
    QString                 categname;
    int                     order;
}  Group;

typedef struct
{
    QString                 categname;
    QString                 categlabel;
    QString                 modulename;
    int                     order;
}  Categ;

typedef struct
{
    QString                 modulename;
    QString                 modulelabel;
    int                     order;
    QMap<QString,Categ>     categs;
    QMap<QString,Group>     groups;
    QMap<QString,Prop>      props;
}  Mod;

class Properties : public QObject
{
  Q_OBJECT
public:
    Properties(QObject *parent = Q_NULLPTR);
    ~Properties() = default;
    void    createModule(QString modulename,  QString modulelabel,int order);
    void    deleteModule(QString modulename);

    void    createCateg(QString modulename,QString categname,  QString categlabel,int order);
    void    deleteCateg(QString modulename,QString categname);

    void    createGroup(QString modulename,QString categname, QString groupname,  QString grouplabel,int order);
    void    deleteGroup(QString modulename,QString categname, QString groupname);

    void    createProp (QString modulename, QString propname, Prop    prop);
    void    createProp (QString modulename,QString propname,QString label,propType typ,QString categname,QString groupname,OPerm perm,OSRule rule,double timeout,OPState state,QString aux0,QString aux1,int order);
    void    deleteProp (QString modulename,QString propname);
    Prop    getProp    (QString modulename,QString propname);
    void    setProp    (QString modulename,QString propname,Prop    prop);

    void    appendElt  (QString modulename,QString propname,  QString txtname, QString text     , QString label, QString aux0,QString aux1);
    void    appendElt  (QString modulename,QString propname,  QString numname, double  num      , QString label, QString aux0,QString aux1);
    void    appendElt  (QString modulename,QString propname,  QString swtname, OSState swt      , QString label, QString aux0,QString aux1);
    void    appendElt  (QString modulename,QString propname,  QString lgtname, OPState lgt      , QString label, QString aux0,QString aux1);
    void    appendElt  (QString modulename,QString propname,  QString imgname, OImgType imt     , QString label, QString aux0,QString aux1, QString url, QString file);
    void    appendElt  (QString modulename,QString propname,  QString graname, OGraph graph     );
    void    appendGra  (QString modulename,QString propname,  QString graname, OGraphValue val  );
    void    resetGra   (QString modulename,QString propname,  QString graname);
    void    deleteElt  (QString modulename,QString propname,  QString eltname);
    void    setElt     (QString modulename,QString propname,  QString txtname, QString text);
    void    setElt     (QString modulename,QString propname,  QString numname, double  num);
    void    setElt     (QString modulename,QString propname,  QString swtname, OSState swt);
    void    setElt     (QString modulename,QString propname,  QString lgtname, OPState lgt);
    void    setElt     (QString modulename,QString propname,  QString imgname, QString url, QString file);
    void    setElt     (QString modulename,QString propname,  QString graname, OGraph  graph);
    QString getTxt     (QString modulename,QString propname,  QString txtname);
    double  getNum     (QString modulename,QString propname,  QString numname);
    OSState getSwt     (QString modulename,QString propname,  QString swtname);
    OPState getLgt     (QString modulename,QString propname,  QString lgtname);
    OImage  getImg     (QString modulename,QString propname,  QString imgname);
    OGraph  getGraph   (QString modulename,QString propname,  QString graname);
    QMap<QString,Mod> getModules(void)    {return modules;}
    Mod     getModule(QString modulename) {return modules[modulename];}


    //QJsonObject getJsonProp(QString modulename,QString propname, QString modulename);
    void    dumproperties(void);
signals:
    void    signalPropCreated (Prop prop);
    void    signalPropDeleted (Prop prop);
    void    signalvalueChanged(Prop prop);
    void    signalAppendGraph (Prop prop,OGraph gra,OGraphValue val);
private:
    QMap<QString,Mod>   modules;

};




#endif // PROPERTIES_H
