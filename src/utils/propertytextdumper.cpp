#include "propertytextdumper.h"
#include "model/property.h"
#include <sstream>
#include <model/textproperty.h>
#include <model/lightproperty.h>

std::string PropertyTextDumper::dumpPropertyCommons(Property *pProperty) {

    std::stringstream stream;
    stream << "Device=" << pProperty->getDeviceName().toStdString()
           << ". Group=" << pProperty->getGroupName().toStdString()
           << ". Name=" << pProperty->getName().toStdString()
           << ". Label=" << pProperty->getLabel().toStdString()
           << ". Permission=" << _permsToNamesMap[pProperty->getPermission()] << "(" << pProperty->getPermission() << ")"
           << ". State=" << _statesToNamesMap[pProperty->getState()] << "(" << pProperty->getState() << ")";
    return stream.str();
}

void PropertyTextDumper::visit(NumberProperty *pProperty) {

    std::stringstream stream;

    stream << "Number Property : " << dumpPropertyCommons(pProperty);

    for (NumberValue* pNumber : pProperty->getNumbers() ) {

        stream << "   *Name=" << pNumber->name().toStdString()
               << ". Label=" << pNumber->label().toStdString()
               << ". Format=" << pNumber->getFormat().toStdString()
               << ". Value=" << pNumber->getValue()
               << ". Min=" << pNumber->getMin()
               << ". Max=" << pNumber->getMax()
               << ". Step=" << pNumber->getStep();
    }
    _result = stream.str();
}

void PropertyTextDumper::visit(SwitchProperty *pProperty) {

    std::stringstream stream;

    stream << "Switch Property : " << dumpPropertyCommons(pProperty);
    stream << ". Rule=" << _ruleToNamesMap[pProperty->getRule()] << "(" << pProperty->getRule() << ")";

    for ( SwitchValue* pSwitch : pProperty->getSwitches() ) {
        stream << "   *Name=" << pSwitch->name().toStdString()
               << ". Label=" << pSwitch->label().toStdString()
               << ". State=" << pSwitch->switchState();
    }
    _result = stream.str();
}

void PropertyTextDumper::visit(TextProperty *pProperty) {

    std::stringstream stream;

    stream << "Text Property : " << dumpPropertyCommons(pProperty);

    for ( TextValue* pText : pProperty->getTexts() ) {

        stream << "  *Name=" << pText->name().toStdString()
               << ". Label=" << pText->label().toStdString()
               << ". Text=" << pText->text().toStdString() << ". ";
    }
    _result = stream.str();
}

void PropertyTextDumper::visit(LightProperty *pProperty) {

    std::stringstream stream;

    stream << "Light Property : " << dumpPropertyCommons(pProperty);

    for ( LightValue* pLight : pProperty->getLights() ) {
        stream << "   *Name=" << pLight->name().toStdString()
               << ". Label=" << pLight->label().toStdString()
               << ". State=" << _statesToNamesMap[pLight->lightState()] << "(" << pLight->lightState() << ")";
    }
    _result = stream.str();
}
