#include "propertystore.h"


void PropertyStore::add(Property* pProperty) {

    _store[pProperty->getDeviceName()][pProperty->getGroupName()].insert(pProperty->getName(), pProperty);
}

void PropertyStore::update(Property* pProperty) {

    Property* oldProperty = _store[pProperty->getDeviceName()][pProperty->getGroupName()][pProperty->getName()];
    _store[pProperty->getDeviceName()][pProperty->getGroupName()][pProperty->getName()] = pProperty;
    delete oldProperty;
}
