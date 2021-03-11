//
// Created by deufrai on 09/03/2021.
//

#ifndef OST_PROPERTYTEXTDUMPER_H
#define OST_PROPERTYTEXTDUMPER_H

#include <string>
#include <model/numberproperty.h>
#include <model/switchproperty.h>

class Property;

class PropertyTextDumper {

protected:
    PropertyTextDumper() = default;
    std::string dumpPropertyCommons(Property* pProperty);
    std::string dumpNumbers(NumberProperty* pProperty);
    std::string dump(NumberProperty* pProperty);
    std::string dumpSwitches(SwitchProperty* pProperty);
    std::string dump(SwitchProperty* pProperty);
    std::string dumpTexts(TextProperty* pProperty);
    std::string dump(TextProperty* pProperty);

    std::map<int, std::string> _propertyStatesToNamesMap = {
            {0, "Idle"},
            {1, "OK"},
            {2, "Busy"},
            {3, "Alert"}
    };

    std::map<int, std::string> _propertyPermsToNamesMap = {
            {0, "ReadOnly"},
            {1, "WriteOnly"},
            {2, "ReadWrite"}
    };

    std::map<int, std::string> _switchRuleToNamesMap = {
            {0, "1ofMany"},
            {1, "atMost1"},
            {2, "NofMany"}
    };
};

#endif //OST_PROPERTYTEXTDUMPER_H
