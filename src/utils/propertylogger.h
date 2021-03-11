#ifndef OST_PROPERTYLOGGER_H
#define OST_PROPERTYLOGGER_H

#include "propertyvisitor.h"
#include "propertytextdumper.h"

class PropertyLogger : public PropertyVisitor, public PropertyTextDumper {

public:
    PropertyLogger(bool updateEvent=true)  { _eventType = updateEvent ? "Updated" : "Created"; }
    void visit(NumberProperty *pProperty) override;
    void visit(SwitchProperty *pProperty) override;
    void visit(TextProperty *pProperty) override;
    void visit(LightProperty *pProperty) override;

private:
    std::string _eventType;
};


#endif //OST_PROPERTYLOGGER_H
