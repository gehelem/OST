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
#include <elementmessage.h>
#include <elementprg.h>


namespace  OST
{

class ElementJsonDumper : public ElementVisitor
{

    public:
        ElementJsonDumper() = default;

        void visit(ElementBool *pElement) override;
        void visit(ElementBool* pElement, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(data)
        }
        void visit(ElementBool* pElement, QString &action, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data)
        }
        void visit(ElementInt *pElement) override;
        void visit(ElementInt* pElement, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(data)
        }
        void visit(ElementInt* pElement, QString &action, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data)
        }
        void visit(ElementFloat *pElement) override;
        void visit(ElementFloat* pElement, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(data)
        }
        void visit(ElementFloat* pElement, QString &action, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data)
        }
        void visit(ElementString *pElement) override;
        void visit(ElementString* pElement, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(data)
        }
        void visit(ElementString* pElement, QString &action, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data)
        }
        void visit(ElementLight *pElement) override;
        void visit(ElementLight* pElement, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(data)
        }
        void visit(ElementLight* pElement, QString &action, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data)
        }
        void visit(ElementImg *pElement) override;
        void visit(ElementImg* pElement, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(data)
        }
        void visit(ElementImg* pElement, QString &action, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data)
        }
        void visit(ElementVideo *pElement) override;
        void visit(ElementVideo* pElement, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(data)
        }
        void visit(ElementVideo* pElement, QString &action, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data)
        }
        void visit(ElementMessage *pElement) override;
        void visit(ElementMessage* pElement, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(data)
        }
        void visit(ElementMessage* pElement, QString &action, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data)
        }
        void visit(ElementPrg *pElement) override;
        void visit(ElementPrg* pElement, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(data)
        }
        void visit(ElementPrg* pElement, QString &action, QVariantMap &data ) override
        {
            Q_UNUSED(pElement);
            Q_UNUSED(action);
            Q_UNUSED(data)
        }

        [[nodiscard]] const QJsonObject &getResult() const
        {
            return mResult;
        }

    private:
        QJsonObject mResult;
        QString mJsonString;
        QJsonObject dumpElementCommons(ElementBase *pElement);
};
}
#endif //OST_ELEMENTJSONDUMPER_H
