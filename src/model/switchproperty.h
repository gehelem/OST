#ifndef OST_SWITCHPROPERTY_H
#define OST_SWITCHPROPERTY_H

#include "property.h"
#include "model/switchvalue.h"
#include "utils/propertyvisitor.h"

class SwitchProperty : public Property {
public:
    SwitchProperty() = delete;
    SwitchProperty(const QString &moduleName, const QString &deviceName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state, int rule, QObject *parent = nullptr)
            : Property(moduleName,deviceName, groupName, name, label, permission, state, parent), _rule(rule) {}

    ~SwitchProperty() override { for (SwitchValue* pVal : _switches) {delete pVal;} }

    void addSwitch(SwitchValue* pValue) { _switches.append(pValue); }

    [[nodiscard]] int getRule() const { return _rule; }
    inline const QList<SwitchValue*>& getSwitches() { return _switches; }
    void setSwitches(QList<SwitchValue*> switches)  {_switches=switches;}
    void setSwitch(QString name,bool value);
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this,&_moduleName); }
    void accept(PropertyVisitor* pVisitor,double s,double x,double y,double z) override {Q_UNUSED(pVisitor);Q_UNUSED(s);Q_UNUSED(x);Q_UNUSED(y)Q_UNUSED(z)};


private:
    int _rule;
    QList<SwitchValue*> _switches;
};


#endif //OST_SWITCHPROPERTY_H
