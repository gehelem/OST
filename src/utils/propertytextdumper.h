//
// Created by deufrai on 09/03/2021.
//

#ifndef OST_PROPERTYTEXTDUMPER_H
#define OST_PROPERTYTEXTDUMPER_H

#include <string>
#include <model/numberproperty.h>

class Property;

class PropertyTextDumper {

protected:
    PropertyTextDumper() = default;
    std::string dumpPropertyCommons(Property* pProperty);
    std::string dumpNumbers(NumberProperty* pProperty);
    std::string dump(NumberProperty* pProperty);

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
};

#endif //OST_PROPERTYTEXTDUMPER_H
