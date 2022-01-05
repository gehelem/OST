#ifndef OST_NUMBERPROPERTY_H
#define OST_NUMBERPROPERTY_H

#include "property.h"
#include "model/numbervalue.h"
#include "utils/propertyvisitor.h"


class NumberProperty : public Property {

public:
    NumberProperty() = delete;
    NumberProperty(const QString &moduleName,const QString &deviceName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state, QObject *parent = nullptr)
    : Property(moduleName,deviceName, groupName, name, label, permission, state, parent) {}

    ~NumberProperty() override { for (NumberValue* pVal : _numbers) {delete pVal;} }

    void addNumber(NumberValue* pValue) { _numbers.append(pValue); }
    void setNumber(QString name, double value) {
                                                for (int i=0; i< _numbers.size();i++ ) {
                                                    if(_numbers[i]->name()==name)  _numbers[i]->setValue(value);
                                                };
                                              }
    inline const QList<NumberValue*>& getNumbers() { return _numbers; }
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this,&_moduleName); }
    void accept(PropertyVisitor* pVisitor,double s,double x,double y,double z) override {Q_UNUSED(pVisitor);Q_UNUSED(s);Q_UNUSED(x);Q_UNUSED(y)Q_UNUSED(z)};


private:
    QList<NumberValue*> _numbers;
};


#endif //OST_NUMBERPROPERTY_H
