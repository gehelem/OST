#include "propertystore.h"


void PropertyStore::add(Property* pProperty) {

    _store[pProperty->getDeviceName()][pProperty->getGroupName()].insert(pProperty->getName(), pProperty);
}

void PropertyStore::update(Property* pProperty) {

    Property* oldProperty = _store[pProperty->getDeviceName()][pProperty->getGroupName()][pProperty->getName()];
    _store[pProperty->getDeviceName()][pProperty->getGroupName()][pProperty->getName()] = pProperty;
    delete oldProperty;
}

void PropertyStore::remove(Property *pProperty) {

    Property* oldProperty = _store[pProperty->getDeviceName()][pProperty->getGroupName()][pProperty->getName()];
    _store[pProperty->getDeviceName()][pProperty->getGroupName()].remove(pProperty->getName());
    delete oldProperty;
    delete pProperty;
}

int PropertyStore::getSize() const {

    int size = 0;

    for ( const QString& device : _store.keys() ) {
        for ( const QString& group : _store[device].keys() ) {
            size += _store[device][group].keys().count();
        }
    }
    return size;
}
