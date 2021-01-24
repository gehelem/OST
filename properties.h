#ifndef PROPERTIES_H
#define PROPERTIES_H
#include <QtCore>
#include <indibase.h>
enum elemType
{
    /*! Text element */
    ET_TEXT,
    /*! Number element */
    ET_NUM,
    /*! Boolean element */
    ET_BOOL,
    /*! Light element - just like indi light properties OK/Idle/busy/error */
    ET_LIGHT,
    /*! Message */
    ET_MESSAGE,
    /*! Image element */
    ET_IMAGE,
    /*! Graphic element */
    ET_GRAPH,
    /*! Button element AKA "clic"
        It doesn't store any value, should be used as an action signal */
    ET_BTN
};
/*! Elements represent a signel value */
struct elem
{
    /*! Element internal name */
    QString elemname;
    /*! Element label - should be used on UI, whatever it is */
    QString elemlabel;
    /*! Module owner internal name */
    QString modulename;
    /*! Category owner internal name */
    QString categname;
    /*! Group owner internal name */
    QString groupname;
    /*! Property internal name
    Should be used to build logic group of values,
    like radiobuttons or exclusive switches - see "struct pro" */
    QString  propname;
    elemType type;
    QString text = "";
    double  num  = 0;
    bool    sw   = false;
};
struct pro
{
    /*! Property internal name */
    QString propname;
    /*! Property label - should be used on UI, whatever it is */
    QString proplabel;
    /*! Property rule - like indi */
    QString rule;
    /*! Property permission - like indi */
    QString permission;
    /*! Property state - like indi */
    QString state;
    /*! Module owner internal name */
    QString modulename;
    /*! Category owner internal name */
    QString categname;
    /*! Group owner internal name */
    QString groupname;

};
struct group
{
    /*! Group internal name */
    QString groupname;
    /*! Group label - should be used on UI, whatever it is */
    QString grouplabel;
    /*! Module owner internal name */
    QString modulename;
    /*! Category owner internal name */
    QString categname;

};
struct categ
{
    /*! Property internal name */
    QString categname;
    /*! Property label - should be used on UI, whatever it is */
    QString categlabel;
    /*! Module owner internal name */
    QString modulename;
};
struct mod
{
    /*! module internal name */
    QString modulename;
    /*! module label - should be used on UI, whatever it is */
    QString modulelabel;
};

class OSTProperties : public QObject
{
  Q_OBJECT
public:
    OSTProperties(QObject *parent = Q_NULLPTR);
    ~OSTProperties() = default;
    QString module;
    void    createMod  (QString modulename, QString modulelabel);
    void    createCateg(QString modulename, QString categname,  QString categlabel);
    void    createGroup(QString modulename, QString groupname,  QString grouplabel, QString categname);
    void    createProp (QString modulename, QString propname,   QString proplabel,  QString groupname, QString categname, QString rule,  QString permission, QString state);
    void    createElem (elem elt);
    void    createText(QString modulename, QString elemname,QString elemlabel,QString propname, QString groupname, QString categname,QString text);
    void    createNum (QString modulename, QString elemname,QString elemlabel,QString propname, QString groupname, QString categname,double num);
    void    createBool(QString modulename, QString elemname,QString elemlabel,QString propname, QString groupname, QString categname,bool sw);
    void    createBTN (QString modulename, QString elemname,QString elemlabel,QString propname, QString groupname, QString categname);
    void    deleteProp(QString modulename, QString propname);
    elem    getElem   (QString modulename, QString elemname);
    QString getText   (QString modulename, QString elemname);
    double  getNum    (QString modulename, QString elemname);
    bool    getBool   (QString modulename, QString elemname);
    void    setText   (QString modulename, QString elemname,QString text);
    void    setNum    (QString modulename, QString elemname,double num);
    void    setBool   (QString modulename, QString elemname,bool sw);
    QJsonObject getJsonProp(QString propname, QString modulename);
    void    dumproperties(void);
signals:
    void    signalPropCreated (pro  prop);
    void    signalPropDeleted (pro  prop);
    void    signalElemCreated (elem elt);
    void    signalElemDeleted (elem elt);
    void    signalvalueChanged(elem elt);
private:
    QVector<elem>  elems;
    QVector<pro>   props;
    QVector<group> groups;
    QVector<categ> categs;
    QVector<mod>   mods;

};


#endif // PROPERTIES_H
