#include "newpropertylogger.h"
#include <boost/log/trivial.hpp>
#include "model/numberproperty.h"
#include <sstream>


void NewPropertyLogger::visit(NumberProperty *pProperty) {

    std::stringstream stream;
    stream << "New Number Property: " << dump(pProperty);
    BOOST_LOG_TRIVIAL(debug) << stream.str();
}

void NewPropertyLogger::visit(SwitchProperty *pProperty) {
    std::stringstream stream;
    stream << "New Switch Property: " << dump(pProperty);
    BOOST_LOG_TRIVIAL(debug) << stream.str();
}

void NewPropertyLogger::visit(TextProperty *pProperty) {

}
