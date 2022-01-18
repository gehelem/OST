#ifndef OST_TEXTPROPERTY_H
#define OST_TEXTPROPERTY_H


#include "property.h"
#include "model/textvalue.h"
#include "utils/propertyvisitor.h"


class TextProperty : public Property {

public:
    TextProperty() = delete;
    TextProperty(const QString &moduleName, const QString &deviceName, const QString &groupName, const QString &name, const QString &label,
                   int permission, int state, QObject *parent = nullptr)
            : Property(moduleName,deviceName, groupName, name, label, permission, state, parent) {}

    ~TextProperty() override { for (TextValue* pVal : _texts) {delete pVal;} }

    void addText(TextValue* pValue) { _texts.append(pValue); }
    void setText(QString name, QString txt) {
                                                for (int i=0; i< _texts.size();i++ ) {
                                                    if(_texts[i]->name()==name)  _texts[i]->setText(txt);
                                                };
                                              }

    inline const QList<TextValue*>& getTexts() { return _texts; }
    void accept(PropertyVisitor *pVisitor) override { pVisitor->visit(this,&_moduleName); }
    void accept(PropertyVisitor* pVisitor,double s,double x,double y,double z,double k) override {Q_UNUSED(pVisitor) Q_UNUSED(s) Q_UNUSED(x) Q_UNUSED(y) Q_UNUSED(z) Q_UNUSED(k)};

private:
    QList<TextValue*> _texts;
};



#endif //OST_TEXTPROPERTY_H
