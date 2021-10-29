#include "setup.h"

#include "boost/log/trivial.hpp"


Setup::~Setup()
{
    cleanup();
}

void Setup::addDevcat(const Devcat* pDevcat) {

    if( ! _devcatsMap.contains(pDevcat->getName()) ) {
        _devcatsMap[pDevcat->getName()] = pDevcat;
        BOOST_LOG_TRIVIAL(info) << "New device/category added: " << pDevcat->getName().toStdString();
    } else {
        BOOST_LOG_TRIVIAL(warning) << "Duplicate device/category name: " << pDevcat->getName().toStdString() << ". This should not happen";
    }
}

const Devcat *Setup::getDevcatByName(const QString &devcatName) {
    return _devcatsMap.contains(devcatName) ? _devcatsMap[devcatName] : nullptr;
}

void Setup::removeDevcatByName(const QString &devcatName) {
    if( _devcatsMap.contains(devcatName) ) {
        delete _devcatsMap[devcatName];;
        _devcatsMap.remove(devcatName);
    }
    BOOST_LOG_TRIVIAL(info) << "Device/category removed: " << devcatName.toStdString();
}

void Setup::cleanup() {

    for( const QString& devcatName : _devcatsMap.keys() ) {
        removeDevcatByName(devcatName);
    }
}
