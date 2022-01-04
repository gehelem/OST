#ifndef OST_MESSAGEPROPERTY_H
#define OST_MESSAGEPROPERTY_H

#include "property.h"
#include "utils/propertyvisitor.h"


class MessageProperty : public Property {

public:
    MessageProperty() = delete;
    MessageProperty(const QString &moduleName, const QString &deviceName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state, QObject *parent = nullptr)
            : Property(moduleName,deviceName, groupName, name, label, permission, state, parent) {}

    ~MessageProperty() override {delete &_message;}

    void setMessage(QString pValue) { _message = pValue; }

    QString getMessage() { return _message; }
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this,&_moduleName); }
    void accept(PropertyVisitor* pVisitor,double s,double x,double y,double z) override {};


private:
    QString _message;
};



#endif //OST_MESSAGEPROPERTY_H
