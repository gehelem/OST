#ifndef OST_DEVCAT_H
#define OST_DEVCAT_H

#include <QString>

class Devcat {

public:
    Devcat(const QString& name);
    [[nodiscard]] inline const QString& getName() const {return _name;}
private:
    QString _name;
};


#endif //OST_DEVCAT_H
