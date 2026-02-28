#ifndef OST_VALUEJSONDUMPER_H
#define OST_VALUEJSONDUMPER_H

#include <elementbase.h>

namespace  OST
{

class ValueBase;

class ValueJsonDumper : public ValueVisitor
{

    public:
        ValueJsonDumper(const OST::EvType evt, QVariant data, ElementBase *elt): mEvent(evt), mData(data), mElt(elt) {;};


        void visit(ValueInt *pValue) override;
        void visit(ValueBool* pValue) override;
        void visit(ValueFloat* pValue) override;
        void visit(ValueString* pValue) override;
        void visit(ValueLight* pValue) override;
        void visit(ValueImg*  pValue) override;
        void visit(ValueVideo*  pValue)override;
        void visit(ValuePrg* pValue) override;
        void visit(ValueDate* pValue) override;
        void visit(ValueTime* pValue) override;

        [[nodiscard]] const QJsonValue &getResult() const
        {
            return mResult;
        }

    private:
        QJsonValue mResult;
        /* pass event details */
        EvType mEvent;
        QVariant mData;
        ElementBase* mElt;


};
}
#endif //OST_PROPERTYJSONDUMPER_H
