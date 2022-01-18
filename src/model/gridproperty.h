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
    double k =0;
} GridPropertyValue;

typedef  QVector<GridPropertyValue> GridPropertyValues;

class GridProperty : public Property {

public:
    GridProperty() = delete;
    GridProperty(const QString &moduleName, const QString &deviceName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state,
                   const QString &Type, const QString &Sname, const QString &Xname, const QString &Yname, const QString &Zname, const QString &Kname,
                   QObject *parent = nullptr)
            : Property(moduleName,deviceName, groupName, name, label, permission, state, parent),
              _type(Type), _sname(Sname), _xname(Xname), _yname(Yname), _zname(Zname) , _kname(Kname) {}

    ~GridProperty() override {}
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this,&_moduleName); }
    void accept(PropertyVisitor *pVisitor,double s,double x,double y,double z,double k) override  { pVisitor->visit(this,&_moduleName,s,x,y,z,k); }

    void clear(void) { _vals.clear();}
    void append(double s, double x,double y)          {GridPropertyValue xy; xy.s=s; xy.x = x; xy.y=y ;  _vals.append(xy)    ;}
    void append(double s, double x,double y,double z,double k) {GridPropertyValue xy; xy.s=s; xy.x = x; xy.y=y ; xy.z=z ; xy.k=k;  _vals.append(xy)    ;}
    void append(double x,double y)                    {GridPropertyValue xy; xy.x = x; xy.y=y ;  _vals.append(xy)    ;}
    QVector<GridPropertyValue> getValues(void) {return  _vals;}
    QString getType(void) {return  _type;}
    QString getSname(void) {return  _sname;}
    QString getXname(void) {return  _xname;}
    QString getYname(void) {return  _yname;}
    QString getZname(void) {return  _zname;}
    QString getKname(void) {return  _kname;}

private:
    QString _type;
    QString _sname;
    QString _xname;
    QString _yname;
    QString _zname;
    QString _kname;
    QVector<GridPropertyValue> _vals;

};



#endif //OST_GRIDPROPERTY_H
