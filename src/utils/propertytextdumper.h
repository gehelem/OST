#ifndef OST_PROPERTYTEXTDUMPER_H
#define OST_PROPERTYTEXTDUMPER_H

#include <string>
#include <model/textproperty.h>
#include <model/numberproperty.h>
#include <model/lightproperty.h>
#include <model/switchproperty.h>
#include <model/messageproperty.h>
#include <model/imageproperty.h>
#include <utils/propertyvisitor.h>


class Property;

class PropertyTextDumper : public PropertyVisitor {

public:
    PropertyTextDumper() = default;

    void visit(NumberProperty *pProperty,QString *moduleName) override;
    void visit(SwitchProperty *pProperty,QString *moduleName) override;
    void visit(TextProperty *pProperty,QString *moduleName) override;
    void visit(LightProperty *pProperty,QString *moduleName) override;
    void visit(MessageProperty *pProperty,QString *moduleName) override;
    void visit(ImageProperty *pProperty,QString *moduleName) override;

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

    std::string dumpPropertyCommons(Property* pProperty, QString *moduleName);

    std::string _result;

};

#endif //OST_PROPERTYTEXTDUMPER_H
