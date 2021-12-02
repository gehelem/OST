#ifndef OST_PROPERTYTEXTDUMPER_H
#define OST_PROPERTYTEXTDUMPER_H

#include <string>
#include <model/numberproperty.h>
#include <model/switchproperty.h>
#include <utils/propertyvisitor.h>

class Property;

class PropertyTextDumper : public PropertyVisitor {

public:
    PropertyTextDumper() = default;

    void visit(NumberProperty *pProperty) override;
    void visit(SwitchProperty *pProperty) override;
    void visit(TextProperty *pProperty) override;
    void visit(LightProperty *pProperty) override;

    [[nodiscard]] const std::string& getResult() const { return _result; }

private:

    std::map<int, std::string> _statesToNamesMap = {
            {0, "Idle"},
            {1, "OK"},
            {2, "Busy"},
            {3, "Alert"}
    };

    std::map<int, std::string> _permsToNamesMap = {
            {0, "ReadOnly"},
            {1, "WriteOnly"},
            {2, "ReadWrite"}
    };

    std::map<int, std::string> _ruleToNamesMap = {
            {0, "1ofMany"},
            {1, "atMost1"},
            {2, "NofMany"}
    };

    std::string dumpPropertyCommons(Property* pProperty);

    std::string _result;

};

#endif //OST_PROPERTYTEXTDUMPER_H