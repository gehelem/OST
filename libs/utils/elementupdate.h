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
#include <elementmessage.h>
#include <elementprg.h>
#include <elementdate.h>
#include <elementtime.h>

namespace  OST
{

class ElementUpdate : public ElementVisitor
{

    public:
        ElementUpdate() = default;
        void visit(ElementInt* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementInt* pElement, QVariantMap &data ) override;
        void visit(ElementInt* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementFloat* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementFloat* pElement, QVariantMap &data ) override;
        void visit(ElementFloat* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementBool* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementBool* pElement, QVariantMap &data ) override;
        void visit(ElementBool* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementString* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementString* pElement, QVariantMap &data ) override;
        void visit(ElementString* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementLight* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementLight* pElement, QVariantMap &data ) override;
        void visit(ElementLight* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementImg* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementImg* pElement, QVariantMap &data ) override;
        void visit(ElementImg* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementVideo* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementVideo* pElement, QVariantMap &data ) override;
        void visit(ElementVideo* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementMessage* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementMessage* pElement, QVariantMap &data ) override;
        void visit(ElementMessage* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementPrg* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementPrg* pElement, QVariantMap &data ) override;
        void visit(ElementPrg* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementDate* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementDate* pElement, QVariantMap &data ) override;
        void visit(ElementDate* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementTime* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementTime* pElement, QVariantMap &data ) override;
        void visit(ElementTime* pElement, QString &action, QVariantMap &data) override;

        [[nodiscard]] const QVariantList &getGrid() const
        {
            return mGrid;
        }
    private:
        QVariantList mGrid;


};

}
#endif //OST_ELEMENTUPDATE_H
