#include "propertylogger.h"
#include <boost/log/trivial.hpp>
#include "model/numberproperty.h"
#include <sstream>


void PropertyLogger::visit(NumberProperty *pProperty) {

    std::stringstream stream;
    stream << "Number Property " << _eventType << ": " << dump(pProperty);
    BOOST_LOG_TRIVIAL(debug) << stream.str();
}

void PropertyLogger::visit(SwitchProperty *pProperty) {
    std::stringstream stream;
    stream << "Switch Property " << _eventType << ": " << dump(pProperty);
    BOOST_LOG_TRIVIAL(debug) << stream.str();
}

void PropertyLogger::visit(TextProperty *pProperty) {
    std::stringstream stream;
    stream << "Text Property " << _eventType << ": " << dump(pProperty);
    BOOST_LOG_TRIVIAL(debug) << stream.str();
}

void PropertyLogger::visit(LightProperty *pProperty) {
    std::stringstream stream;
    stream << "Light Property " << _eventType << ": " << dump(pProperty);
    BOOST_LOG_TRIVIAL(debug) << stream.str();
}
