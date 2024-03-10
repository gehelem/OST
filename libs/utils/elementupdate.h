#ifndef OST_VALUEUPDATE_H
#define OST_VALUEUPDATE_H
#include <string>
#include <valueint.h>
#include <valuefloat.h>
#include <valuebool.h>
#include <valuestring.h>
#include <valuelight.h>
#include <valueimg.h>
#include <valuevideo.h>
#include <valuemessage.h>
#include <valuegraph.h>
#include <valueprg.h>

namespace  OST
{

class ValueUpdate : public ValueVisitor
{

    public:
        ValueUpdate() = default;
        void visit(ValueInt* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueInt* pValue, QVariantMap &data ) override;
        void visit(ValueInt* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueFloat* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueFloat* pValue, QVariantMap &data ) override;
        void visit(ValueFloat* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueBool* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueBool* pValue, QVariantMap &data ) override;
        void visit(ValueBool* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueString* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueString* pValue, QVariantMap &data ) override;
        void visit(ValueString* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueLight* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueLight* pValue, QVariantMap &data ) override;
        void visit(ValueLight* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueImg* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueImg* pValue, QVariantMap &data ) override;
        void visit(ValueImg* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueVideo* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueVideo* pValue, QVariantMap &data ) override;
        void visit(ValueVideo* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueMessage* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueMessage* pValue, QVariantMap &data ) override;
        void visit(ValueMessage* pValue, QString &action, QVariantMap &data) override;
        void visit(ValueGraph* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValueGraph* pValue, QVariantMap &data ) override;
        void visit(ValueGraph* pValue, QString &action, QVariantMap &data) override;
        void visit(ValuePrg* pValue) override
        {
            Q_UNUSED(pValue)
        }
        void visit(ValuePrg* pValue, QVariantMap &data ) override;
        void visit(ValuePrg* pValue, QString &action, QVariantMap &data) override;

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
#endif //OST_VALUEUPDATE_H
