//
// Created by deufrai on 12/03/2021.
//

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

    [[nodiscard]] inline const std::string& getResult() const { return _result; }

private:
    std::string _result;

    QJsonObject dumpPropertyCommons(Property* pProperty);
};


#endif //OST_JSONDUMPER_H
