#include "propertytextdumper.h"
#include "model/property.h"
#include <sstream>
#include <model/textproperty.h>

std::string PropertyTextDumper::dumpPropertyCommons(Property *pProperty) {

    std::stringstream stream;
    stream << "Device=" << pProperty->getDeviceName().toStdString()
           << ". Group=" << pProperty->getGroupName().toStdString()
           << ". Name=" << pProperty->getName().toStdString()
           << ". Label=" << pProperty->getLabel().toStdString()
           << ". Permission=" << _propertyPermsToNamesMap[pProperty->getPermission()] << "(" << pProperty->getPermission() << ")"
           << ". State=" << _propertyStatesToNamesMap[pProperty->getState()] << "(" << pProperty->getState() << ")";
    return stream.str();
}

std::string PropertyTextDumper::dumpNumbers(NumberProperty* pProperty) {

    std::stringstream stream;

    for (NumberValue* pNumber : pProperty->getNumbers() ) {

        stream << "   *Name=" << pNumber->name().toStdString()
               << ". Label=" << pNumber->label().toStdString()
               << ". Format=" << pNumber->getFormat().toStdString()
               << ". Value=" << pNumber->getValue()
               << ". Min=" << pNumber->getMin()
               << ". Max=" << pNumber->getMax()
               << ". Step=" << pNumber->getStep();
    }
    return stream.str();
}

std::string PropertyTextDumper::dump(NumberProperty *pProperty) {
    std::stringstream stream;
    stream << dumpPropertyCommons(pProperty) << dumpNumbers(pProperty);
    return stream.str();
}

std::string PropertyTextDumper::dumpSwitches(SwitchProperty *pProperty) {

    std::stringstream stream;

    stream << ". Rule=" << _switchRuleToNamesMap[pProperty->getRule()] << "(" << pProperty->getRule() << ")";

    for ( SwitchValue* pSwitch : pProperty->getSwitches() ) {
        stream << "   *Name=" << pSwitch->name().toStdString()
               << ". Label=" << pSwitch->label().toStdString()
               << ". State=" << pSwitch->switchState();
    }
    return stream.str();
}

std::string PropertyTextDumper::dump(SwitchProperty *pProperty) {
    std::stringstream stream;
    stream << dumpPropertyCommons(pProperty) << dumpSwitches(pProperty);
    return stream.str();
}

std::string PropertyTextDumper::dumpTexts(TextProperty *pProperty) {

    std::stringstream stream;

    for ( TextValue* pText : pProperty->getTexts() ) {

        stream << "  *Name=" << pText->name().toStdString()
               << ". Label=" << pText->label().toStdString()
               << ". Text=" << pText->text().toStdString() << ". ";
    }
    return stream.str();
}

std::string PropertyTextDumper::dump(TextProperty *pProperty) {
    std::stringstream stream;
    stream << dumpPropertyCommons(pProperty) << dumpTexts(pProperty);
    return stream.str();
}
