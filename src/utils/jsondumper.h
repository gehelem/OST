#ifndef OST_JSONDUMPER_H
#define OST_JSONDUMPER_H

#include <QString>
#include "propertyvisitor.h"
#include "model/property.h"

class JSonDumper : public PropertyVisitor {


public:
    void visit(NumberProperty *pProperty) override;
    void visit(SwitchProperty *pProperty) override;
    void visit(TextProperty *pProperty) override;
    void visit(LightProperty *pProperty) override;

    [[nodiscard]] inline const QString& getResult() const { return _result; }
    [[nodiscard]] inline const QJsonObject& getJsonResult() const { return _jsonResult; }

private:
    QString _result;
    QJsonObject _jsonResult;
    QJsonObject dumpPropertyCommons(Property* pProperty);

};


#endif //OST_JSONDUMPER_H
