#ifndef OST_DEVICE_H
#define OST_DEVICE_H

#include <QString>

class Device {

public:
    Device(const QString& name);
    [[nodiscard]] inline const QString& getName() const {return _name;}
private:
    QString _name;
};


#endif //OST_DEVICE_H
