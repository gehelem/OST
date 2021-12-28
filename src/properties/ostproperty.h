#ifndef OSTPROPERTY_H
#define OSTPROPERTY_H

#include <QObject>
#include <QMetaType>
#include "properties/ostproperty.h"

class Oproperty : public QObject {
Q_OBJECT
public:
    Oproperty(QString moduleName, QString devcatName, QString groupName, QString name, QString label,
             int permission, int state, QObject *parent);
    ~Oproperty() = default;

    [[nodiscard]] inline const QString &getModuleName() const { return _moduleName; }
    [[nodiscard]] inline const QString &getDevcatName() const { return _devcatName; }
    [[nodiscard]] inline const QString &getGroupName() const { return _groupName; }
    [[nodiscard]] inline const QString &getDevcatNameShort() const { return _devcatNameShort; }
    [[nodiscard]] inline const QString &getGroupNameShort() const { return _groupNameShort; }
    [[nodiscard]] inline const QString &getName() const { return _name; }
    [[nodiscard]] inline const QString &getLabel() const { return _label; }
    [[nodiscard]] inline int getPermission() const { return _permission; }
    [[nodiscard]] inline int getState() const { return _state; }
    void setState (int val);
signals:
    void changed(Oproperty* prop);

protected:


    QString _moduleName;
    QString _devcatName;
    QString _groupName;
    QString _devcatNameShort;
    QString _groupNameShort;
    QString _name;
    QString _label;
    int _permission;
    int _state;


};


#endif //OSTPROPERTY_H
