#ifndef OST_ELEMENTJSONDUMPER_H
#define OST_ELEMENTJSONDUMPER_H

#include <string>
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


namespace  OST
{

class ElementJsonDumper : public ElementVisitor
{

    public:
        ElementJsonDumper(const QString &type): mType(type) {;};

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

        [[nodiscard]] const QJsonObject &getResult() const
        {
            return mResult;
        }

    private:
        QJsonObject mResult;
        QString mJsonString;
        QJsonObject dumpElementCommons(ElementBase *pElement);
        QString mType;
};
}
#endif //OST_ELEMENTJSONDUMPER_H
