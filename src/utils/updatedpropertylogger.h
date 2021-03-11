//
// Created by deufrai on 09/03/2021.
//

#ifndef OST_UPDATEDPROPERTYLOGGER_H
#define OST_UPDATEDPROPERTYLOGGER_H

#include "propertyvisitor.h"
#include "propertytextdumper.h"

class UpdatedPropertyLogger : public PropertyVisitor, public PropertyTextDumper {

public:
    void visit(NumberProperty *pProperty) override;
    void visit(SwitchProperty *pProperty) override;
    void visit(TextProperty *pProperty) override;
    void visit(LightProperty *pProperty) override;
};

#endif //OST_UPDATEDPROPERTYLOGGER_H
