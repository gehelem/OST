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
    json["proptype"] = "number";
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
    json["proptype"] = "switch";
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
    json["proptype"] = "text";
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
    json["proptype"] = "light";
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

void JSonDumper::visit(MessageProperty *pProperty, QString *moduleName) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    json["proptype"] = "message";
    json["message"] = pProperty->getMessage();
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

void JSonDumper::visit(ImageProperty *pProperty, QString *moduleName) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    json["proptype"] = "image";
    json["URL"] = pProperty->getURL();
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);
}

void JSonDumper::visit(GridProperty *pProperty, QString *moduleName) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    json["proptype"] = "grid";
    json["type"] = pProperty->getType();
    QJsonArray jsonValues;
    for( const GridPropertyValue pValue : pProperty->getValues() ) {
        QJsonObject jsonValue;
        jsonValue["s"] = pValue.s;
        jsonValue["x"] = pValue.x;
        jsonValue["y"] = pValue.y;
        jsonValue["z"] = pValue.z;
        jsonValues.append(jsonValue);
    }
    json["values"] = jsonValues;
    QJsonDocument doc(json);
    _jsonResult = json;
    _result = doc.toJson(QJsonDocument::Compact);

}
void JSonDumper::visit(GridProperty *pProperty, QString *moduleName,double s,double x,double y,double z) {
    QJsonObject json = dumpPropertyCommons(pProperty);
    json["proptype"] = "grid";
    json["type"] = pProperty->getType();
    QJsonArray jsonValues;
    QJsonObject jsonValue;
    jsonValue["s"] = s;
    jsonValue["x"] = x;
    jsonValue["y"] = y;
    jsonValue["z"] = z;
    jsonValues.append(jsonValue);
    json["values"] = jsonValues;
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

TextProperty& JSonDumper::setProTextFromJson(QJsonObject obj)  {

    if (!(obj["texts"].isArray())) {
        auto* _propt = new TextProperty(
                    obj["modulename"].toString(),
                    obj["devicename"].toString(),
                    obj["groupname"].toString(),
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

NumberProperty& JSonDumper::setProNumberFromJson(QJsonObject obj)  {

    if (!(obj["numbers"].isArray())) {
        auto* _propn = new NumberProperty(
                    obj["modulename"].toString(),
                    obj["devicename"].toString(),
                    obj["groupname"].toString(),
                    obj["propname"].toString(),
                    obj["label"].toString(),
                    obj["permission"].toInt(),
                    obj["state"].toInt()
                );
        QJsonArray numbers = obj["numbers"].toArray();
        for (int i = 0; i < numbers.size() ; ++i)
        {
            QJsonObject number = numbers[i].toObject();
            _propn->addNumber(new NumberValue(
                        number["name"].toString(),
                        number["label"].toString(),
                        number["hint"].toString(),
                        number["value"].toDouble(),
                        number["format"].toString(),
                        number["min"].toDouble(),
                        number["max"].toDouble(),
                        number["step"].toDouble()
                    ));
        }
        return* _propn;
    }
}

SwitchProperty& JSonDumper::setProSwitchFromJson(QJsonObject obj)  {

    if (!(obj["switches"].isArray())) {
        auto* _props = new SwitchProperty(
                    obj["modulename"].toString(),
                    obj["devicename"].toString(),
                    obj["groupname"].toString(),
                    obj["propname"].toString(),
                    obj["label"].toString(),
                    obj["permission"].toInt(),
                    obj["state"].toInt(),
                    obj["rule"].toInt()
                );
        QJsonArray switches = obj["switches"].toArray();
        for (int i = 0; i < switches.size() ; ++i)
        {
            QJsonObject sw = switches[i].toObject();
            _props->addSwitch(new SwitchValue(
                        sw["name"].toString(),
                        sw["label"].toString(),
                        sw["hint"].toString(),
                        true                    /* frontend sends only click information ... must be handled in module */
                    ));
        }
        return* _props;
    }


}
