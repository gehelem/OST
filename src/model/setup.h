#ifndef OST_SETUP_H
#define OST_SETUP_H

#include <QMap>
#include "device.h"

class Setup {

public:
    virtual ~Setup();
    void addDevice(const Device *pDevice);
    void removeDeviceByName(const QString &deviceName);
    const Device* getDeviceByName(const QString &deviceName);
    void cleanup();
private:
    QMap<QString, const Device*> _devicesMap;

};


#endif //OST_SETUP_H
