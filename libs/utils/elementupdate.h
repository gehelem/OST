#ifndef OST_ELEMENTUPDATE_H
#define OST_ELEMENTUPDATE_H
#include <string>
#include <elementint.h>
#include <elementfloat.h>
#include <elementbool.h>
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

class ElementUpdate : public ElementVisitor
{

    public:
        ElementUpdate() = default;
        void visit(ElementInt* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementFloat* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementBool* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementString* pElement, QVariantMap &data, bool &emitEvent ) override;
        void visit(ElementLight* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementImg* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementVideo* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementPrg* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementDate* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementTime* pElement, QVariantMap &data, bool &emitEvent) override;
        void visit(ElementDateTime* pElement, QVariantMap &data, bool &emitEvent) override;

        [[nodiscard]] const QVariantList &getGrid() const
        {
            return mGrid;
        }
    private:
        QVariantList mGrid;


};

}
#endif //OST_ELEMENTUPDATE_H
