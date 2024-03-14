#ifndef OST_VALUEJSONDUMPER_H
#define OST_VALUEJSONDUMPER_H

#include <string>
#include <elementbase.h>

namespace  OST
{

class ValueBase;

class ValueJsonDumper : public ValueVisitor
{

    public:
        ValueJsonDumper() = default;

        void visit(ValueInt *pValue) override;
        void visit(ValueBool* pValue) override;
        void visit(ValueFloat* pValue) override;
        void visit(ValueString* pValue) override;
        void visit(ValueLight* pValue) override;
        void visit(ValueImg*  pValue) override;
        void visit(ValueVideo*  pValue)override;
        void visit(ValueMessage*  pValue)override;
        void visit(ValueGraph*  pValue) override;
        void visit(ValuePrg* pValue) override;

        [[nodiscard]] const QJsonValue &getResult() const
        {
            return mResult;
        }

    private:
        QJsonValue mResult;

};
}
#endif //OST_PROPERTYJSONDUMPER_H
