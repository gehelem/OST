#ifndef OST_IMAGEPROPERTY_H
#define OST_IMAGEPROPERTY_H

#include "property.h"
#include "utils/propertyvisitor.h"


class ImageProperty : public Property {

public:
    ImageProperty() = delete;
    ImageProperty(const QString &moduleName, const QString &deviceName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state, QObject *parent = nullptr)
            : Property(moduleName,deviceName, groupName, name, label, permission, state, parent) {}

    ~ImageProperty() override {delete &_URL;}

    void setURL(QString pValue) { _URL = pValue; }

    QString getURL() { return _URL; }
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this,&_moduleName); }

private:
    QString _URL;
};



#endif //OST_MESSAGEPROPERTY_H
