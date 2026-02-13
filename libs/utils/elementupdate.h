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

namespace  OST
{

class ElementUpdate : public ElementVisitor
{

    public:
        ElementUpdate() = default;
        void visit(ElementInt* pElement, QVariantMap &data, OST::SignalType &signalType) override;
        void visit(ElementFloat* pElement, QVariantMap &data, OST::SignalType &signalType ) override;
        void visit(ElementBool* pElement, QVariantMap &data, OST::SignalType &signalType) override;
        void visit(ElementString* pElement, QVariantMap &data, OST::SignalType &signalType ) override;
        void visit(ElementLight* pElement, QVariantMap &data, OST::SignalType &signalType) override;
        void visit(ElementImg* pElement, QVariantMap &data, OST::SignalType &signalType) override;
        void visit(ElementVideo* pElement, QVariantMap &data, OST::SignalType &signalType) override;
        void visit(ElementPrg* pElement, QVariantMap &data, OST::SignalType &signalType) override;
        void visit(ElementDate* pElement, QVariantMap &data, OST::SignalType &signalType) override;
        void visit(ElementTime* pElement, QVariantMap &data, OST::SignalType &signalType) override;

        [[nodiscard]] const QVariantList &getGrid() const
        {
            return mGrid;
        }
    private:
        QVariantList mGrid;


};

}
#endif //OST_ELEMENTUPDATE_H
