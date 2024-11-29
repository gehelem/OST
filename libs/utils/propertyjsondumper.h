#ifndef OST_PROPERTYJSONDUMPER_H
#define OST_PROPERTYJSONDUMPER_H

#include <string>
#include <propertymulti.h>


namespace  OST
{

class Property;

class PropertyJsonDumper : public PropertyVisitor
{

    public:
        PropertyJsonDumper() = default;

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

        [[nodiscard]] const QJsonObject &getResult() const
        {
            return mResult;
        }

    private:
        QJsonObject mResult;
        QJsonObject dumpPropertyCommons(PropertyBase *pProperty);

};
}
#endif //OST_PROPERTYJSONDUMPER_H
