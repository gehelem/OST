#ifndef OST_ELEMENTJSONDUMPER_H
#define OST_ELEMENTJSONDUMPER_H

#include <elementbool.h>
#include <elementint.h>
#include <elementfloat.h>
#include <elementstring.h>
#include <elementlight.h>
#include <elementimg.h>
#include <elementvideo.h>
#include <elementprg.h>
#include <elementdate.h>
#include <elementtime.h>
#include <elementdatetime.h>


namespace  OST
{

class ElementJsonDumper : public ElementVisitor
{

    public:
        ElementJsonDumper(const OST::EvType evt, QVariant data, ElementBase *elt): mEvent(evt), mData(data), mElt(elt) {;};

        void visit(ElementBool* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementInt* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementFloat* pElement, QVariantMap &data, bool &emitEvent ) override;
        void visit(ElementString* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementLight* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementImg* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementVideo* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementPrg* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementDate* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementTime* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementDateTime* pElement, QVariantMap &data, bool &emitEvent) override;

        [[nodiscard]] const QJsonObject &getResult() const
        {
            return mResult;
        }

    private:
        QJsonObject mResult;
        QString mJsonString;
        QJsonObject dumpElementCommons(ElementBase *pElement);

        /* pass event details */
        EvType mEvent;
        QVariant mData;
        ElementBase* mElt;

};
}
#endif //OST_ELEMENTJSONDUMPER_H
