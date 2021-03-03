#ifndef OST_SETUP_H
#define OST_SETUP_H

#include <QMap>
#include "device.h"

class Setup {

public:
    void addDevice(const Device *pDevice);
    void removeDeviceByName(const std::string& deviceName);
    const Device* getDeviceByName(const std::string& deviceName);
private:
    QMap<std::string, const Device*> _devicesMap;
};


#endif //OST_SETUP_H
