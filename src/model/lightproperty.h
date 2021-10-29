#ifndef OST_LIGHTPROPERTY_H
#define OST_LIGHTPROPERTY_H

#include "property.h"
#include "model/lightvalue.h"
#include "utils/propertyvisitor.h"


class LightProperty : public Property {
public:
    LightProperty() = delete;
    LightProperty(const QString &devcatName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state, QObject *parent = nullptr)
            : Property(devcatName, groupName, name, label, permission, state, parent) {}

    ~LightProperty() override { for (LightValue* pVal : _lights) {delete pVal;} }

    void addLight(LightValue* pValue) { _lights.append(pValue); }
    inline const QList<LightValue*>& getLights() { return _lights; }
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this); }

private:
    QList<LightValue*> _lights;
};


#endif //OST_LIGHTPROPERTY_H
