#ifndef OST_JSONDUMPER_H
#define OST_JSONDUMPER_H

#include <QString>
#include "propertyvisitor.h"
#include "model/property.h"
#include <model/setup.h>
#include <model/value.h>
#include <model/textvalue.h>
#include <utils/propertytextdumper.h>
#include <model/propertystore.h>
#include "utils/propertyfactory.h"

class JSonDumper : public PropertyVisitor {


public:
    void visit(NumberProperty *pProperty,QString *modulName) override;
    void visit(SwitchProperty *pProperty,QString *modulName) override;
    void visit(TextProperty *pProperty,QString *modulName) override;
    void visit(LightProperty *pProperty,QString *modulName) override;

    [[nodiscard]] inline const QString& getResult() const { return _result; }
    [[nodiscard]] inline const QJsonObject& getJsonResult() const { return _jsonResult; }
    Property& setProFromJson(QJsonObject obj);

private:
    QString _result;
    QJsonObject _jsonResult;
    QJsonObject dumpPropertyCommons(Property* pProperty);

};


#endif //OST_JSONDUMPER_H
