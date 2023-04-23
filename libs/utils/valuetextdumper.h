#ifndef OST_VALUETEXTDUMPER_H
#define OST_VALUETEXTDUMPER_H

#include <string>
#include <valuevisitor.h>
#include <valuebase.h>
#include <valuebool.h>
#include <valueint.h>
#include <valuefloat.h>
#include <valuestring.h>
#include <valuelight.h>

namespace  OST
{

class ValueTextDumper : public ValueVisitor
{

    public:
        ValueTextDumper() = default;

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

        [[nodiscard]] const QString &getResult() const
        {
            return mResult;
        }

    private:

        std::string dumpValueCommons(ValueBase *pValue);
        QString mResult;

};
}
#endif //OST_VALUETEXTDUMPER_H
