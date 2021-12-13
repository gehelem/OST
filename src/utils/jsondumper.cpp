#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "jsondumper.h"
#include "model/numberproperty.h"
#include "model/textproperty.h"
#include "model/switchproperty.h"
#include "model/lightproperty.h"

void JSonDumper::visit(NumberProperty *pProperty, QString *moduleName) {
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
        jsonNumber["hint"] = pNumber->hint();
        jsonNumbers.append(jsonNumber);
    }
    json["numbers"] = jsonNumbers;
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

void JSonDumper::visit(SwitchProperty *pProperty, QString *moduleName) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    json["rule"] = pProperty->getRule();
    QJsonArray jsonSwitches;
    for( const SwitchValue* pSwitch : pProperty->getSwitches() ) {
        QJsonObject jsonSwitch;
        jsonSwitch["name"] = pSwitch->name();
        jsonSwitch["label"] = pSwitch->label();
        jsonSwitch["state"] = pSwitch->switchState();
        jsonSwitch["hint"] = pSwitch->hint();
        jsonSwitches.append(jsonSwitch);
    }
    json["switches"] = jsonSwitches;
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

void JSonDumper::visit(TextProperty *pProperty, QString *moduleName) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    QJsonArray jsonTexts;
    for( const TextValue* pText : pProperty->getTexts() ) {
        QJsonObject jsonText;
        jsonText["name"] = pText->name();
        jsonText["label"] = pText->label();
        jsonText["text"] = pText->text();
        jsonText["hint"] = pText->hint();
        jsonTexts.append(jsonText);
    }
    json["texts"] = jsonTexts;
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

void JSonDumper::visit(LightProperty *pProperty, QString *moduleName) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    QJsonArray jsonLights;
    for( const LightValue* pSwitch : pProperty->getLights() ) {
        QJsonObject jsonLight;
        jsonLight["name"] = pSwitch->name();
        jsonLight["label"] = pSwitch->label();
        jsonLight["state"] = pSwitch->lightState();
        jsonLight["hint"] = pSwitch->hint();
        jsonLights.append(jsonLight);
    }
    json["lights"] = jsonLights;
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

QJsonObject JSonDumper::dumpPropertyCommons(Property *pProperty) {

    QJsonObject json;

    json["modulename"] = pProperty->getModuleName();
    json["devicename"] = pProperty->getDeviceName();
    json["groupname"] = pProperty->getGroupName();
    json["devicenameshort"] = pProperty->getDeviceNameShort();
    json["groupnameshort"] = pProperty->getGroupNameShort();
    json["propname"] = pProperty->getName();
    json["label"] = pProperty->getLabel();
    json["permission"] = pProperty->getPermission();
    json["state"] = pProperty->getState();
    return json;
}

Property& JSonDumper::setProFromJson(QJsonObject obj)  {

    if (!(obj["texts"].isArray())) {
        auto* _propt = new TextProperty(
                    obj["modulename"].toString(),
                    obj["devicenameshort"].toString(),
                    obj["groupnameshort"].toString(),
                    obj["propname"].toString(),
                    obj["label"].toString(),
                    obj["permission"].toInt(),
                    obj["state"].toInt()
                );
        QJsonArray texts = obj["texts"].toArray();
        for (int i = 0; i < texts.size() ; ++i)
        {
            QJsonObject text = texts[i].toObject();
            _propt->addText(new TextValue(
                        text["name"].toString(),
                        text["label"].toString(),
                        text["hint"].toString(),
                        text["text"].toString()
                    ));
        }
        return* _propt;
    }
}
