#ifndef OST_GRIDPROPERTY_H
#define OST_GRIDPROPERTY_H
#include <QVector>
#include "property.h"
#include "utils/propertyvisitor.h"

typedef struct
{
    double x =0;
    double y =0;
} GridPropertyValue;

typedef  QVector<GridPropertyValue> GridPropertyValues;

class GridProperty : public Property {

public:
    GridProperty() = delete;
    GridProperty(const QString &moduleName, const QString &deviceName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state, QObject *parent = nullptr)
            : Property(moduleName,deviceName, groupName, name, label, permission, state, parent) {}

    ~GridProperty() override {}
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this,&_moduleName); }

    void clear(void) { _vals.clear();}
    void append(double x,double y) {GridPropertyValue xy; xy.x = x; xy.y=y ;  _vals.append(xy)    ;}
    QVector<GridPropertyValue> getValues(void) {return  _vals;}

private:
    QVector<GridPropertyValue> _vals;
};



#endif //OST_GRIDPROPERTY_H
