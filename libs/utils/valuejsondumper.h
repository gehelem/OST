#ifndef OST_VALUEJSONDUMPER_H
#define OST_VALUEJSONDUMPER_H

#include <string>
#include <valuevisitor.h>
#include <valuebool.h>
#include <valueint.h>
#include <valuefloat.h>
#include <valuestring.h>
#include <valuelight.h>
#include <valueimg.h>
#include <valuemessage.h>


namespace  OST
{

class ValueJsonDumper : public ValueVisitor
{

    public:
        ValueJsonDumper() = default;

        void visit(ValueBool *pValue) override;
        void visit(ValueBool* pValue, QVariantMap &data ) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(data)
        }
        void visit(ValueInt *pValue) override;
        void visit(ValueInt* pValue, QVariantMap &data ) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(data)
        }
        void visit(ValueFloat *pValue) override;
        void visit(ValueFloat* pValue, QVariantMap &data ) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(data)
        }
        void visit(ValueString *pValue) override;
        void visit(ValueString* pValue, QVariantMap &data ) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(data)
        }
        void visit(ValueLight *pValue) override;
        void visit(ValueLight* pValue, QVariantMap &data ) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(data)
        }
        void visit(ValueImg *pValue) override;
        void visit(ValueImg* pValue, QVariantMap &data ) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(data)
        }
        void visit(ValueMessage *pValue) override;
        void visit(ValueMessage* pValue, QVariantMap &data ) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(data)
        }

        [[nodiscard]] const QJsonObject &getResult() const
        {
            return mResult;
        }

    private:
        QJsonObject mResult;
        QString mJsonString;
        QJsonObject dumpValueCommons(ValueBase *pValue);
};
}
#endif //OST_VALUEJSONDUMPER_H
