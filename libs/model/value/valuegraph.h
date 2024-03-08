#ifndef VALUEGRAPH_h
#define VALUEGRAPH_h

#include <valuesingle.h>

namespace  OST
{

class ValueGraph: public ValueSingleNotNumeric<GraphDefs>
{

        Q_OBJECT

    public:
        void accept(ValueVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(ValueVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }
        void accept(ValueVisitor *pVisitor, QString &action, QVariantMap &data) override
        {
            pVisitor->visit(this, action, data);
        }

        ValueGraph(const QString &label, const QString &order, const QString &hint);
        ~ValueGraph();
        QString getType() override
        {
            return "graph";
        }

        GraphDefs getGraphDefs(void)
        {
            return mGraphDefs;
        }
        void setGraphDefs(GraphDefs defs);
        void setGraphParams(QVariantMap params);

    private:
        GraphDefs mGraphDefs;

};

class GridGraph: public GridTemplate<GraphDefs>
{
    public:

        void accept(GridVisitor* pVisitor, QVariant &data)  override
        {
            pVisitor->visit(this, data);
        }

        QString getType() override
        {
            return "graph";
        }
};


}
#endif
