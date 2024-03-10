#ifndef OST_VALUESET_H
#define OST_VALUESET_H
#include <string>
#include <valueint.h>
#include <valuefloat.h>
#include <valuebool.h>
#include <valuestring.h>
#include <valuelight.h>
#include <valueimg.h>
#include <valuevideo.h>
#include <valuemessage.h>
#include <valuegraph.h>
#include <valueprg.h>

namespace  OST
{

class ValueSet: public ValueVisitor
{

    public:
        ValueSet() = default;
        void visit(ValueInt* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueInt* pValue, QVariantMap &data ) override;
        void visit(ValueInt* pValue, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ValueFloat* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueFloat* pValue, QVariantMap &data ) override;
        void visit(ValueFloat* pValue, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ValueBool* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueBool* pValue, QVariantMap &data ) override;
        void visit(ValueBool* pValue, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ValueString* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueString* pValue, QVariantMap &data ) override;
        void visit(ValueString* pValue, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ValueLight* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueLight* pValue, QVariantMap &data ) override;
        void visit(ValueLight* pValue, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ValueImg* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueImg* pValue, QVariantMap &data ) override;
        void visit(ValueImg* pValue, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ValueVideo* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueVideo* pValue, QVariantMap &data ) override;
        void visit(ValueVideo* pValue, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ValueMessage* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueMessage* pValue, QVariantMap &data ) override;
        void visit(ValueMessage* pValue, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ValueGraph* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueGraph* pValue, QVariantMap &data ) override;
        void visit(ValueGraph* pValue, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }

        void visit(ValuePrg* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValuePrg* pValue, QVariantMap &data ) override;
        void visit(ValuePrg* pValue, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pValue);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
};
}
#endif //OST_VALUESET_H
