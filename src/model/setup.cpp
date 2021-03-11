#include "setup.h"

#include "boost/log/trivial.hpp"


Setup::~Setup()
{
    purgeDeviceMap();
}

void Setup::addDevice(const Device* pDevice) {

    if( ! _devicesMap.contains(pDevice->getName()) ) {
        _devicesMap[pDevice->getName()] = pDevice;
        BOOST_LOG_TRIVIAL(info) << "New device added: " << pDevice->getName().toStdString();
    } else {
        BOOST_LOG_TRIVIAL(warning) << "Duplicate device name: " << pDevice->getName().toStdString() << ". This should not happen";
    }
}

const Device *Setup::getDeviceByName(const QString &deviceName) {
    return _devicesMap.contains(deviceName) ? _devicesMap[deviceName] : nullptr;
}

void Setup::removeDeviceByName(const QString &deviceName) {
    if( _devicesMap.contains(deviceName) ) {
        delete _devicesMap[deviceName];;
        _devicesMap.remove(deviceName);
    }
    BOOST_LOG_TRIVIAL(info) << "Device removed: " << deviceName.toStdString();
}

void Setup::purgeDeviceMap() {

    for( QString deviceName : _devicesMap.keys() ) {
        removeDeviceByName(deviceName);
    }
}
