#ifndef OST_VALUEUPDATE_H
#define OST_VALUEUPDATE_H
#include "propertybase.h"
#include <QtCore>
#include <string>
#include <valueint.h>
#include <valuefloat.h>
#include <valuebool.h>
#include <valuestring.h>
#include <valuelight.h>
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
        void visit(ValueFloat* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueFloat* pProperty, QVariantMap &data ) override;
        void visit(ValueBool* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueBool* pProperty, QVariantMap &data ) override;
        void visit(ValueString* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueString* pProperty, QVariantMap &data ) override;
        void visit(ValueLight* pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(ValueLight* pProperty, QVariantMap &data ) override;


};
}
#endif //OST_PROPERTYUPDATE_H
