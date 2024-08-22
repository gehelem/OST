#ifndef OST_ELEMENTSET_H
#define OST_ELEMENTSET_H
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

namespace  OST
{

class ElementSet: public ElementVisitor
{

    public:
        ElementSet() = default;
        void visit(ElementInt* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementInt* pElement, QVariantMap &data ) override;
        void visit(ElementInt* pElement, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ElementFloat* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementFloat* pElement, QVariantMap &data ) override;
        void visit(ElementFloat* pElement, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ElementBool* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementBool* pElement, QVariantMap &data ) override;
        void visit(ElementBool* pElement, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ElementString* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementString* pElement, QVariantMap &data ) override;
        void visit(ElementString* pElement, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ElementLight* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementLight* pElement, QVariantMap &data ) override;
        void visit(ElementLight* pElement, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ElementImg* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementImg* pElement, QVariantMap &data ) override;
        void visit(ElementImg* pElement, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ElementVideo* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementVideo* pElement, QVariantMap &data ) override;
        void visit(ElementVideo* pElement, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ElementMessage* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementMessage* pElement, QVariantMap &data ) override;
        void visit(ElementMessage* pElement, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
        void visit(ElementPrg* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementPrg* pElement, QVariantMap &data ) override;
        void visit(ElementPrg* pElement, QString &action, QVariantMap &data) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data);
        }
};
}
#endif //OST_ELEMENTSET_H
