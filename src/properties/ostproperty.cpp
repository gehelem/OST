#include <QtGlobal>
#include "ostproperty.h"
#include <boost/log/trivial.hpp>
#include "basemodule.h"

Oproperty::Oproperty(QString moduleName, QString devcatName, QString groupName, QString name, QString label,
                     int permission, int state, QObject *parent) :
    QObject(parent),
    _moduleName(moduleName),
    _devcatName(devcatName),
    _groupName(groupName),
    _devcatNameShort(devcatName.replace(" ","")),
    _groupNameShort(groupName.replace(" ","")),
    _name(name),
    _label(label),
    _permission(permission),
    _state(state)

{
    setObjectName(name);
    Basemodule *w = dynamic_cast<Basemodule *> (parent);
    connect(this,&Oproperty::changed,w,&Basemodule::OnOstPropertyChanged);
    BOOST_LOG_TRIVIAL(debug) <<  "********************   " << name.toStdString();
    BOOST_LOG_TRIVIAL(debug) <<  "********************   " << objectName().toStdString();

}

void Oproperty::setState (int val)
{
    _state=val;
    emit changed(this);
}
