#ifndef OST_PROPERTYTEXTDUMPER_H
#define OST_PROPERTYTEXTDUMPER_H

#include <string>
#include <propertymulti.h>

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
        void visit(PropertyMulti *pProperty) override;
        void visit(PropertyMulti* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }

        [[nodiscard]] const QString &getResult() const
        {
            return mResult;
        }

    private:

        std::string dumpPropertyCommons(PropertyBase *pProperty);

        QString mResult;

};
}
#endif //OST_PROPERTYTEXTDUMPER_H
