#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "jsondumper.h"
#include "model/numberproperty.h"
#include "model/textproperty.h"
#include "model/switchproperty.h"
#include "model/lightproperty.h"

void JSonDumper::visit(NumberProperty *pProperty) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    QJsonArray jsonNumbers;
    for (const NumberValue *pNumber : pProperty->getNumbers()) {
        QJsonObject jsonNumber;
        jsonNumber["name"] = pNumber->name();
        jsonNumber["label"] = pNumber->label();
        jsonNumber["format"] = pNumber->getFormat();
        jsonNumber["min"] = pNumber->getMin();
        jsonNumber["max"] = pNumber->getMax();
        jsonNumber["value"] = pNumber->getValue();
        jsonNumbers.append(jsonNumber);
    }
    json["numbers"] = jsonNumbers;
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

void JSonDumper::visit(SwitchProperty *pProperty) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    json["rule"] = pProperty->getRule();
    QJsonArray jsonSwitches;
    for( const SwitchValue* pSwitch : pProperty->getSwitches() ) {
        QJsonObject jsonSwitch;
        jsonSwitch["name"] = pSwitch->name();
        jsonSwitch["label"] = pSwitch->label();
        jsonSwitch["state"] = pSwitch->switchState();
        jsonSwitches.append(jsonSwitch);
    }
    json["switches"] = jsonSwitches;
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

void JSonDumper::visit(TextProperty *pProperty) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    QJsonArray jsonTexts;
    for( const TextValue* pText : pProperty->getTexts() ) {
        QJsonObject jsonText;
        jsonText["name"] = pText->name();
        jsonText["label"] = pText->label();
        jsonText["text"] = pText->text();
        jsonTexts.append(jsonText);
    }
    json["texts"] = jsonTexts;
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

void JSonDumper::visit(LightProperty *pProperty) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    QJsonArray jsonLights;
    for( const LightValue* pSwitch : pProperty->getLights() ) {
        QJsonObject jsonLight;
        jsonLight["name"] = pSwitch->name();
        jsonLight["label"] = pSwitch->label();
        jsonLight["state"] = pSwitch->lightState();
        jsonLights.append(jsonLight);
    }
    json["lights"] = jsonLights;
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

QJsonObject JSonDumper::dumpPropertyCommons(Property *pProperty) {

    QJsonObject json;

    json["deviceName"] = pProperty->getDeviceName();
    json["groupName"] = pProperty->getGroupName();
    json["deviceNameShort"] = pProperty->getDeviceNameShort();
    json["groupNameShort"] = pProperty->getGroupNameShort();
    json["name"] = pProperty->getName();
    json["label"] = pProperty->getLabel();
    json["permission"] = pProperty->getPermission();
    json["state"] = pProperty->getState();

    return json;
}

