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
#include <elementgraph.h>
#include <elementprg.h>

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
        void visit(ElementGraph* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementGraph* pElement, QVariantMap &data ) override;
        void visit(ElementGraph* pElement, QString &action, QVariantMap &data) override;
        void visit(ElementPrg* pElement) override
        {
            Q_UNUSED(pElement)
        }
        void visit(ElementPrg* pElement, QVariantMap &data ) override;
        void visit(ElementPrg* pElement, QString &action, QVariantMap &data) override;

        [[nodiscard]] const QVariantList &getGrid() const
        {
            return mGrid;
        }
    private:
        QVariantList mGrid;


};
class GridUpdate : public GridVisitor
{

    public:
        GridUpdate() = default;
        void visit(GridInt* pGrid, QString &action, int &line) override;
        void visit(GridBool* pGrid, QString &action, int &line) override;
        void visit(GridFloat* pGrid, QString &action, int &line) override;
        void visit(GridString* pGrid, QString &action, int &line) override;
        void visit(GridLight* pGrid, QString &action, int &line) override;
        void visit(GridImg* pGrid, QString &action, int &line) override;
        void visit(GridVideo* pGrid, QString &action, int &line) override;
        void visit(GridMessage* pGrid, QString &action, int &line) override;
        void visit(GridGraph* pGrid, QString &action, int &line) override;
        void visit(GridPrg* pGrid, QString &action, int &line) override;

};

class GridJsonDumper: public GridVisitor
{

    public:
        GridJsonDumper() = default;
        void visit(GridInt* pGrid, QString &action, int &line) override;
        void visit(GridBool* pGrid, QString &action, int &line) override;
        void visit(GridFloat* pGrid, QString &action, int &line) override;
        void visit(GridString* pGrid, QString &action, int &line) override;
        void visit(GridLight* pGrid, QString &action, int &line) override;
        void visit(GridImg* pGrid, QString &action, int &line) override;
        void visit(GridVideo* pGrid, QString &action, int &line) override;
        void visit(GridMessage* pGrid, QString &action, int &line) override;
        void visit(GridGraph* pGrid, QString &action, int &line) override;
        void visit(GridPrg* pGrid, QString &action, int &line) override;

        [[nodiscard]] const QJsonObject &getResult() const
        {
            return mResult;
        }

    private:
        QJsonObject mResult;
        QString mJsonString;
        QJsonObject dumpGridCommons(GridBase *pGrid);

};


}
#endif //OST_ELEMENTUPDATE_H
