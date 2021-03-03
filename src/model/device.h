#ifndef OST_DEVICE_H
#define OST_DEVICE_H

#include <string>

class Device {

public:
    Device(const std::string& name);
    [[nodiscard]] inline const std::string& getName() const {return _name;}
private:
    std::string _name;
};


#endif //OST_DEVICE_H
