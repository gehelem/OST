#ifndef PROPERTIES_H
#define PROPERTIES_H
#pragma once
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
    QString name;
    /*! Element label - should be used on UI, whatever it is */
    QString label;
    /*! Module owner internal name */
    QString module;
    /*! Property internal name
    Should be used to build logic group of values,
    like radiobuttons or exclusive switches - see "pro struct" */
    QString  property;
    elemType type;
    QString text = "";
    double  num  = 0;
    bool    sw   = false;
};
struct pro
{
    /*! Property internal name */
    QString name;
    /*! Property label - should be used on UI, whatever it is */
    QString label;
    /*! Property rule - like indi */
    QString rule;
    /*! Property permission - like indi */
    QString permission;
    /*! Property state - like indi */
    QString state;
    /*! Module owner internal name */
    QString module;
    /*! Category owner internal name */
    QString category;
    /*! Group owner internal name */
    QString group;

};
struct group
{
    /*! Group internal name */
    QString name;
    /*! Group label - should be used on UI, whatever it is */
    QString label;
    /*! Module owner internal name */
    QString module;
    /*! Category owner internal name */
    QString category;

};
struct categ
{
    /*! Property internal name */
    QString name;
    /*! Property label - should be used on UI, whatever it is */
    QString label;
    /*! Module owner internal name */
    QString module;
};
class OSTProperties : public QObject
{
  Q_OBJECT
public:
    OSTProperties(QObject *parent = Q_NULLPTR);
    ~OSTProperties() = default;
    QString module;
    void    createCategory(QString name, QString label);
    void    createGroup(QString name, QString label, QString categ);
    void    createProp(QString name, QString label, QString rule,  QString category, QString permission, QString state, QString group);
    void    createElem(elem elt);
    void    createText(QString name,QString label,QString text,QString prop);
    void    createNum(QString name,QString label,double num,QString prop);
    void    createBool(QString name,QString label,bool sw,QString prop);
    void    createBTN(QString name,QString label,QString prop);
    void    deleteProp(QString name);
    void    appendElemToProp(QString elemname,QString propname);
    elem    getElem(QString name);
    QString getText(QString name);
    double  getNum(QString name);
    bool    getBool(QString name);
    void    setText(QString name,QString text);
    void    setNum(QString name,double num);
    void    setBool(QString name,bool sw);
    QJsonObject getJsonProp(QString propname);
signals:
    void    signalPropCreated(pro prop);
    void    signalPropDeleted(pro prop);
    void    signalElemCreated(elem elt);
    void    signalElemDeleted(elem elt);
    void    signalvalueChanged(elem elt);
private:
    QVector<elem>  elems;
    QVector<pro>   props;
    QVector<group> groups;
    QVector<categ> categs;

};


#endif // PROPERTIES_H
