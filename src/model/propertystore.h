#ifndef OST_PROPERTYSTORE_H
#define OST_PROPERTYSTORE_H

#include <QMap>
#include "lightproperty.h"
#include <boost/log/trivial.hpp>

class PropertyStore {

public:
    void add(Property* pProperty);
    void update(Property* pProperty);
    void remove(Property* pProperty);
    void cleanup();
    [[nodiscard]] QList<Property *> toList() const;
    [[nodiscard]] QMap<QString, QMap<QString, QMap<QString, Property*>>> toTreeList() const;

    [[nodiscard]] int getSize() const;
private:
    QMap<QString, QMap<QString, QMap<QString, Property*>>> _store;
};


#endif //OST_PROPERTYSTORE_H
