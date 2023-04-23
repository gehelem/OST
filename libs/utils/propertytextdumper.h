#ifndef OST_PROPERTYTEXTDUMPER_H
#define OST_PROPERTYTEXTDUMPER_H

#include <string>
#include <propertybase.h>
#include <propertysimple.h>
#include <propertymulti.h>
#include <propertyvisitor.h>
namespace  OST
{

class Property;

class PropertyTextDumper : public PropertyVisitor
{

    public:
        PropertyTextDumper() = default;

        void visit(PropertyBase *pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(PropertyBase* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }
        void visit(PropertySimple *pProperty) override;
        void visit(PropertySimple* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }
        void visit(PropertyMulti *pProperty) override;
        void visit(PropertyMulti* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }

        [[nodiscard]] const QString &getResult() const
        {
            return _result;
        }

    private:

        std::string dumpPropertyCommons(PropertyBase *pProperty);

        QString _result;

};
}
#endif //OST_PROPERTYTEXTDUMPER_H
