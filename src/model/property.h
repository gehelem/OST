#ifndef OST_PROPERTY_H
#define OST_PROPERTY_H

#include <QObject>

class PropertyVisitor;

class Property : public QObject {
Q_OBJECT
protected:
    Property(QString moduleName, QString deviceName, QString groupName, QString name, QString label,
             int permission, int state, QObject* parent = nullptr)
    : QObject(parent),
      _moduleName(moduleName),_deviceName(deviceName), _groupName(groupName),
      _deviceNameShort(deviceName.replace(" ","")), _groupNameShort(groupName.replace(" ","")),
      _name(name), _label(label),
    _permission(permission), _state(state) {}

    QString _moduleName;
    QString _deviceName;
    QString _groupName;
    QString _deviceNameShort;
    QString _groupNameShort;
    QString _name;
    QString _label;
    int _permission;
    int _state;

public:
    virtual ~Property() = default;
    virtual void accept(PropertyVisitor* pVisitor) = 0;
    virtual void accept(PropertyVisitor* pVisitor,double s,double x,double y,double z) = 0;

    [[nodiscard]] inline const QString &getModuleName() const { return _moduleName; }
    [[nodiscard]] inline const QString &getDeviceName() const { return _deviceName; }
    [[nodiscard]] inline const QString &getGroupName() const { return _groupName; }
    [[nodiscard]] inline const QString &getDeviceNameShort() const { return _deviceNameShort; }
    [[nodiscard]] inline const QString &getGroupNameShort() const { return _groupNameShort; }
    [[nodiscard]] inline const QString &getName() const { return _name; }
    [[nodiscard]] inline const QString &getLabel() const { return _label; }
    [[nodiscard]] inline int getPermission() const { return _permission; }
    [[nodiscard]] inline int getState() const { return _state; }
    void setState (int val) { _state=val; }
};


#endif //OST_PROPERTY_H
