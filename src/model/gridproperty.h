#ifndef OST_GRIDPROPERTY_H
#define OST_GRIDPROPERTY_H
#include <QVector>
#include "property.h"
#include "utils/propertyvisitor.h"

typedef struct
{
    double s =0;
    double x =0;
    double y =0;
    double z =0;
} GridPropertyValue;

typedef  QVector<GridPropertyValue> GridPropertyValues;

class GridProperty : public Property {

public:
    GridProperty() = delete;
    GridProperty(const QString &moduleName, const QString &deviceName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state,
                   const QString &Type, const QString &Sname, const QString &Xname, const QString &Yname, const QString &Zname,
                   QObject *parent = nullptr)
            : Property(moduleName,deviceName, groupName, name, label, permission, state, parent),
              _type(Type), _sname(Sname), _xname(Xname), _yname(Yname), _zname(Zname) {}

    ~GridProperty() override {}
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this,&_moduleName); }
    void accept(PropertyVisitor *pVisitor,double s,double x,double y,double z) override  { pVisitor->visit(this,&_moduleName,s,x,y,z); }

    void clear(void) { _vals.clear();}
    void append(double s, double x,double y)          {GridPropertyValue xy; xy.s=s; xy.x = x; xy.y=y ;  _vals.append(xy)    ;}
    void append(double s, double x,double y,double z) {GridPropertyValue xy; xy.s=s; xy.x = x; xy.y=y ; xy.z=z ;  _vals.append(xy)    ;}
    void append(double x,double y)                    {GridPropertyValue xy; xy.x = x; xy.y=y ;  _vals.append(xy)    ;}
    QVector<GridPropertyValue> getValues(void) {return  _vals;}
    QString getType(void) {return  _type;}

private:
    QString _type;
    QString _sname;
    QString _xname;
    QString _yname;
    QString _zname;
    QVector<GridPropertyValue> _vals;

};



#endif //OST_GRIDPROPERTY_H
