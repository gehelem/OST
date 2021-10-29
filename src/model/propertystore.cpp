#include "propertystore.h"


void PropertyStore::add(Property* pProperty) {

    _store[pProperty->getModuleName()][pProperty->getDevcatName()][pProperty->getGroupName()].insert(pProperty->getName(), pProperty);
}

void PropertyStore::update(Property* pProperty) {

    Property* oldProperty = _store[pProperty->getModuleName()][pProperty->getDevcatName()][pProperty->getGroupName()][pProperty->getName()];
    _store[pProperty->getModuleName()][pProperty->getDevcatName()][pProperty->getGroupName()][pProperty->getName()] = pProperty;
    delete oldProperty;
}

void PropertyStore::remove(Property *pProperty) {

    Property* oldProperty = _store[pProperty->getModuleName()][pProperty->getDevcatName()][pProperty->getGroupName()][pProperty->getName()];
    _store[pProperty->getModuleName()][pProperty->getDevcatName()][pProperty->getGroupName()].remove(pProperty->getName());
    delete oldProperty;
    delete pProperty;
}

int PropertyStore::getSize() const {

    int size = 0;

    for ( const QString& devcat : _store.keys() ) {
        for ( const QString& group : _store[devcat].keys() ) {
            size += _store[devcat][group].keys().count();
        }
    }
    return size;
}

void PropertyStore::cleanup() {

    for ( const QString& module : _store.keys() ) {
        for ( const QString& devcat : _store[module].keys() ) {
            for ( const QString& group : _store[module][devcat].keys() ) {
                for ( const QString& property : _store[module][devcat][group].keys() ) {
                    delete _store[module][devcat][group][property];
                    _store[module][devcat][group].remove(property);
                }
            }
        }
    }
}

QList<Property *> PropertyStore::toList() const {

    QList<Property *> properties;

    for ( const QString& module : _store.keys() ) {
        for ( const QString& devcat : _store[module].keys() ) {
            for ( const QString& group : _store[module][devcat].keys() ) {
                for ( const QString& property : _store[module][devcat][group].keys() ) {
                    properties.append(_store[module][devcat][group][property]);
                }
            }
        }
    }

    return properties;
}
