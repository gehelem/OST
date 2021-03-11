//
// Created by deufrai on 09/03/2021.
//

#ifndef OST_NUMBERPROPERTY_H
#define OST_NUMBERPROPERTY_H

#include "property.h"
#include "model/numbervalue.h"
#include "utils/propertyvisitor.h"


class NumberProperty : public Property {

public:
    NumberProperty() = delete;
    NumberProperty(const QString &deviceName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state, QObject *parent = nullptr)
    : Property(deviceName, groupName, name, label, permission, state, parent) {}

    ~NumberProperty() override { for (NumberValue* pVal : _numbers) {delete pVal;} }

    void addNumber(NumberValue* pValue) { _numbers.append(pValue); }
    inline const QList<NumberValue*>& getNumbers() { return _numbers; }
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this); }

private:
    QList<NumberValue*> _numbers;
};


#endif //OST_NUMBERPROPERTY_H
