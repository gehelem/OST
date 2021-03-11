#include "updatedpropertylogger.h"
#include <boost/log/trivial.hpp>
#include "model/numberproperty.h"
#include <sstream>

void UpdatedPropertyLogger::visit(TextProperty *pProperty) {

}

void UpdatedPropertyLogger::visit(SwitchProperty *pProperty) {
    std::stringstream stream;
    stream << "Updated Switch Property: " << dump(pProperty);
    BOOST_LOG_TRIVIAL(debug) << stream.str();
}

void UpdatedPropertyLogger::visit(NumberProperty *pProperty) {

    std::stringstream stream;
    stream << "Updated Number Property: " << dump(pProperty);
    BOOST_LOG_TRIVIAL(debug) << stream.str();
}
