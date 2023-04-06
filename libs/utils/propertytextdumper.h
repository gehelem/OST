#ifndef OST_PROPERTYTEXTDUMPER_H
#define OST_PROPERTYTEXTDUMPER_H

#include <string>
#include <numberproperty.h>
#include <basicproperty.h>
#include <propertyvisitor.h>

class Property;

class PropertyTextDumper : public PropertyVisitor
{

    public:
        PropertyTextDumper() = default;

        void visit(RootProperty *pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(BasicProperty *pProperty) override;
        void visit(NumberProperty *pProperty) override;
        void visit(TextProperty *pProperty) override;
        void visit(RootProperty* pProperty, QVariantMap &data ) override {}
        void visit(BasicProperty* pProperty, QVariantMap &data ) override {}
        void visit(NumberProperty* pProperty, QVariantMap &data ) override {}
        void visit(TextProperty* pProperty, QVariantMap &data ) override {}

        [[nodiscard]] const QString &getResult() const
        {
            return _result;
        }

    private:

        std::string dumpPropertyCommons(RootProperty *pProperty);

        QString _result;

};

#endif //OST_PROPERTYTEXTDUMPER_H
