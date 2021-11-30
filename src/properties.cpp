#include "properties.h"
#include "basemodule.h"

Property::Property( QString name, QString label, int permission, int state, QObject* parent)
: QObject(parent), _name(name), _label(label), _permission(permission), _state(state)
{
    setObjectName(name);
    //connect();
    emit propertyCreated(this);
}

