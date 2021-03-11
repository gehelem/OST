#ifndef OST_NEWPROPERTYLOGGER_H
#define OST_NEWPROPERTYLOGGER_H

#include "propertyvisitor.h"
#include "propertytextdumper.h"

class NewPropertyLogger : public PropertyVisitor, public PropertyTextDumper {

public:
    void visit(NumberProperty *pProperty) override;
    void visit(SwitchProperty *pProperty) override;
    void visit(TextProperty *pProperty) override;
    void visit(LightProperty *pProperty) override;
};


#endif //OST_NEWPROPERTYLOGGER_H
