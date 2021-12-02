#ifndef OST_TEXTPROPERTY_H
#define OST_TEXTPROPERTY_H


#include "property.h"
#include "model/textvalue.h"
#include "utils/propertyvisitor.h"


class TextProperty : public Property {

public:
    TextProperty() = delete;
    TextProperty(const QString &deviceName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state, QObject *parent = nullptr)
            : Property(deviceName, groupName, name, label, permission, state, parent) {}

    ~TextProperty() override { for (TextValue* pVal : _texts) {delete pVal;} }

    void addText(TextValue* pValue) { _texts.append(pValue); }
    inline const QList<TextValue*>& getTexts() { return _texts; }
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this); }

private:
    QList<TextValue*> _texts;
};



#endif //OST_TEXTPROPERTY_H
