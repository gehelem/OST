#ifndef OST_VALUEUPDATE_H
#define OST_VALUEUPDATE_H
#include "propertybase.h"
#include <string>
#include <valueint.h>
#include <valuefloat.h>
#include <valuebool.h>
#include <valuestring.h>
#include <valuelight.h>
#include <valueimg.h>
#include <valuemessage.h>
#include <valuegraph.h>

namespace  OST
{

class ValueUpdate : public ValueVisitor
{

    public:
        ValueUpdate() = default;
        void visit(ValueInt* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueInt* pProperty, QVariantMap &data ) override;
        void visit(ValueInt* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueFloat* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueFloat* pProperty, QVariantMap &data ) override;
        void visit(ValueFloat* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueBool* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueBool* pProperty, QVariantMap &data ) override;
        void visit(ValueBool* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueString* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueString* pProperty, QVariantMap &data ) override;
        void visit(ValueString* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueLight* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueLight* pProperty, QVariantMap &data ) override;
        void visit(ValueLight* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueImg* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueImg* pProperty, QVariantMap &data ) override;
        void visit(ValueImg* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueMessage* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueMessage* pProperty, QVariantMap &data ) override;
        void visit(ValueMessage* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueGraph* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueGraph* pProperty, QVariantMap &data ) override;
        void visit(ValueGraph* pValue, QString &action, QVariantMap &data) override;

        [[nodiscard]] const QVariantList &getGrid() const
        {
            return mGrid;
        }
    private:
        QVariantList mGrid;


};
}
#endif //OST_PROPERTYUPDATE_H
