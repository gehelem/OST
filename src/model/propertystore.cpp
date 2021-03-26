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

void PropertyStore::cleanup() {

    for ( const QString& device : _store.keys() ) {
        for ( const QString& group : _store[device].keys() ) {
            for ( const QString& property : _store[device][group].keys() ) {
                delete _store[device][group][property];
            }
        }
    }
}

QList<Property *> PropertyStore::toList() const {

    QList<Property *> properties;

    for ( const QString& device : _store.keys() ) {
        for ( const QString& group : _store[device].keys() ) {
            for ( const QString& property : _store[device][group].keys() ) {
                properties.append(_store[device][group][property]);
            }
        }
    }

    return properties;
}
