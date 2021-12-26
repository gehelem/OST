#ifndef OST_PROPERTYSTORE_H
#define OST_PROPERTYSTORE_H

#include <QMap>
#include "property.h"
#include <boost/log/trivial.hpp>
#include "textproperty.h"
#include "numberproperty.h"
#include "switchproperty.h"
#include "lightproperty.h"
#include "messageproperty.h"
#include "imageproperty.h"

class PropertyStore {

public:
    void add(Property* pProperty);
    void update(Property* pProperty);
    void remove(Property* pProperty);
    void cleanup();
    [[nodiscard]] QList<Property *> toList() const;
    [[nodiscard]] QMap<QString, QMap<QString, QMap<QString, Property*>>> toTreeList() const;
    [[nodiscard]] int getSize() const;
    TextProperty*    getText   (QString device, QString group, QString name) {return static_cast<TextProperty*>(_store[device][group][name]);}
    NumberProperty*  getNumber (QString device, QString group, QString name) {return static_cast<NumberProperty*>(_store[device][group][name]);}
    SwitchProperty*  getSwitch (QString device, QString group, QString name) {return static_cast<SwitchProperty*>(_store[device][group][name]);}
    LightProperty*   getLight  (QString device, QString group, QString name) {return static_cast<LightProperty*>(_store[device][group][name]);}
    MessageProperty* getMessage(QString device, QString group, QString name) {return static_cast<MessageProperty*>(_store[device][group][name]);}
    ImageProperty*   getImage  (QString device, QString group, QString name) {return static_cast<ImageProperty*>(_store[device][group][name]);}

private:
    QMap<QString, QMap<QString, QMap<QString, Property*>>> _store;
};


#endif //OST_PROPERTYSTORE_H
